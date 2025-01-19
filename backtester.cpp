#include<stdio.h>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
namespace fs = std::filesystem;
using namespace std;
size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *data = static_cast<std::string*>(userdata);
    data->append(ptr, size * nmemb);
    return size * nmemb;
}
    
string APIKEY = "YOUR API KEY";
std::string stockurl = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&datatype=csv&outputsize=full&apikey=" + APIKEY + "&symbol=";
string optionurl = "https://www.alphavantage.co/query?function=HISTORICAL_OPTIONS&datatype=csv&apikey=" + APIKEY + "&symbol=";

void get_data_inet(string url, string *data){
    CURL *curl = curl_easy_init();
    cout<<url<<endl;
    cout<<"BEFORE:"<<endl;
    cout<<*data<<endl;
    if(curl){
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	    res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } /*else {
            long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        //cout<<response_code<<endl;
            //std::cout << "Response: " << *data << std::endl;
        }*/
        curl_easy_cleanup(curl);
    }
    else cout<<"No curl"<<endl;
}

void get_data(string symbol, string*data, int stock_option, string date){
    string filepath;
    string url;
    if(stock_option==0) {
        filepath = "data/stock/" + symbol+".csv";
        url = stockurl + symbol;
    }
    else {
        filepath = "data/option/" + symbol+ "_" + date+".csv";
        url = optionurl + symbol + "&date="+date;
    }
    if (!fs::exists(filepath)){
        ofstream file(filepath);
        if(!file){
            cerr<<"Error creating file: "<<filepath<<endl;
            return;
        }
        get_data_inet(url, data);
        file << *data <<endl;
        file.close();
    }
    else{
        ifstream file(filepath);
        if(!file.is_open()){
            cerr<<"Error opening file: "<<filepath<<endl;
            return;
        }
        string line;
        while(getline(file, line)){
            *data+=line + "\n";
        }
        file.close();
    }
}

ofstream createFile(string path){
    if(!fs::exists(path)){
        ofstream file(path);
        if(!file){
            cerr<<"Error opening file: "<<path<<endl;
            exit(1);
        }
        return file;
    }
    else{
        cerr<<"File already exists!: "<<endl;
        exit(1);
    }
}
vector<string> split(string text, char delimeter){
    vector <string> result;
    stringstream s(text);
    string line;
    while(getline(s, line, delimeter)){
        result.push_back(line);
    }
    return result;
}
float calc_pv(unordered_map<string, vector<float>> info, unordered_map<string, int> prt){
    float res = 0;
    for(const auto&pair:prt){
        if(info.find(pair.first)!=info.end()){
            if(pair.second<0){
                res = res+info[pair.first][3]*pair.second;
            }
            else{
                res = res+info[pair.first][1]*pair.second;
            }
        }
    }
    return res;
}
int main(int argc, char* argv[])
{
    string port = argv[1];
    string symbol = argv[2];
    float capital = stof(argv[3]);
    float initial_capital = capital;
    string start_date = argv[4];
    int trade_length = stoi(argv[5]);
    float transaction_fee = 0;

    auto now = std::chrono::system_clock::now();
    time_t current_time = std::chrono::system_clock::to_time_t(now);
    tm local_tm = *std::localtime(&current_time);
    ostringstream oss;
    oss << put_time(&local_tm, "%Y%m%d_%H%M%S");
    string datetime = oss.str();
    cout<<datetime<<endl;
    //ofstream meta = createFile("logs/metas/"+datetime+".log");
    ofstream session = createFile("logs/sessions/"+datetime+".csv");
    //string stockapi = stockurl+argv[2]+"&apikey="+APIKEY;
    //string optionapi = optionurl + argv[2]+"&apikey="+APIKEY;
    //std::cout << stockapi << std::endl;
    //std::cout << optionapi << std::endl;
    string stock_data;
    string option_data;
    get_data(symbol, &stock_data, 0, "");
    //cout<<"Got the data"<<endl;
    //get_data(optionapi, &option_data, curl);
    vector<string> stock_vector= split(stock_data, '\n');
    size_t pos = 0;
    string token;
    string date;
    string stock_meta;
    string option_meta;
    //int o_size = atoi(argv[1]);
    //cout<<o_size<<endl;
    pos = stock_data.find("\n");
    stock_meta = stock_vector[0] + "\n";
    reverse(stock_vector.begin(), stock_vector.end());
    /*
    pos = option_data.find("\n");
    option_meta = option_data.substr(0,pos+1);
    option_data.erase(0,pos+1);
    cout << token << endl;
*/

    int server_fd, sock;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buff[1024] = {0};

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0))==0){
       perror("socket failed");
       exit(EXIT_FAILURE);
      }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
       perror("setsockopt");
       exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(stoi(port));


    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
       perror("bind failed");
       exit(EXIT_FAILURE);
    }
    if(listen(server_fd,3)<0){
      perror("listen");
      exit(EXIT_FAILURE);
    }

    cout<< "Waiting connect on port: "<<port<<endl;
    if ((sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }


    int i=1;
    while(i<stock_vector.size()){
        token = stock_vector[i];
        if(token.find(start_date)!=string::npos) break;
        i++;
    }

    //cout<<i<<endl;
    float pv = 0;
    unordered_map<string, int> portfolio;
    // contractID -> #of contracts;
   
    unordered_map<string, vector<float>> current_option_infos;
    // contractID ->vector[type, bid, bid_size, ask, ask_size, strike]
    
    float underlying_price;
    vector<string> temp;
    char buffer[4096] = {};
    
    vector<string> orders;
    vector<string> order;
    vector<string> options;
    //float margin_pct = 0.1;
    // order:
    // or vectors separated by: contractID, size, action
    session<<"step,contractID,size,action,status,capital,portfolio value,pnl"<<endl;
    for(int j=i; j<i+trade_length&&j<stock_vector.size(); j++){
        send(sock, stock_meta.c_str(), stock_meta.length(), 0);
        //cout<<stock_meta;
        token = stock_vector[j] + "\n";
        send(sock, token.c_str(), token.length(),0);
        send(sock, "\n", 1, 0);
        date = token.substr(0,10);
        //cerr<<token;
        underlying_price = stof(split(token, ',')[4]);


        get_data(symbol, &option_data, 1, date);
        options = split(option_data, '\n');
        token = options[0];
        send(sock, token.c_str(), token.length(),0);
        send(sock, "\n", 1, 0);
        for(int k=1; k<options.size(); k++){
        token = options[k] + "\n";
        //cerr<<"TOKEN SIZE: "<<token.size()<<endl;
        if(token.size()>1){
        temp = split(token, ',');
        //cerr<<token;
        //cerr<<temp[0]<< " "<<temp[7]<<" "<<temp[8]<<" "<<temp[9]<<" "<<temp[10]<<endl;
        current_option_infos[temp[0]]= vector<float>{0,stof(temp[7]), stof(temp[8]), stof(temp[9]), stof(temp[10]), stof(temp[3])};
        if(temp[4]=="put") current_option_infos[temp[0]][0]=1;
        send(sock, token.c_str(), token.length(),0);
        }
        }
        option_data = "";
        send(sock, "\n", 1, 0);
        recv(sock, buffer, 4096, 0);
        orders = split(buffer, '\n');
        //cout<<"Orders Size: "<<orders.size()<<endl;
        /*cout<<"test: "<<orders[0].size()<<endl;*/
        /*if(orders.empty()){
            cout<<"EMPTY"<<endl;
        }*/
        for(int k=0; k<orders.size(); k++){
            if(orders[k].size()==0)break;
            string status = "Fail";
            cout<<orders[k]<<endl;
            order = split(orders[k], ',');
            string contract = order[0];
            int size = stoi(order[1]);
            string action = order[2];
            if(current_option_infos.find(contract)!=current_option_infos.end()){
                vector<float> info = current_option_infos[contract];
                if(action=="B"){
                    if(size<=info[4]){
                        float cost = size*info[3] + transaction_fee;
                        if (cost<=capital){
                            capital=capital-cost;
                            portfolio[contract] = portfolio[contract]+1;
                        }
                        //float margin = cost;
                    }
                }
                else if(action=="S"){
                    if(size<=info[2]){
                        float revenue = size*info[1] - transaction_fee;
                        capital = capital+revenue;
                        portfolio[contract] = portfolio[contract]-1;
                    }
                }
                status = "Success"; 
            }  
            pv = calc_pv(current_option_infos, portfolio); 
            session<<k<<","<<orders[k]<<","<<status<<","<<capital<<","<<pv<<","<<capital+pv-initial_capital<<endl;
        }
    };
    //meta.close();
    session.close();
    close(sock);
    return 0;
}

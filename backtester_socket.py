import socket
import pandas as pd
from io import StringIO
class Socket:
    def __init__(self, ip: str, port: int):
        '''
        Creates socket using given ip and port.
        '''
        self.ip = ip
        self.port = port
        self.sock = None
        self.file_descriptor = None
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            print(f"Socket created for IP {self.ip} and port {self.port}")
        except socket.error as e:
            print(f"Error creating socket: {e}")
            raise
    def connect(self):
        '''
        Connects to the socket if the server exists.
        '''
        try:
            self.sock.connect((self.ip, self.port))
            print(f"Connected to {self.ip} on port {self.port}")
            self.file_descriptor = self.sock.makefile('r')
        except socket.error as e:
            print(f"Error connecting to {self.ip}:{self.port}: {e}")
            raise
    def get_csv_string(self):
        res = ""
        while(True):
            line = self.file_descriptor.readline()
            if line:
                res+=line+ "\n"
                #print(line)
                if(len(line)==1):
                    break
        return res
    def get_data(self):
        '''
        Receives stock&option data from the server. Converts them to stock and option dataframes.
        Returns tuple in the form of (stock, option).
        '''
        temp = self.get_csv_string()
        stock = pd.read_csv(StringIO(temp))
        temp = self.get_csv_string()
        option = pd.read_csv(StringIO(temp))
        return(stock, option)
    def send_orders(self, orders):
        '''
        Takes pandas dataframe as an argument, sends them to server.
        Dataframe format:
        contractID,size,action

        Example:
        IBM270115P00340000,1,B
        '''
        if(not isinstance(orders, pd.DataFrame)):
            print("Not valid input")
            return
        res = ""
        for _, row in orders.iterrows():
            res += str(row["contractID"])+","+str(row["size"])+","+row["action"]+"\n"
        res+="\n"
        self.sock.sendall(res.encode("utf-8"))
    def close(self):
        '''
        Closes the socket.
        '''
        if self.file_descriptor:
            self.file_descriptor.close()
        if self.sock:
            self.sock.close()            
# Backtesting Engine for Options Trading Strategies

## Features
- Access to Market Data: The engine uses historical data from Alpha Vantage's API.
- Saves Data: To avoid waste of API requests, the engine saves the retrieved data to access it during other sessions.
- The Backtesting engine sends the data and receives the orders through the socket. The engine processes the orders and logs them to .csv file
- Separate socket class for sending orders allows the users to use the backtester with the least changes in their code.
- Faster testing performance

## Usage 
- Install [libcurl]("https://curl.se/libcurl/c/libcurl-tutorial.html) and setup linking in MakeFile
- Input the API key in backtester.cpp code (line 23)
- run "make" command to generate executable file
- run "./backtester {port} {symbol} {capital} {date} {length}" Example: "./backtester 10157 IBM 10000 2024-12-10 25"
- When backtester has started, connect to it using backtester_socket.py
- See the [example code](https://github.com/K1ta141k/Backtesting-Engine/blob/main/Using_Backtester_Socket.ipynb) for using the backtester engine

## Comments
If you have any issues or suggestions, please open an issue!
Thank You! :3

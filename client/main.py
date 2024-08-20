import socket
import json

settings = open('./settings.json')
settings = json.load(settings)



HOST = settings['HOST']  # The server's hostname or IP address
PORT = settings['PORT']  # The port used by the server

def welcomeScreen():
    print('''
\033[1;33m      ▄████▀█▄         \033[0m\033[0;33m .       , .        .__ .           ,  , 
\033[1;33m    ▄█████████████████▄\033[0m\033[0;33m |   . .-+-|_  _ ._.[__)|* __ __ _ -+--+-
\033[1;33m  ▄█████.▼.▼.▼.▼.▼.▼▼▼▼\033[0m\033[0;33m	|___(_| | [ )(/,[  [__)||_) _) (/, |  | 	
\033[1;33m ▄███████▄.▲.▲▲▲▲▲▲▲▲	\033[0m
\033[1;33m████████████████████▀▀ \033[0m By \033[0;34m@UsboKirishima\033[0m
''')

    
welcomeScreen()

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
client_socket.connect((HOST, PORT))

message = input(" -> ")

while message.lower().strip() != 'exit':

        
    client_socket.send((message + '\n').encode())

    data = client_socket.recv(1024).decode()  
    print('Received from server: ' + data)


    
    message = input(" -> ")  

client_socket.send(message.encode())  
client_socket.close() 

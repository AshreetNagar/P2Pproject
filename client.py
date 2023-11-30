import socket
import select
import sys
import os

MAX_DATA_SIZE = 100

sockets = []
socketsIndex = 0
filenames = []

class PDU:
    def __init__(self):
        self.type = ''
        self.data = [None]*100

def handleRegisterContent(peer, filename): ##Handles peer and content registration
    ##Registration PDU is initialized with peer name and content name
    contentRegistration = PDU()
    contentRegistration.type = 'R'
    peerLength = len(peer)
    contentRegistration.data[0:peerLength] = peer #could have been appended but i tried with a c approach with byte by byte
    contentRegistration.data[peerLength] = '\n' #new line appended to split it to show that it is done
    filenameLength = len(filename) 
    contentRegistration.data[peerLength+1:filenameLength] = filename
    contentRegistration.data[peerLength+filenameLength+1] = '\n'
    try:
        file = open(filename, "rb")
    except:
        print("Error, file not found. Please try again.\n")
        main()
    filesize = os.stat(filename).st_size
    if filesize == 0:
        print("Sorry, file is empty. Please try again.\n")
        main()
    filenames.append(filename) ##An array is used to keep track of uploaded content

    ##A TCP socket is created for each new content a user uploads
    contentServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #insitalize socket with IPv4 Address and as a tcp socket
    reg_addr = ('', 0) #set address to any interface and set port to any port
    contentServerSocket.bind(reg_addr) #socket is binded
    address = contentServerSocket.getsockname()[0] #address is retrieved from itself
    addressLength = len(address)
    port = str(contentServerSocket.getsockname()[1]) #port is retreived from itself
    portLength = len(port)
    contentServerSocket.listen(5) #checks for incoming connections

    ##Address and port are also added to Registration PDU
    contentRegistration.data[peerLength+filenameLength+2:peerLength+filenameLength+2+addressLength] = address #address is appened to contentregistered data
    contentRegistration.data[peerLength+filenameLength+2+addressLength] = '\n' 
    contentRegistration.data[peerLength+filenameLength+addressLength+3:peerLength+filenameLength+addressLength+portLength+3] = port #port is appened to contentregister data
    print(contentRegistration.data)
    contentRegistrationString = contentRegistration.type + "".join([char for char in contentRegistration.data if char is not None]) #because it is byte by byte, all characters are joined as long as they are not none
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) ##A UDP socket is initialized for communication with the server
    indexServerSocket.sendto(contentRegistrationString.encode(), ("127.0.0.1", 3000))
    print("Data sent to index server")
    print(contentRegistrationString)

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE) ##Client receives incoming data from the index server
    
    ##A response PDU is initialized to handle and parse data sent by the index server
    contentRegistrationResponse = PDU()
    contentRegistrationResponse.type = data.decode()
    print(f"Response type: {contentRegistrationResponse.type}")
    if contentRegistrationResponse.type == 'E':
        print("Error, a peer with the same name already exists. Try again.")
        peer = str(input("Please enter your name (10 characters only): "))
        filename = str(input("Please enter the name of the file you would like to register (10 characters only): "))
        handleRegisterContent(peer, filename)
    else:
        print(f"Content host server listening on port {port}")

        sockets.append(contentServerSocket) ##An array is used to keep track of the TCP sockets that have been created and are listening
        print(sockets)


def handleContentDownload(peer, content): ##Handles content download
    ##Search PDU is initialized with peer name and content name to check if requested content is available
    contentSearch = PDU()
    contentSearch.type = 'S'
    peerLength = len(peer)
    contentSearch.data[0:peerLength] = peer
    contentSearch.data[peerLength] = '\n'
    filenameLength = len(content)
    contentSearch.data[peerLength+1:filenameLength] = content
    contentSearch.data[peerLength+filenameLength+1] = '\n'
    print(contentSearch.data)
    contentSearchString = contentSearch.type + "".join([char for char in contentSearch.data if char is not None])
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    indexServerSocket.sendto(contentSearchString.encode(), ("127.0.0.1", 3000))
    print("Data sent to index server")

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE) ##Client receives incoming data from the index server
    
    ##A response PDU is initialized to handle and parse data sent by the index server
    contentSearchResponse = PDU()
    contentSearchResponse.type = data[0:1].decode()
    if contentSearchResponse.type == 'S':
        contentSearchResponse.data = data[1:].decode().split("\n")
        address = contentSearchResponse.data[0]
        port = contentSearchResponse.data[1]
        print("Content found")
        print(f"Response type: {contentSearchResponse.type}")
        print(f"Content server address and port: {address}, {port}")

        ##Download PDU is initialized to signal the index server to begin file download
        contentDownload = PDU()
        contentDownload.type = 'D\n'
        contentDownload.data = content
        contentDownloadString = contentDownload.type + contentDownload.data
        contentClientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        contentClientSocket.connect((address, int(port)))
        contentClientSocket.sendall(contentDownloadString.encode())
        print(f"TCP connection established with content server {address}, {port}")
        print(f"Response type: {contentClientSocket.recv(1).decode()}")
        with open(f"{content}", "w") as f:
            while True:
                data = contentClientSocket.recv(1024)
                if not data:
                    break
                print(f"{data} received")
                f.write(data.decode())
                print("File successfully downloaded")
                break
            f.close()
            handleRegisterContent(peer, content) ##Registration function is called to make the user a content host of the downloaded content
        main()
    else:
        print("Content not found")
        print(f"Response type: {contentSearchResponse.type}")

def handleContentListing():
    ##Listing PDU is initialized to request index server for available content
    contentListing = PDU()
    contentListing.type = 'O'
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    indexServerSocket.sendto(contentListing.type.encode(), ("127.0.0.1", 3000))
    print("Data sent to index server")
 
    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE) ##Client receives incoming data from the index server
    
    ##A response PDU is initialized to handle and parse data sent by the index server
    contentListingResponse = PDU()
    contentListingResponse.type = data[0:1].decode()
    contentListingResponse.data = data[1:].decode().split("X") #splits by the x to list the contents so x is new line character
    print("Content List:")
    for content in contentListingResponse.data:
        print(content)

def handleDeregisterContent(peer, content):
    ##Deregistration PDU is initialized with peer name and content name for content deregistration
    contentDeregistration = PDU()
    contentDeregistration.type = 'T'
    peerLength = len(peer)
    contentDeregistration.data[0:peerLength] = peer
    contentDeregistration.data[peerLength] = '\n'
    filenameLength = len(content)
    contentDeregistration.data[peerLength+1:filenameLength] = content
    contentDeregistration.data[peerLength+filenameLength+1] = '\n'
    print(contentDeregistration.data)
    contentDeregistrationString = contentDeregistration.type + "".join([char for char in contentDeregistration.data if char is not None])
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    indexServerSocket.sendto(contentDeregistrationString.encode(), ("127.0.0.1", 3000))

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE) ##Client receives incoming data from the index server

    ##A response PDU is initialized to handle and parse data sent by the index server
    contentDeregistrationResponse = PDU()
    contentDeregistrationResponse.type = data[0:1].decode()

    if contentDeregistrationResponse.type == 'A':
        print("Content successfully deregistered")
    if contentDeregistrationResponse.data:
        contentDeregistrationResponse.data = data[1:].decode()
        print(contentDeregistrationResponse.data)
    else:
        print("Error")

def handleUserInput(option): ##Handles user input
    if(option == "1"): ##Calls content registration function
        peer = str(input("Please enter your name (10 characters only): "))
        filename = str(input("Please enter the name of the file you would like to register (10 characters only): "))
        handleRegisterContent(peer, filename)
    if(option == '2'): ##Calls content download function
        peer = str(input("Please enter your name: "))
        content = str(input("Please enter the content you would like to download: "))
        handleContentDownload(peer, content)
    if(option == '3'): ##Calls content listing function
        handleContentListing()
    if(option == '4'): ##Calls content deregistration function
        peer = str(input("Please enter your name: "))
        content = str(input("Please enter the content you would like to deregister: "))
        handleDeregisterContent(peer, content)
    if(option == '5'): ##Calls user quit function
        peer = str(input("Please enter your name: "))
        content = "All"
        handleDeregisterContent(peer, content)

def main():
        run = 1
        while run:
            inputsockets = [sys.stdin] + sockets ##Creates an array with user input and the created TCP sockets
            print("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n5.Quit\n") ##Prompts the user for input
            print("Please choose one of the above: ")
            readable, _, _ = select.select(inputsockets, [], []) 
            print("Select passed")
            if sys.stdin in readable: ##Handles user input
                option = str(input())
                handleUserInput(option=option)                
                if option == 'exit':
                    run = 0
                    print("Program closed")
                    exit()
            else: ##Handles TCP sockets
                print(f"Got TCP sockets: {readable}")
                for sock in readable:
                    while True:
                        ##Download PDU is initialized to receive signal from client for content download
                        contentDownload = PDU()
                        conn, addr = sock.accept() ##Peer accepts connection from peer acting as content client
                        response = conn.recv(1024).decode().split("\n")
                        contentDownload.type = response[0] #Parses PDU type
                        contentDownload.data = response[1] #Parses PDU data
                        print(f"Response type: {contentDownload.type}")
                        print(f"Content name: {contentDownload.data}")
                        print(f"Content client {addr} has connected")
                        conn.sendall(b'C')
                        filename = contentDownload.data ##Retrieves the filename from the contentDownload PDU sent by requesting client
                        with open(filename, 'r') as f:
                            packet = f.read(1024)
                            print(f"{packet.encode()} sent")
                            while packet:
                                conn.sendall(packet.encode())
                                packet = f.read(1024)
                            f.close()
                        conn.close()
                        print("File successfully sent") ##Shows on completion that file has been sent
                        break

main()

# if __name__ == "__main__":
#     run = 1
#     while run:
#         print("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n")
#         option = str(input("Please choose one of the above: "))
#         handleUserInput(option=option)
#         if option == 'exit':
#             run = 0
#             print('Program closed')
import socket
import select
import sys

MAX_SOCKS = 10
MAX_DATA_SIZE = 100

# sockets = [None]*10
sockets = []
socketsIndex = 0
filenames = []

class PDU:
    def __init__(self):
        self.type = ''
        self.data = [None]*100

def handleRegisterContent(peer, filename):
    contentRegistration = PDU()
    contentRegistration.type = 'R'
    peerLength = len(peer)
    contentRegistration.data[0:peerLength] = peer
    contentRegistration.data[peerLength] = '\n'
    filenameLength = len(filename)
    contentRegistration.data[peerLength+1:filenameLength] = filename
    contentRegistration.data[peerLength+filenameLength+1] = '\n'
    file = open(filename, "rb")
    if not file:
        print("Error, file not found.")
    filenames.append(filename)

    contentServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    reg_addr = ('', 0)
    contentServerSocket.bind(reg_addr)
    address = contentServerSocket.getsockname()[0]
    addressLength = len(address)
    port = str(contentServerSocket.getsockname()[1])
    portLength = len(port)
    contentServerSocket.listen(5)
    ##Child process
    # while True:
    #     conn, addr = contentServerSocket.accept()
    #     print(f"{addr} has connected")
    #     filename = filenames[sockets.index(sock)]
    #     print(filename)

    contentRegistration.data[peerLength+filenameLength+2:peerLength+filenameLength+2+addressLength] = address
    contentRegistration.data[peerLength+filenameLength+2+addressLength] = '\n'
    contentRegistration.data[peerLength+filenameLength+addressLength+3:peerLength+filenameLength+addressLength+portLength+3] = port
    print(contentRegistration.data)
    contentRegistrationString = contentRegistration.type + "".join([char for char in contentRegistration.data if char is not None])
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    indexServerSocket.sendto(contentRegistrationString.encode(), ("127.0.0.1", 3000))
    print("Data sent to index server")
    print(contentRegistrationString)

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE)
    
    contentRegistrationResponse = PDU()
    contentRegistrationResponse.type = data.decode()
    print(f"Response type: {contentRegistrationResponse.type}")
    if contentRegistrationResponse.type == 'E':
        print("Error, a peer with the same name already exists. Try again.")
        peer = str(input("Please enter your name (10 characters only): "))
        filename = str(input("Please enter the name of the file you would like to register (10 characters only): "))
        # return handleRegisterContent(peer, filename)
        handleRegisterContent(peer, filename)
    else:
        # print(f"Response type: {contentRegistrationResponse.type}")
        print(f"Content host server listening on port {port}")

        sockets.append(contentServerSocket)
        print(sockets)

    # while True:
    #     conn, addr = contentServerSocket.accept()
    #     print(f"{addr} has connected")
    #     filename = filenames[sockets.index(sock)]
    #     print(filename)


def handleContentDownload(peer, content):
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

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE)
    
    contentSearchResponse = PDU()
    contentSearchResponse.type = data[0:1].decode()
    if contentSearchResponse.type == 'S':
        contentSearchResponse.data = data[1:].decode().split("\n")
        address = contentSearchResponse.data[0]
        port = contentSearchResponse.data[1]
        print("Content found")
        print(f"Response type: {contentSearchResponse.type}")
        print(f"Content server address and port: {address}, {port}")

        contentDownload = PDU()
        contentDownload.type = 'D'
        # contentDownload.data.append(filename)
        contentClientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        contentClientSocket.connect((address, int(port)))
        contentClientSocket.sendall(contentDownload.type.encode())
        print(f"TCP connection established with content server {address}, {port}")
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
        # contentClientSocket.close()
            handleRegisterContent(peer, content)
        main()
    else:
        print("Content not found")
        print(f"Response type: {contentSearchResponse.type}")

def handleContentListing():
    contentListing = PDU()
    contentListing.type = 'O'
    indexServerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    indexServerSocket.sendto(contentListing.type.encode(), ("127.0.0.1", 3000))
    print("Data sent to index server")
 
    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE)

    contentListingResponse = PDU()
    contentListingResponse.type = data[0:1].decode()
    contentListingResponse.data = data[1:].decode().split("X")
    print("Content List:")
    for content in contentListingResponse.data:
        print(content)

def handleDeregisterContent(peer, content):
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

    data, addr = indexServerSocket.recvfrom(MAX_DATA_SIZE)

    contentDeregistrationResponse = PDU()
    contentDeregistrationResponse.type = data.decode()

    if contentDeregistrationResponse.type == 'A':
        print("Content successfully deregistered")
    else:
        print("Error")
    

def handleUserInput(option):
    if(option == "1"):
        peer = str(input("Please enter your name (10 characters only): "))
        filename = str(input("Please enter the name of the file you would like to register (10 characters only): "))
        handleRegisterContent(peer, filename)
    if(option == '2'):
        peer = str(input("Please enter your name: "))
        content = str(input("Please enter the content you would like to download: "))
        handleContentDownload(peer, content)
    if(option == '3'):
        handleContentListing()
    if(option == '4'):
        peer = str(input("Please enter your name: "))
        content = str(input("Please enter the content you would like to deregister: "))
        handleDeregisterContent(peer, content)

# if __name__ == "__main__":
def main():
        run = 1
        while run:
            inputsockets = [sys.stdin] + sockets
            print("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n")
            print("Please choose one of the above: ")
            readable, _, _ = select.select(inputsockets, [], [])
            print("Select passed")
            # option = str(input("Please choose one of the above: "))
            if sys.stdin in readable:
                # print("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n")
                # option = str(input("Please choose one of the above: "))
                option = str(input())
                handleUserInput(option=option)                
                if option == 'exit':
                    run = 0
                    print("Program closed")
            else:
                print(f"Got TCP sockets: {readable}")
                for sock in sockets:
                    while True:
                        contentDownload = PDU()
                        conn, addr = sock.accept()
                        contentDownload.type = conn.recv(1024).decode()
                        print(f"Response type: {contentDownload.type}")
                        print(f"Content client {addr} has connected")
                        filename = filenames[sockets.index(sock)]
                        print(filename)
                        with open(filename, 'r') as f:
                            packet = f.read(1024)
                            while packet:
                                conn.sendall(packet.encode())
                                packet = f.read(1024)
                                print(f"{f.read(1024)} sent")
                            f.close()
                        print("File successfully sent")
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
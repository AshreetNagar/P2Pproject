import socket

MAX_DATA_SIZE = 100

max_peers = 10
current_peers = 0
peernames = [[] for _ in range(max_peers)]
filenames = [[] for _ in range(max_peers)]
ips = [[] for _ in range(max_peers)]
ports = [[] for _ in range(max_peers)]

class PDU:
    def __init__(self):
        self.type = ''
        self.data = [None]*100

def registerContent(client_socket, data, addr):
    peer = data[0]
    filename = data[1]
    # address = str(data[2])
    port = str(data[3])
    address = addr[0]
    # port = str(addr[1])

    global current_peers, max_peers
    contentRegistrationResponse = PDU()

    files = ["".join(char) for char in filenames if char is not None]
    peers = ["".join(char) for char in peernames if char is not None]

    if filename in files and peer in peers:
            contentRegistrationResponse.type = 'E'    
    else:
    # Find the first available slot for the new peer
        for i in range(current_peers, max_peers):
            if not peernames[i]:
                peernames[i] = list(peer)
                filenames[i] = list(filename)
                ips[i] = list(address)
                ports[i] = list(port)
                current_peers += 1
                contentRegistrationResponse.type = 'A'
                break

    print(peernames)
    print(filenames)
    print(ips)
    print(ports)

    client_socket.sendto(contentRegistrationResponse.type.encode(), addr)
    print("Response sent")

def contentDownload(client_socket, data, addr):
    peer = data[0]
    content = data[1]

    contentCat = []
    addressCat = []
    portCat = []
    for file in filenames:
        contentCat.append("".join(file))
    
    for addy in ips:
        addressCat.append("".join(addy))
    
    for port in ports:
        portCat.append("".join(port))

    contentSearchResponse = PDU()
    contentSearchResponse.type = 'E'
    
    if content in contentCat:
        print("Content found")
        contentSearchResponse.type = 'S'
        index = contentCat.index(content)
        host = addressCat[index]
        port = portCat[index]
        contentSearchResponse.data.append(host)
        contentSearchResponse.data.append("\n")
        contentSearchResponse.data.append(port)
        contentSearchString = contentSearchResponse.type + "".join([char for char in contentSearchResponse.data if char is not None])
        print("Sending address and port")
    else:
        print("Content not found")
        contentSearchString = contentSearchResponse.type

    print(contentSearchString)
        
    client_socket.sendto(contentSearchString.encode(), addr)
    print("Response sent")

def contentListing(client_socket, addr):
    contentListing = PDU()
    contentListing.type = 'O'
    print(filenames)
    for file in filenames:
        contentListing.data.append("".join(file))
    contentListing.data = [file + "X" for file in contentListing.data if file]
    print(contentListing.data)
    contentListingString = contentListing.type + "".join([char for char in contentListing.data if char is not None])
    print(contentListingString)

    client_socket.sendto(contentListingString.encode(), addr)
    print("Response sent")

def DeregisterContent(client_socket, data, addr):
    peer = data[0]
    content = data[1]
    contentDeregistration = PDU()

    print(peernames)
    print(filenames)
    
    # for name in peernames:
    names = ["".join(name) for name in peernames if name]

    # for file in filenames:
    contents = ["".join(file) for file in filenames if file]

    print(names.index(peer))
    print(contents.index(content))
    
    if content in contents and contents.index(content) == names.index(peer):
        contentIndex = contents.index(content)
        del filenames[contentIndex]
        contentDeregistration.type = 'A'
    else:
        contentDeregistration.type = 'E'

    print(filenames)

    client_socket.sendto(contentDeregistration.type.encode(), addr)
    print("Response sent")
    
def handleFileRequest(client_socket):
    try:
        data, addr = client_socket.recvfrom(MAX_DATA_SIZE)
        if not data:
            return
        
        Response = PDU()
        Response.type = data[0:1].decode()
        Response.data = data[1:].decode()
        parts = Response.data.split("\n")
        print(Response.type)
        # print(parts)

        if Response.type == 'R':
            print(f"Received content registration request for: {data}")
            registerContent(client_socket, parts, addr)
        if Response.type == 'S':
            print(f"Received content download request for: {data}")
            contentDownload(client_socket, parts, addr)
        if Response.type == 'O':
            contentListing(client_socket, addr)
        if Response.type == 'T':
            print(f"Received content deregistration request for: {data}")
            DeregisterContent(client_socket, parts, addr)
    except Exception as e:
        print(f"Error receiving PDU: {e}")

if __name__ == "__main__":

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.bind(("127.0.0.1", 3000))

    print(f"Server listening on port 3000")

    try:
        while True:
            handleFileRequest(client_socket)
    except KeyboardInterrupt:
        print("Server is shutting down")
    finally:
        client_socket.close()
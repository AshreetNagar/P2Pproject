import socket

MAX_DATA_SIZE = 100

max_peers = 10
current_peers = 0
peernames = [[] for _ in range(max_peers)]
filenames = [[] for _ in range(max_peers)]
ips = [[] for _ in range(max_peers)]
ports = [[] for _ in range(max_peers)]
downloadRequests = [0 for _ in range(max_peers)]

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
    contentRegistrationResponse = PDU() ##A registration PDU is initialized to send info once content registration is complete

    files = ["".join(char) for char in filenames if char is not None]
    peers = ["".join(char) for char in peernames if char is not None]

    ##Sends an error if there is a peer already registered with the same name and content
    if filename in files and peer in peers:
        if files.index(filename) == peers.index(peer):
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

    client_socket.sendto(contentRegistrationResponse.type.encode(), addr) ##Sends content registration response back to peer
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

    contentSearchResponse = PDU() ##A search PDU is initialized to send info whether content is available or not
    contentSearchResponse.type = 'E'
    
    ##Checks whether content is available
    if content in contentCat:
        print("Content found")
        contentSearchResponse.type = 'S'
        # index = contentCat.index(content)
        indices = [i for i, x in enumerate(contentCat) if x == content] ##Load balancing is implemented here; Indices of content names are appended here
        requests = [downloadRequests[i] for i in indices] ##Content download requests are appended here for each socket, showing how many downloads have taken place
        index = requests.index(min(requests)) ##Retrieves the index with the least download requests
        host = addressCat[index] ##Retrieves content host address
        port = portCat[index] ##Retrieves content host port
        downloadRequests[index] += 1 ## Increments content host download request by 1 
        print(downloadRequests)
        contentSearchResponse.data.append(host)
        contentSearchResponse.data.append("\n")
        contentSearchResponse.data.append(port)
        contentSearchString = contentSearchResponse.type + "".join([char for char in contentSearchResponse.data if char is not None])
        print("Sending address and port")
    else:
        print("Content not found")
        contentSearchString = contentSearchResponse.type

    print(contentSearchString)
        
    client_socket.sendto(contentSearchString.encode(), addr) ##Sends content search response back to peer
    print("Response sent")

def contentListing(client_socket, addr):
    contentListing = PDU() ##A listing PDU is initialized to send content listing to peer
    contentListing.type = 'O'
    print(filenames)
    for file in filenames:
        contentListing.data.append("".join(file)) ##Content names are appended to PDU data portion
    contentListing.data = [file + "X" for file in contentListing.data if file]
    print(contentListing.data)
    contentListingString = contentListing.type + "".join([char for char in contentListing.data if char is not None])
    print(contentListingString)

    client_socket.sendto(contentListingString.encode(), addr) ##Sends content listing response back to peer
    print("Response sent")

def DeregisterContent(client_socket, data, addr):
    peer = data[0]
    content = data[1]
    contentDeregistration = PDU() ##A Deregistration PDU is initialized to send info when deregistration is complete

    print(peernames)
    print(filenames)
    
    names = ["".join(name) for name in peernames if name]

    contents = ["".join(file) for file in filenames if file]
    
    #Checks if peer's content is available, and if so, it is deregistered
    if content in contents:
        contentIndex = names.index(peer)
        del filenames[contentIndex]
        contentDeregistration.type = 'A'
    else:
        contentDeregistration.type = 'E'

    print(peernames)
    print(filenames)

    client_socket.sendto(contentDeregistration.type.encode(), addr) ##Sends content deregistration response back to peer
    print("Response sent")

def Quit(client_socket, data, addr):
    peer = data[0]
    contentDeregistration = PDU()

    print(peernames)
    print(filenames)
    
    names = ["".join(name) for name in peernames if name]

    print(names)
    print(names.index(peer))

    contentDeregistration.type = 'E'

    for name in names:
        if name == peer:
            del filenames[names.index(name)]
            contentDeregistration.type = 'A'
    del peernames[names.index(peer)]

    print(peernames)
    print(filenames)

    client_socket.sendto(contentDeregistration.type.encode(), addr)
    print("Response sent")
    
def handleFileRequest(client_socket):
    try:
        data, addr = client_socket.recvfrom(MAX_DATA_SIZE)
        if not data:
            return
        
        Response = PDU() ##Response PDU is initialized to handle incoming data
        Response.type = data[0:1].decode() ##Parses PDU type
        Response.data = data[1:].decode() ##Parses PDU data
        parts = Response.data.split("\n") ##Splits data by '\n'
        print(Response.type)

        if Response.type == 'R': ##Handles request for content registration
            print(f"Received content registration request for: {data}")
            registerContent(client_socket, parts, addr)
        if Response.type == 'S': ##Handles request for content search and download
            print(f"Received content download request for: {data}")
            contentDownload(client_socket, parts, addr)
        if Response.type == 'O': ##Handles request for content listing
            contentListing(client_socket, addr)
        if Response.type == 'T': ##Handles request for content deregistration
            print(f"Received content deregistration request for: {data}")
            DeregisterContent(client_socket, parts, addr)
        if Response.type == 'Q': ##Handles request for user deregistration
            print(f"Received user quit request for: {data}")
            Quit(client_socket, parts, addr)
    except Exception as e:
        print(f"Error receiving PDU: {e}")

if __name__ == "__main__":

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) ##A UDP socket is initialized for communication with peers
    client_socket.bind(("127.0.0.1", 3000))

    print(f"Server listening on port 3000")

    try:
        while True:
            handleFileRequest(client_socket) ##Peer requests are handled here
    except KeyboardInterrupt:
        print("Server is shutting down")
    finally:
        client_socket.close()

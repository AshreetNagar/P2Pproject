#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_PORT 4000
#define TCP_PORT 5000
#define BUFLEN 256
#define MAX_CONTENTS 100

struct Content {
    char peerName[11];
    char contentName[11];
    char address[22]; // Combined IP address and port ("IP:port")
};

struct Content contents[MAX_CONTENTS]; 

void registerContent(int indexServerSocket, struct sockaddr_in indexServerAddr, const char *peerName, const char *contentName, const char *contentServerAddress, int serverSocket) {
    // Get the port number associated with the server socket s1
    struct sockaddr_in serverAddr;
    socklen_t len = sizeof(serverAddr);
    getsockname(serverSocket, (struct sockaddr *)&serverAddr, &len);
    int serverPort = ntohs(serverAddr.sin_port);
	if (getsockname(serverSocket, (struct sockaddr *)&serverAddr, &len) == -1) {
    perror("Error retrieving port number");
    // Handle the error (e.g., retry, log, or exit the function)
}
    // Formulate the registration PDU data including the port number
    char registrationData[MAX_DATA_SIZE];
    snprintf(registrationData, sizeof(registrationData), "%s %s %s:%d", peerName, contentName, contentServerAddress, serverPort);

    // Formulate and send the content registration PDU to the index server
    struct pdu registration_pdu;
    registration_pdu.type = 'R';
    strncpy(registration_pdu.data, registrationData, sizeof(registration_pdu.data));

    // Send registration PDU to the index server
    if (sendto(indexServerSocket, &registration_pdu, sizeof(registration_pdu), 0, (struct sockaddr *)&indexServerAddr, sizeof(indexServerAddr)) == -1) {
        perror("Error sending content registration PDU to the index server");
        // Handle error - retry or other error handling
    }

    // Receive acknowledgment from the index server
    struct pdu acknowledgment_pdu;
    socklen_t addrLength = sizeof(indexServerAddr);
    if (recvfrom(indexServerSocket, &acknowledgment_pdu, sizeof(acknowledgment_pdu), 0, (struct sockaddr *)&indexServerAddr, &addrLength) == -1) {
        perror("Error receiving acknowledgment from the index server");
        // Handle error - retry or other error handling
    }

    if (acknowledgment_pdu.type == 'A') { 
        printf("Registration is successful!");
    } else if (acknowledgment_pdu.type == 'E') {
        printf("ERROR! Registration is not successful");
    }
}

void deregisterContent(int indexServerSocket, struct sockaddr_in indexServerAddr, const char *peerName, const char *contentName) {
    // Formulate the deregistration PDU data
    char deregistrationData[MAX_DATA_SIZE];
    snprintf(deregistrationData, sizeof(deregistrationData), "%s %s", peerName, contentName);

    // Formulate and send the content deregistration PDU to the index server
    struct pdu deregistration_pdu;
    deregistration_pdu.type = 'T'; // Assuming 'T' represents content deregistration
    strncpy(deregistration_pdu.data, deregistrationData, sizeof(deregistration_pdu.data));

    // Send deregistration PDU to the index server
    if (sendto(indexServerSocket, &deregistration_pdu, sizeof(deregistration_pdu), 0, (struct sockaddr *)&indexServerAddr, sizeof(indexServerAddr)) == -1) {
        perror("Error sending content deregistration PDU to the index server");
        // Handle error - retry or other error handling
    }

    // Receive acknowledgment from the index server
    struct pdu acknowledgment_pdu;
    socklen_t addrLength = sizeof(indexServerAddr);
    if (recvfrom(indexServerSocket, &acknowledgment_pdu, sizeof(acknowledgment_pdu), 0, (struct sockaddr *)&indexServerAddr, &addrLength) == -1) {
        perror("Error receiving acknowledgment from the index server");
        // Handle error - retry or other error handling
    }

    if (acknowledgment_pdu.type == 'A') { 
        printf("Deregistration is successful!");
    } else if (acknowledgment_pdu.type == 'E') {
        printf("ERROR! Deregistration is not successful");
    }
}

void handleContentSearch(int socket, struct sockaddr_in indexServerAddr, const char *contentName) {
    // Formulate and send the content search PDU to the index server
    struct pdu search_pdu;
    search_pdu.type = 'S';
    strncpy(search_pdu.data, contentName, strlen(contentName));

    // Send search PDU to the index server
    if (sendto(socket, &search_pdu, sizeof(search_pdu), 0, (struct sockaddr *)&indexServerAddr, sizeof(indexServerAddr)) == -1) {
        perror("Error sending content search PDU to the index server");
        // Handle error - retry or other error handling
        return;
    }

    // Receive content server addresses or error message from the index server
    struct pdu response_pdu;
    socklen_t addrLength = sizeof(indexServerAddr);
    if (recvfrom(socket, &response_pdu, sizeof(response_pdu), 0, (struct sockaddr *)&indexServerAddr, &addrLength) == -1) {
        perror("Error receiving content server addresses from the index server");
        // Handle error - retry or other error handling
        return;
    }

    // Process the response PDU
    if (response_pdu.type == 'S') {
        printf("Content server addresses: %s\n", response_pdu.data); // Display content server addresses
    } else if (response_pdu.type == 'E') {
        printf("Content search error: %s\n", response_pdu.data); // Display appropriate error message
    }
}

void handleContentListing(int socket, struct sockaddr_in indexServerAddr) {
    // Formulate and send the content listing request PDU to the index server
    struct pdu listing_pdu;
    listing_pdu.type = 'O';

    // Send listing request PDU to the index server
    if (sendto(socket, &listing_pdu, sizeof(listing_pdu), 0, (struct sockaddr *)&indexServerAddr, sizeof(indexServerAddr)) == -1) {
        perror("Error sending content listing request PDU to the index server");
        // Handle error - retry or other error handling
        return;
    }

    // Receive content listing or error message from the index server
    struct pdu response_pdu;
    socklen_t addrLength = sizeof(indexServerAddr);
    if (recvfrom(socket, &response_pdu, sizeof(response_pdu), 0, (struct sockaddr *)&indexServerAddr, &addrLength) == -1) {
        perror("Error receiving content listing from the index server");
        // Handle error - retry or other error handling
        return;
    }

    // Process the response PDU
    if (response_pdu.type == 'O') {
        printf("List of registered contents: %s\n", response_pdu.data); // Display list of registered contents
    } else if (response_pdu.type == 'E') {
        printf("Content listing error: %s\n", response_pdu.data); // Display appropriate error message
    }
}

void handleFileRequest(int serverSocket) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    char buffer[MAX_BUFFER_SIZE];

    printf("Waiting for client to send a request...\n");

    int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                                (struct sockaddr *)&clientAddress, &clientAddressLength);

    if (bytesReceived <= 0) {
        perror("Error receiving request");
        return;
    }

    struct pdu request_pdu;
    request_pdu.type = buffer[0];
    strncpy(request_pdu.data, &buffer[1], bytesReceived - 1);
    request_pdu.data[bytesReceived - 2] = '\0';

    if (request_pdu.type == 'S') {
        printf("Received content search request for: %s\n", request_pdu.data);
        // Handle content search
        handleContentSearch(serverSocket, clientAddress, request_pdu.data);
        return;
    } else if (request_pdu.type == 'O') {
        printf("Received content listing request\n");
        // Handle content listing
        handleContentListing(serverSocket, clientAddress);
        return;
    } else if (request_pdu.type == 'D') {
        printf("Received download request for file: %s\n", request_pdu.data);
        // Handle file transfer
        handleFileTransfer(serverSocket, clientAddress, request_pdu.data);
        return;
    } else {
        printf("Malformed PDU received\n");
        // Respond with an error PDU for malformed requests
        sendErrorPDU(serverSocket, clientAddress, "Malformed PDU");
        return;
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in indexServerAddr;
    int indexServerSocket;    
    int serverSocket,indexServerSocket, serverPort;
    struct sockaddr_in sin, indexServerAddr;
    int port = 3000;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    /* Allocate a socket */
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0)
        fprintf(stderr, "can't creat socket\n");
    /* Bind the socket */
    if (bind(serverSocket, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        fprintf(stderr, "can't bind to %d port\n", port);
    while (1) {
        // Accept client connections and handle file requests
        handleFileRequest(serverSocket);
    }

    // Close the server socket if needed
    close(serverSocket);
    return 0;
}

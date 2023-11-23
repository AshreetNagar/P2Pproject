/* time_client.c - main */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// #include <sys/socket.h>
#include <winsock2.h>
// #include <netinet/in.h>
#include <sys/types.h>
#include "client.h"
// #include <arpa/inet.h>

// #include <netdb.h>

#define MAX_SOCKS 10
#define BUFSIZE 64
#define MSG "Any Message \n"

struct PDU
{
    char type;
    char data[100];
};

/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */

void handleUserInput(char option, int serverSocket, int clientSocket, int address)
{
    struct PDU contentRegistration;
    struct PDU contentSearch;
    struct PDU contentDownload;
    struct PDU contentListing;
    struct PDU contentDeregistration;
    // fd_set rfds, afds;
    // FD_ZERO(&afds);
    // FD_SET(clientSocket, &afds);
    // FD_SET(0, &afds);
    // memcpy(&rfds, &afds, sizeof(rfds));
    // select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

    if (option == '1')
    {
        // printf("Please enter your name: \n");
        // int peer = read(0, contentRegistration.data, 10);
        // contentRegistration.data[peer - 1] = '\0';
        // printf("Please enter your filename :\n");
        // int filename = read(0, contentRegistration.data + peer, 10);
        // contentRegistration.data[peer + filename - 1] = '\0';
        // strncpy(contentRegistration.data, contentRegistration.data + 20, sizeof(address));
        // contentRegistration.data[sizeof(contentRegistration.data) - 1] = '\0';
        // // handleRegisterContent(peer, filename);
        // contentRegistration.type = 'R';
        // printf("%c\n", contentRegistration.type);
        // int i;
        // for(i = 0; i <= sizeof(contentRegistration.data)/sizeof(contentRegistration.data[0]); i++){
        //     printf("%c\n", contentRegistration.data[i]);
        // }
        // write(serverSocket, &contentRegistration, sizeof(contentRegistration));

        // struct PDU contentRegistrationResponse;
        // char RegistrationBuffer[101];
        // int response = read(serverSocket, RegistrationBuffer, sizeof(RegistrationBuffer));
        // if(contentRegistrationResponse.type == 'E'){
        //     printf("Please enter a different name. A user already exists under that name");
        // }else{
        //     printf("%c", contentRegistrationResponse.type);
        //     printf("Content successfully registered");
        // }

        char fileNames[100][11];
        char peerNames[100][11];
        char tempResponseBuf[100];

        struct pdu
        {
            char type;
            char data[100];
        };

        struct pdu loadPdufromBuf(char data[], int responseSize)
        {
            struct pdu returnPdu;
            returnPdu.type = data[0];

            int copySize = responseSize - 1;
            if (copySize > 0)
            {
                strcpy(returnPdu.data, &data[1], copySize);
                returnPdu.data[copySize] = '\0';
            }
            else
            {
                returnPdu.data[0] = '\0';
            }

            return returnPdu;
        }

        struct pdu createPdu(char pduType)
        {
            struct pdu returnPdu;
            returnPdu.type = pduType;
            returnPdu.data[0] = '\0';
            return returnPdu;
        }

        printf("Registration: \n");
        char contName[11] = {0};
        printf("Enter a 10-character content name. Larger names are shortened\n");
        read(0, &contName, 11);
        contName[10] = 0;
        contName[strcspn(contName, "\r\n")] = 0;

        if (fopen(contName, "rb") == NULL)
        {
            printf("File was not found\n");
        }

        strcpy(fileNames[sockArrIndex], contName, 11);

        int contHostSock;
        struct sockaddr_in reg_addr;
        contHostSock = socket(AF_INET, SOCK_STREAM, 0);

        reg_addr.sin_family = AF_INET;
        reg_addr.sin_port = htons(0);
        reg_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        bind(contHostSock, (struct sockaddr *)&reg_addr, sizeof(reg_addr));
        int alen = sizeof(struct sockaddr_in);
        getsockname(contHostSock, (struct sockaddr *)&reg_addr, &alen);
        listen(contHostSock, 5);

        // change up next 12 lines a bit
        struct pdu contentRegistrationPDU = createPdu('R');
        int portLength = (int)((ceil(log10(reg_addr.sin_port)) + 1) * sizeof(char)) - 1;
        printf("host on port %d\n", ntohs(reg_addr.sin_port));
        char portStr[11] = {0};
        contentRegistrationPDU
            snprintf(portStr, portLength + 1, "%d", ntohs(reg_addr.sin_port));
        printf("port string is: %s\n", portStr);
        memcpy(&contentRegistrationPDU.data[0], peerName, 10);
        memcpy(&contentRegistrationPDU.data[10], contName, 10);
        memcpy(&contentRegistrationPDU.data[20], portStr, 11);
        contentRegistrationPDU.data[10 + 10 + portLength] = 0;
        printf("sending pdu with type %c and data %s%s%s \n", contentRegistrationPDU.type, &contentRegistrationPDU.data[0], &contentRegistrationPDU.data[10], &contentRegistrationPDU.data[20]);
        write(s, &contentRegistrationPDU, 10 + 10 + portLength + 2);

        int responseLen = read(s, &tempResponseBuf[0], 100);
        if (responseLen == -1)
        {
            printf("Error reading response, exiting to menu\n");
            break;
        }
        struct pdu response = loadPdufromBuf(tempResponseBuf, responseLen);
        printf("The response pdu type is %c", response.type);
        while (response.type == 'E')
        {
            printf("Enter a new peer name \n");
            read(0, &peerName, 11);
            peerName[10] = 0;
            peerName[strcspn(peerName, "\r\n")] = 0;

            memset(contentRegistrationPDU.data, 0, 100);
            strcpy(&contentRegistrationPDU.data, peerName, 10);
            strcpy(&contentRegistrationPDU.data, contName, 10);
            strcpy(&contentRegistrationPDU.data, portStr, 11);
            contentRegistrationPDU.data[10 + 10 + portLength] = 0;
            write(s, &contentRegistrationPDU, 10 + 10 + portLength + 2);

            responseLen = read(s, &tempResponseBuf[0], 100);
            response = loadPdufromBuf(tempResponseBuf, responseLen);
        }

        sockArr[sockArrIndex] = contHostSock;
        strcpy(peerNames[sockArrIndex], peerName, 11);

        FD_ZERO(&afds);
        FD_SET(sockArr[sockArrIndex], &afds);
        FD_SET(0, &afds);

        strcpy(&rfds, &afds, sizeof(rfds));
        sockArrIndex++;
    }

    if (option == '2')
    {
        printf("Please enter your name: \n");
        int peer = read(0, contentSearch.data, 10);
        contentSearch.data[peer - 1] = '\0';
        printf("Please enter your filename :\n");
        int filename = read(0, contentSearch.data + peer, 10);
        contentSearch.data[peer + filename - 1] = '\0';
        contentSearch.type = 'S';

        struct PDU contentSearchResponse;
        char SearchBuffer[101];
        int data = read(serverSocket, SearchBuffer, sizeof(SearchBuffer)); // Gotta put the socket in the parameters, udp_s is just a placeholder
        contentSearchResponse.type = SearchBuffer[0];
        char contentPeer[11];
        char contentAddress[89];
        if (contentSearchResponse.type == 'E')
        {
            printf("No such content available.");
        }
        else
        {
            strncpy(contentSearchResponse.data, &SearchBuffer[1], data);
            contentSearchResponse.data[data - 1] = '\0';
            // strncpy(contentPeer, &contentSearchResponse.data[0], 11);
            strncpy(contentPeer, contentSearchResponse.data, 10);
            contentPeer[10] = '\0';
            // strcpy(contentAddress, &contentSearchResponse.data[11]);
            strncpy(contentAddress, contentSearchResponse.data + 10, 88);
            contentAddress[88] = '\0';
            contentDownload.type = 'D';
            contentDownload.data[sizeof(contentAddress) + sizeof(contentAddress) - 1] = '\0';
            strncpy(contentDownload.data, contentPeer, sizeof(contentPeer));
            int addressIndex = strlen(contentDownload.data);
            strcpy(contentDownload.data + strlen(contentAddress), contentAddress);
            for (int i = 0; i < sizeof(contentDownload.data) / sizeof(contentDownload.data[0]); i++)
            {
                printf("%c\n", contentDownload.data[i]);
            }
            write(clientSocket, &contentDownload, sizeof(contentDownload)); // Gotta put TCP socket in the parameters, tcp_s is just a placeholder
        }
    }

    if (option == '3')
    {
        contentListing.type = 'O';
        write(serverSocket, &contentListing, sizeof(contentListing));
        struct PDU contentListingResponse;
        char ListBuffer[101];
        int list = read(serverSocket, ListBuffer, sizeof(ListBuffer)); // Should return peer and address to retrieve content
        contentListingResponse.type = ListBuffer[0];
        if (contentListingResponse.type != 'O')
        {
            printf("Error");
        }
        else
        {
            strncpy(contentListingResponse.data, &ListBuffer[1], list);
            printf(contentListingResponse.data); // Gotta change this, let this be for now
        }
    }

    if (option == '4')
    {
        contentDeregistration.type = 'T';
        write(serverSocket, &contentDeregistration, sizeof(contentDeregistration));
        struct PDU contentDeregistrationResponse;
        char DeregistrationBuffer[101];
        int deregistration = read(serverSocket, DeregistrationBuffer, sizeof(DeregistrationBuffer)); // Should return peer and address to retrieve content
        contentDeregistrationResponse.type = DeregistrationBuffer[0];
        if (contentDeregistrationResponse.type != 'A')
        {
            printf("Error");
        }
        else
        {
            printf("File successfully deregistered");
        }
    }
}

void handleRegisterContent(int peer, int filename)
{
    // struct PDU contentRegistration;

    // contentRegistration.type = 'R';
    //     contentRegistration.data[peer+filename-1] = 0; //Needs to be changed
    //     sendto(socket, &contentRegistration, sizeof(contentRegistration));

    //     struct PDU contentRegistrationResponse;
    //     if(contentRegistrationResponse.type == 'E'){
    //         printf("Please enter a different name. A user already exists under that name");
    //     }else{
    //         printf("%c", contentRegistration.type);
    //         printf("Content successfully registered");
    //     }
    while (1)
    {
        break;
    }
}

int main(int argc, char **argv)
{
    char *host = "localhost";
    int port = 3000;
    char now[100];       /* 32-bit integer to hold time	*/
    struct hostent *phe; /* pointer to host information entry	*/
    // struct sockaddr_in sin;	/* an Internet endpoint address		*/
    struct sockaddr_in reg_addr;
    int s, n, type; /* socket descriptor and socket type	*/

    switch (argc)
    {
    case 1:
        break;
    case 2:
        host = argv[1];
    case 3:
        host = argv[1];
        port = atoi(argv[2]);
        break;
    default:
        fprintf(stderr, "usage: UDPtime [host [port]]\n");
        exit(1);
    }

    memset(&reg_addr, 0, sizeof(reg_addr));
    reg_addr.sin_family = AF_INET;
    // reg_addr.sin_port = htons(port);
    reg_addr.sin_port = htons(0);
    reg_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // bind(tcp_s, (struct sockaddr*)&reg_addr, sizeof(reg_addr));

    /* Map host name to IP address, allowing for dotted decimal */
    if (phe = gethostbyname(host))
    {
        memcpy(&reg_addr.sin_addr, phe->h_addr, phe->h_length);
    }
    else if ((reg_addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
        fprintf(stderr, "Can't get host entry \n");

    /* Allocate a UDP socket */
    int udp_s = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_s < 0)
        fprintf(stderr, "Can't create socket \n");

    /* Allocate a TCP socket */
    int tcp_s = socket(AF_INET, SOCK_STREAM, 0);
    bind(tcp_s, (struct sockaddr *)&reg_addr, sizeof(reg_addr));
    if (tcp_s < 0)
        fprintf(stderr, "Can't create socket \n");

    /* Connect the socket */
    if (connect(udp_s, (struct sockaddr *)&reg_addr, sizeof(reg_addr)) < 0)
        fprintf(stderr, "Can't connect to %s %s \n", host, "Time");

    int alen = sizeof(struct sockaddr_in);
    int peerAddress = getsockname(udp_s, (struct sockaddr *)&reg_addr, &alen); // Send during registration

    int sockets[MAX_SOCKS];
    int max_sd, i;

    for (i = 0; i < MAX_SOCKS; ++i)
    {
        if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0) < 0))
        {
            fprintf()
        }
    }

    // fd_set rfds, afds;
    // FD_ZERO(&afds);
    // FD_SET(udp_s, &afds);
    // FD_SET(0, &afds);
    // memcpy(&rfds, &afds, sizeof(rfds));
    // select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

    // while(1){
    int peerBytes;
    int optionBytes;
    char buf[100];
    char option;
    // printf("Please enter your name: \n")
    printf("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n");
    // peerBytes = read(0, buf, 10);
    optionBytes = read(0, buf, sizeof(buf));
    buf[optionBytes] = '\0';
    option = buf[0];
    handleUserInput(option, udp_s, tcp_s, peerAddress);
    // write(1, buf, optionBytes);

    // }
    // (void) write(s, MSG, strlen(MSG));

    // /* Read the time */

    // n = read(s, (char *)&now, sizeof(now));
    // if (n < 0)
    // 	fprintf(stderr, "Read failed\n");
    // write(1, now, n);
    exit(0);
}

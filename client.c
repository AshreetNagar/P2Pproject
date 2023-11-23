/* time_client.c - main */



#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// #include <sys/socket.h>
#include <winsock2.h>                                                                            
// #include <netinet/in.h>
#include <sys/types.h>
// #include <arpa/inet.h>
                                                                                
// #include <netdb.h>

#define MAX_SOCKS 10
#define	BUFSIZE 64
#define	MSG		"Any Message \n"

struct PDU{
		char type;
		char data[100];
	};

/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */

void handleUserInput(char option, int serverSocket, int clientSocket, int address){
		struct PDU contentListing;
		struct PDU contentDeregistration;
        fd_set rfds, afds;
        FD_ZERO(&afds);
        FD_SET(clientSocket, &afds);
        FD_SET(0, &afds);
        memcpy(&rfds, &afds, sizeof(rfds));
        select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
        
    if (option == '1'){
        char filenames[100][11];
        char peernames[100][11];
        char tempResponseBuffer[100];
        // char
        //1. Send peer and content name to index server with S-PDU
        struct PDU contentRegistration;
        printf("Please enter your name: \n");
        int peer = read(0, contentRegistration.data, 10);
        contentRegistration.data[peer - 1] = '\0';
        printf("Please enter your filename :\n");
        int filename = read(0, contentRegistration.data + peer, 10);
        contentRegistration.data[peer + filename - 1] = '\0';

        int	contentServerSocket;
        struct sockaddr_in reg_addr;
        contentServerSocket = socket(AF_INET, SOCK_STREAM, 0);
        reg_addr.sin_family = AF_INET;
        reg_addr.sin_port = htons(0);
        reg_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind(contentServerSocket, (struct sockaddr *)&reg_addr, sizeof(reg_addr));
        int alen = sizeof (struct sockaddr_in);
        getsockname(contentServerSocket, (struct sockaddr *) &reg_addr, &alen);
        listen(contentServerSocket, 5);

        int portLength = (int)((ceil(log10(reg_addr.sin_port)) + 1) * sizeof(char));
        char portStr[11] = {0};
        snprintf(portStr, portLength + 1, "%d", ntohs(reg_addr.sin_port));
        printf("Port string is: %s\n", portStr);
        // strncpy(contentRegistration.data, contentRegistration.data + 20, sizeof(address));
        memcpy(&contentRegistration.data[20],portStr,11);
        contentRegistration.data[sizeof(contentRegistration.data) - 1] = '\0'; 
        // handleRegisterContent(peer, filename);
        contentRegistration.type = 'R';
        printf("%c\n", contentRegistration.type);
        write(serverSocket, &contentRegistration, sizeof(contentRegistration));

        //2. Receive E or A-PDU from index server to verify registration status
        struct PDU contentRegistrationResponse;
        char RegistrationBuffer[101];
        int response = read(serverSocket, RegistrationBuffer, sizeof(RegistrationBuffer));
        // if(contentRegistrationResponse.type == 'E'){
        while(contentRegistrationResponse.type == 'E'){
            printf("Please enter a different name. A user already exists under that name: \n");
            char peerName[11]={0};
            read(0,&peerName,11);
            peerName[10] = 0;
            memset(contentRegistration.data, 0, 100);
            memcpy(contentRegistration.data[0], peerName, 10);
            memcpy(contentRegistration.data[10], filename, 10);
            memcpy(contentRegistration.data[20], portStr, 11);
            contentRegistration.data[sizeof(contentRegistration.data) - 1] = 0;
            write(serverSocket, &contentRegistration, sizeof(contentRegistration));

            int response = read(serverSocket, RegistrationBuffer, sizeof(RegistrationBuffer));
        }

        printf("\ncontent host socket %d\n",contentServerSocket);                    
        socketArray[socketArrayIndex] = contentHostSocket;
        memcpy(peernames[socketArrayIndex], peerName,11);
        FD_ZERO(&afds);
        FD_SET(socketArray[socketArrayIndex], &afds);
        FD_SET(0, &afds); /* Listening on stdin */
        memcpy(&rfds, &afds, sizeof(rfds));
        socketArrayIndex++;
        // else{
        //     printf("%c", contentRegistrationResponse.type);
        //     printf("Content successfully registered");
        // }
    }

    if (option == '2'){
        //1. Send peer and content name to index server using S-PDU
        struct PDU contentSearch;
        printf("Please enter your name: \n");
        int peer = read(0, contentSearch.data, 10);
        contentSearch.data[peer - 1] = '\0';
        printf("Please enter the content name you would like to download :\n");
        int filename = read(0, contentSearch.data + peer, 10);
        contentSearch.data[peer + filename - 1] = '\0';
        contentSearch.type = 'S';
        printf("%c\n", contentSearch.type);
        printf("%s\n", contentSearch.data);
        write(serverSocket, &contentSearch, sizeof(contentSearch));
        
        //2. Parse address and port of content server from response S-PDU
        struct PDU contentSearchResponse;
        char SearchBuffer[101];
        int data = read(serverSocket, SearchBuffer, sizeof(SearchBuffer)); //Gotta put the socket in the parameters, udp_s is just a placeholder
        contentSearchResponse.type = SearchBuffer[0];
        char contentPeer[11];
        char contentAddress[100];
        if (contentSearchResponse.type == 'E'){
            printf("No such content available.");
        }else{
            strncpy(contentSearchResponse.data, &SearchBuffer[1], data);
            contentSearchResponse.data[data - 1] = '\0';
            // strncpy(contentPeer, &contentSearchResponse.data[0], 11);
            strncpy(contentPeer, contentSearchResponse.data[0], 11);
            contentPeer[10] = '\0';
            // strcpy(contentAddress, &contentSearchResponse.data[11]);
            strcpy(contentAddress, contentSearchResponse.data[11]);
            char *host = strtok(contentAddress,":"); //Gotta change delimiter
            printf("got file host ip:%s ", host);
            char *port = strtok(NULL,":"); //Gotta change deliiet
            printf("port:%s\n",port);
            contentAddress[sizeof(contentAddress)] = '\0';

            int 	sd;
            struct	sockaddr_in server;
            struct	hostent		*hp;
            if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Can't creat a socket\n");
                exit(1);
            }

            bzero((char *)&server, sizeof(struct sockaddr_in));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            if (hp = gethostbyname(host)) 
            bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
            else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
            fprintf(stderr, "Can't get server's address\n");
            exit(1);
            }

            /* Connecting to the server */
            if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
            fprintf(stderr, "Can't connect \n");
            exit(1);
            }

            struct PDU contentDownload;
            contentDownload.type = 'D';
            contentDownload.data[sizeof(contentAddress) + sizeof(contentAddress) - 1] = '\0';
            strncpy(contentDownload.data, contentPeer, sizeof(contentPeer));
            int addressIndex = strlen(contentDownload.data);
            strcpy(contentDownload.data + strlen(contentAddress), contentAddress);
            write(clientSocket, &contentDownload, sizeof(contentDownload)); //Gotta put TCP socket in the parameters, clientSocket is just a placeholder
        }
    }

    if (option == '3'){
        contentListing.type = 'O';
        write(serverSocket, &contentListing, sizeof(contentListing));
        struct PDU contentListingResponse;
        char ListBuffer[101];
        int list = read(serverSocket, ListBuffer, sizeof(ListBuffer)); //Should return peer and address to retrieve content
        contentListingResponse.type = ListBuffer[0];
        if (contentListingResponse.type != 'O'){
            printf("Error");
        }else{
            strncpy(contentListingResponse.data, &ListBuffer[1], list);
            printf(contentListingResponse.data); //Gotta change this, let this be for now
        }
    }

    if (option == '4'){
        contentDeregistration.type = 'T';
        write(serverSocket, &contentDeregistration, sizeof(contentDeregistration));
        struct PDU contentDeregistrationResponse;
        char DeregistrationBuffer[101];
        int deregistration = read(serverSocket, DeregistrationBuffer, sizeof(DeregistrationBuffer)); //Should return peer and address to retrieve content
        contentDeregistrationResponse.type = DeregistrationBuffer[0];
        if (contentDeregistrationResponse.type != 'A'){
            printf("Error");
        }else{
            printf("File successfully deregistered");
        }
    }
}

void handleRegisterContent(int peer, int filename){
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
    while(1){
        break;
    }
}

int
main(int argc, char **argv)
{
	char	*host = "localhost";
	int	port = 3000;
	char	now[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	// struct sockaddr_in sin;	/* an Internet endpoint address		*/
	struct sockaddr_in reg_addr;
	int	s, n, type;	/* socket descriptor and socket type	*/


	switch (argc) {
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
        if ( phe = gethostbyname(host) ){
                memcpy(&reg_addr.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (reg_addr.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a UDP socket */
        int udp_s = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_s < 0)
		fprintf(stderr, "Can't create socket \n");

	/* Allocate a TCP socket */
        int tcp_s = socket(AF_INET, SOCK_STREAM, 0);
		bind(tcp_s, (struct sockaddr*)&reg_addr, sizeof(reg_addr));
        if (tcp_s < 0)
		fprintf(stderr, "Can't create socket \n");
                                                                                
    /* Connect the socket */
        if (connect(udp_s, (struct sockaddr *)&reg_addr, sizeof(reg_addr)) < 0)
		fprintf(stderr, "Can't connect to %s %s \n", host, "Time");

		int alen = sizeof(struct sockaddr_in);
		int peerAddress = getsockname(udp_s, (struct sockaddr*)&reg_addr, &alen); //Send during registration
        listen(tcp_s, 5);

        int sockets[MAX_SOCKS];
        int max_sd, i;

        // for(i = 0; i < MAX_SOCKS; ++i){
        //     if((sockets[i] = socket(AF_INET, SOCK_STREAM, 0) < 0)){
        //         fprintf()
        //     }
        // }

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


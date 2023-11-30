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
        reg_addr.sin_port = htons(port);
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
		getsockname(udp_s, (struct sockaddr*)&reg_addr, &alen);

	// while(1){
		int peerBytes;
		int optionBytes;
		char buf[100];
		char option;
		struct PDU contentRegistration;
		struct PDU contentSearch;
		struct PDU contentDownload;
		struct PDU contentListing;
		struct PDU contentDeregistration;
		// printf("Please enter your name: \n")
		printf("1.Content Registration\n2.Content Download\n3.Content Listing\n4.Content Deregistration\n");
		// peerBytes = read(0, buf, 10);
		optionBytes = read(0, buf, sizeof(buf));
		buf[optionBytes] = '\0';
		option = buf[0];
		switch(option){
			case '1':
				printf("Please enter your name: \n");
				int peerBytes_1 = read(0, contentRegistration.data, 10);
				printf("Please enter your filename :\n");
				int filenameSize = read(0, contentRegistration.data, 10); //Needs to be changed
				contentRegistration.type = 'R';
				contentRegistration.data[peerBytes_1+filenameSize-1] = 0; //Needs to be changed
				write(udp_s, &contentRegistration, sizeof(contentRegistration));

				struct PDU contentRegistrationResponse;
				if(contentRegistrationResponse.type == 'E'){
					printf("Please enter a different name. A user already exists under that name");
				}else{
					printf("%c", contentRegistration.type);
					printf("Content successfully registered");
				}
				break;
			case '2':
				printf("Please enter your name: \n");
				int peerBytes = read(0, contentSearch.data, 100); 
				printf("Please enter the name of the content you are looking for:\n");
				int contentName = read(0, contentSearch.data, 100); //Needs to be changed
				printf("\nHello");
				contentSearch.type = 'S';
				contentSearch.data[peerBytes+contentName] = 0; //Needs to be changed
				// contentSearch.data[peerBytes] = 0;
				write(udp_s, &contentSearch, sizeof(contentSearch));
				struct PDU contentSearchResponse;
				char buf[101];
				int data = read(udp_s, buf, sizeof(buf)); //Should return peer and address to retrieve content
				strncpy(contentSearchResponse.data, &buf[1], data);
				contentSearchResponse.type = buf[0];
				for(int i = 0; i <= sizeof(contentSearchResponse.data); i++){
					printf("%c", contentRegistrationResponse.data[i]);
				}
				char contentPeer[11];
				char contentAddress[100];
				if (contentSearchResponse.type == 'E'){
					printf("No such content available.");
				}else{
					strncpy(contentPeer, &contentSearchResponse.data[0], 11);
					strcpy(contentAddress, &contentSearchResponse.data[11]);
					contentDownload.type = 'D';
					contentDownload.data[sizeof(contentPeer)+sizeof(contentAddress)] = 0;
					strncpy(contentDownload.data, contentPeer, sizeof(contentPeer));
					int addressIndex = strlen(contentDownload.data);
					strcpy(contentDownload.data + strlen(contentAddress), contentAddress);
					write(tcp_s, &contentDownload, sizeof(contentDownload));
					break;
				}
				break;
			case '3':
				contentListing.type = 'O';
				write(udp_s, &contentListing, sizeof(contentListing));
				struct PDU contentListingResponse;
				char buf1[101];
				int data1 = read(udp_s, buf, sizeof(buf)); //Should return peer and address to retrieve content
				contentListingResponse.type = buf1[0];
				if (contentListingResponse.type != 'O'){
					printf("Error");
				}else{
					strncpy(contentListingResponse.data, &buf1[1], data1);
					printf(contentListingResponse.data); //Gotta change this, let this be for now
					break;
				}
			case '4':
				contentDeregistration.type = 'T';
				write(udp_s, &contentDeregistration, sizeof(contentDeregistration));
				struct PDU contentDeregistrationResponse;
				char buf2[101];
				int data2 = read(udp_s, buf2, sizeof(buf2)); //Should return peer and address to retrieve content
				contentDeregistrationResponse.type = buf2[0];
				if (contentDeregistrationResponse.type != 'A'){
					printf("Error");
				}else{
					printf("File successfully deregistered");
					break;
				}
			default:
				printf("Error");
		}
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

// switch(option){
// 			case '1':
// 				printf("Please enter your name: \n");
// 				int peerBytes_1 = read(0, contentRegistration.data, 10);
// 				printf("Please enter your filename :\n");
// 				int filenameSize = read(0, contentRegistration.data, 10); //Needs to be changed
// 				contentRegistration.type = 'R';
// 				contentRegistration.data[peerBytes_1+filenameSize-1] = 0; //Needs to be changed
// 				write(udp_s, &contentRegistration, sizeof(contentRegistration));

// 				struct PDU contentRegistrationResponse;
// 				if(contentRegistrationResponse.type == 'E'){
// 					printf("Please enter a different name. A user already exists under that name");
// 				}else{
// 					printf("%c", contentRegistration.type);
// 					printf("Content successfully registered");
// 				}
// 				break;
// 			case '2':
// 				printf("Please enter your name: \n");
// 				int peerBytes = read(0, contentSearch.data, 100); 
// 				printf("Please enter the name of the content you are looking for:\n");
// 				int contentName = read(0, contentSearch.data, 100); //Needs to be changed
// 				printf("\nHello");
// 				contentSearch.type = 'S';
// 				contentSearch.data[peerBytes+contentName] = 0; //Needs to be changed
// 				// contentSearch.data[peerBytes] = 0;
// 				write(udp_s, &contentSearch, sizeof(contentSearch));
// 				struct PDU contentSearchResponse;
// 				char buf[101];
// 				int data = read(udp_s, buf, sizeof(buf)); //Should return peer and address to retrieve content
// 				strncpy(contentSearchResponse.data, &buf[1], data);
// 				contentSearchResponse.type = buf[0];
// 				for(int i = 0; i <= sizeof(contentSearchResponse.data); i++){
// 					printf("%c", contentRegistrationResponse.data[i]);
// 				}
// 				char contentPeer[11];
// 				char contentAddress[100];
// 				if (contentSearchResponse.type == 'E'){
// 					printf("No such content available.");
// 				}else{
// 					strncpy(contentPeer, &contentSearchResponse.data[0], 11);
// 					strcpy(contentAddress, &contentSearchResponse.data[11]);
// 					contentDownload.type = 'D';
// 					contentDownload.data[sizeof(contentPeer)+sizeof(contentAddress)] = 0;
// 					strncpy(contentDownload.data, contentPeer, sizeof(contentPeer));
// 					int addressIndex = strlen(contentDownload.data);
// 					strcpy(contentDownload.data + strlen(contentAddress), contentAddress);
// 					write(tcp_s, &contentDownload, sizeof(contentDownload));
// 					break;
// 				}
// 				break;
// 			case '3':
// 				contentListing.type = 'O';
// 				write(udp_s, &contentListing, sizeof(contentListing));
// 				struct PDU contentListingResponse;
// 				char buf1[101];
// 				int data1 = read(udp_s, buf, sizeof(buf)); //Should return peer and address to retrieve content
// 				contentListingResponse.type = buf1[0];
// 				if (contentListingResponse.type != 'O'){
// 					printf("Error");
// 				}else{
// 					strncpy(contentListingResponse.data, &buf1[1], data1);
// 					printf(contentListingResponse.data); //Gotta change this, let this be for now
// 					break;
// 				}
// 			case '4':
// 				contentDeregistration.type = 'T';
// 				write(udp_s, &contentDeregistration, sizeof(contentDeregistration));
// 				struct PDU contentDeregistrationResponse;
// 				char buf2[101];
// 				int data2 = read(udp_s, buf2, sizeof(buf2)); //Should return peer and address to retrieve content
// 				contentDeregistrationResponse.type = buf2[0];
// 				if (contentDeregistrationResponse.type != 'A'){
// 					printf("Error");
// 				}else{
// 					printf("File successfully deregistered");
// 					break;
// 				}
// 			default:
// 				printf("Error");
// 		}
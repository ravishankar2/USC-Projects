#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "global.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void getrequestdoc(int client_id, char *request_doc) {
	switch(client_id) {
		case 1: strcpy(request_doc, "doc1"); break;
		case 2: strcpy(request_doc, "doc2"); break;
		// we can expand doc file
	}
}

void getclientport(int client_id, char *client_port) {
	switch(client_id) {
		case 1: strcpy(client_port, CLIENT1_PHASE2_PORT); break;
		case 2: strcpy(client_port, CLIENT2_PHASE2_PORT); break;
		// we can expand client
	}
}

void getfileservername(char *message, char *file_server_name) {
	// File_Server1 22537
	char *ptr = strchr(message, ' ');
	int length = ptr - message;
	strncpy(file_server_name, message, length);
	file_server_name[length] = '\0';
	// printf("%s\n", file_server_name);
}

void getfileserverport(char *message, char *file_server_port) {
	// File_Server1 22537
	char *ptr = strchr(message, ' ');
	int index = ptr - message + 1;
	int length = strlen(message) - index;
	strncpy(file_server_port, message + index, length);
	file_server_port[length] = '\0';
	// printf("%s\n", file_server_port);
}

int main(void) {
	struct addrinfo hints, *directory_info, *p;
	int phase2_socket, phase3_socket;
	int rv, bc, status = 0, i, k;
	socklen_t fromlen;
	struct sockaddr_storage file_addr;
	char buffer[MAX_DATA_SIZE];
	struct hostent *he;
	struct in_addr **addr_list;
	char *host_addr;
	pid_t wpid;

	/*************************************************** Phase 2 ***************************************************/

	// get host addr
	he = gethostbyname(HOST_NAME);
	addr_list = (struct in_addr **)he->h_addr_list;
	for(k = 0; addr_list[k] != NULL; k++) {
		host_addr = inet_ntoa(*addr_list[k]);
	}

    i = 0;
    while(i++ < CLIENT_COUNT) {  // create 2 clients process
    	if(!fork()) {	// this is the child process
    		char client_id[10];	// client id
    		char request_doc[10]; // requested doc file
    		char client_port[10];	// client port
    		sprintf(client_id, "%d", i); // convert int to string
    		char request_message[MAX_DATA_SIZE] = "Client";	// register_message
    		strcat(request_message, client_id);	// append client id
    		strcat(request_message, " ");
    		getrequestdoc(i, request_doc); // get requested doc file according to client
    		strcat(request_message, request_doc);	// append requested doc file
    		getclientport(i, client_port);
    		// printf("%s\n", request_message);

    		struct addrinfo h1, *f1, *q1;
			int s;
			memset(&h1, 0, sizeof h1);
			h1.ai_family = AF_UNSPEC;
			h1.ai_socktype = SOCK_DGRAM;
			h1.ai_flags = AI_PASSIVE; // use my IP

			// printf("%s\n", file_server_udpport);
			if((rv = getaddrinfo(NULL, client_port, &h1, &f1)) != 0) {
				fprintf(stderr, "Error: phase 2 getaddrinfo %s\n", gai_strerror(rv));
		        return 1;
			}

			// loop through all the results and bind to the first we can
			for(q1 = f1; q1 != NULL; q1 = q1->ai_next) {
				if ((s = socket(q1->ai_family, q1->ai_socktype, q1->ai_protocol)) == -1) {
		            perror("Error: phase 2 client socket");
		            continue;
		        }

				if (bind(s, q1->ai_addr, q1->ai_addrlen) == -1) {
					close(s);
					perror("Error: phase 2 client bind");
					continue;
				}

				break;
			}

			if (q1 == NULL) {
				fprintf(stderr, "Error: phase 2 client failed to bind socket\n");
				return 2;
			}

			freeaddrinfo(f1);

			printf("Phase 2: Client %s has UDP port number %s and IP address %s.\n", client_id, client_port, host_addr);
			close(s);

			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;

			if ((rv = getaddrinfo(host_addr, DIRECTORY_PHASE2_PORT, &hints, &directory_info)) != 0) {
		        fprintf(stderr, "Error: phase 2 getaddrinfo %s\n", gai_strerror(rv));
		        return 1;
		    }

		    // loop through all the results and make a socket
		    for(p = directory_info; p != NULL; p = p->ai_next) {
		        if ((phase2_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		            perror("Error: phase 2 client socket");
		            continue;
		        }

		        break;
		    }

		    if (p == NULL) {
		        fprintf(stderr, "Error: phase 2 client failed to bind socket\n");
		        return 2;
		    }

		    freeaddrinfo(directory_info);

		    // send register message using udp
			if((bc = sendto(phase2_socket, request_message, strlen(request_message)+1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
				perror("Error: phase 2 file server sendto");
        		exit(1);
			}

		    printf("Phase 2: The File request from Client %s has been sent to the Directory Server.\n", client_id);

		    fromlen = sizeof file_addr;
			if((bc = recvfrom(phase2_socket, buffer, sizeof buffer, 0, (struct sockaddr *)&file_addr, &fromlen)) == -1) {
				perror("Error: phase 2 file server recvfrom");
		       	exit(1);
			}
			buffer[bc] = '\0';
			// printf("%s\n", buffer); // File_Server1 22537
			char file_server_name[20];	// file server name
			char file_server_port[10];	// file server port
			getfileservername(buffer, file_server_name);
			getfileserverport(buffer, file_server_port);

			printf("Phase 2: The File requested by Client %s is present in %s and the File Server’s TCP port number is %s.\n", client_id, file_server_name, file_server_port);

			printf("Phase 2: End of Phase 2 for Client %s.\n", client_id);
			close(phase2_socket);

			/*************************************************** Phase 3 ***************************************************/

			struct addrinfo h3, *f3, *q3;

			memset(&h3, 0, sizeof h3);
			h3.ai_family = AF_UNSPEC;
			h3.ai_socktype = SOCK_STREAM;

			if ((rv = getaddrinfo(host_addr, file_server_port, &h3, &f3)) != 0) {
				fprintf(stderr, "Error: phase 3 getaddrinfo %s\n", gai_strerror(rv));
				return 1;
			}

			// loop through all the results and connect to the first we can
			for(q3 = f3; q3 != NULL; q3 = q3->ai_next) {
				if ((phase3_socket = socket(q3->ai_family, q3->ai_socktype, q3->ai_protocol)) == -1) {
					perror("Error: phase 3 client socket");
					continue;
				}

				if (connect(phase3_socket, q3->ai_addr, q3->ai_addrlen) == -1) {
					close(phase3_socket);
					perror("Error: phase 3 client connect");
					continue;
				}

				break;
			}

			if (q3 == NULL) {
				fprintf(stderr, "Error: phase 3 client failed to connect\n");
				return 2;
			}

			// get dynamic address
			char address[INET6_ADDRSTRLEN];
			inet_ntop(q3->ai_family, get_in_addr((struct sockaddr *)q3->ai_addr),address, sizeof address);
			// get dynamic port
			int port;
			struct sockaddr_in sin;
			socklen_t len = sizeof(sin);
			if (getsockname(phase3_socket, (struct sockaddr *)&sin, &len) == -1)
				perror("Error: phase 3 getsockname");
			else
    			port = ntohs(sin.sin_port);

    		printf("Phase 3: Client %s has dynamic TCP port number %d and IP address %s.\n", client_id, port, address);

			freeaddrinfo(f3); // all done with this structure

			// send to doc file request to file server
			// printf("send %s\n", request_message);	// Client1 doc1
			if (send(phase3_socket, request_message, strlen(request_message)+1, 0) == -1)
				perror("Error: phase 3 send");

			printf("Phase 3: The File request from Client %s has been sent to the %s\n", client_id, file_server_name);

			if ((bc = recv(phase3_socket, buffer, MAX_DATA_SIZE-1, 0)) == -1) {
			    perror("Error: phase 3 recv");
			    exit(1);
			}

			// printf("receive %s\n", buffer);

			printf("Phase 3: Client %s received %s from %s.\n", client_id, request_doc, file_server_name);

			printf("Phase 3: End of Phase 3 for Client %s.\n", client_id);

			close(phase3_socket);

    		exit(0);
    	}
    }
    while ((wpid = wait(&status)) > 0);	// wait for chile process
}




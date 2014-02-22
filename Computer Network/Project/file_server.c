#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "global.h"

void getfileserverudpport(int file_server_id, char *file_server_port) {
	switch(file_server_id) {
		case 1: strcpy(file_server_port, FILE1_PHASE1_PORT); break;
		case 2: strcpy(file_server_port, FILE2_PHASE1_PORT); break;
		case 3: strcpy(file_server_port, FILE3_PHASE1_PORT); break;
		// we can expand file server
	}
}

void getfileservertcpport(int file_server_id, char *file_server_port) {
	switch(file_server_id) {
		case 1: strcpy(file_server_port, FILE1_PHASE3_PORT); break;
		case 2: strcpy(file_server_port, FILE2_PHASE3_PORT); break;
		case 3: strcpy(file_server_port, FILE3_PHASE3_PORT); break;
		// we can expand file server
	}
}

void getclientname(char *message, char *client_name) {
	// Client1 doc1
	char *ptr = strchr(message, ' ');
	int length = ptr - message;
	strncpy(client_name, message, length);
	client_name[length] = '\0';
	// printf("%s\n", client_name);
}

void getdocfilename(char *message, char *docfile_name) {
	// Client1 doc1
	char *ptr = strchr(message, ' ');
	int index = ptr - message + 1;
	int length = strlen(message) - index;
	strncpy(docfile_name, message + index, length);
	docfile_name[length] = '\0';
	// printf("%s\n", docfile_name);
}

void sigchld_handler(int s) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(void)
{
	struct addrinfo hints, *directory_info, *p;
	struct sockaddr_storage client_addr; // connector's address information
	socklen_t fromlen;
	int phase1_socket, phase3_socket, new_connect;
	int rv, bc, yes = 1, status = 0, i;
	char buffer[MAX_DATA_SIZE];
	struct sigaction sa;
	struct hostent *he;
	struct in_addr **addr_list;
	char *host_addr;
	pid_t wpid;

	/*************************************************** Phase 1 ***************************************************/

	// get host addr
	he = gethostbyname(HOST_NAME);
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
    	host_addr = inet_ntoa(*addr_list[i]);
    }

	i = 0;
	while(i++ < FILE_SERVER_COUNT) {  // create 3 file server process
		if(!fork()) {	// parent process wait for child process
			char file_server_id[10];	// file server id
			char file_server_tcpport[10];	// file server tcp port
			char file_server_udpport[10];	// file server udp port
			sprintf(file_server_id, "%d", i);	// convert int to string
			char register_message[MAX_DATA_SIZE] = "File_Server";	// register_message
			strcat(register_message, file_server_id);	// append file server id
			strcat(register_message, " ");
			getfileservertcpport(i, file_server_tcpport);	// get tcp port according id
			strcat(register_message, file_server_tcpport);	// append static port
			getfileserverudpport(i, file_server_udpport);	// get udp port according id
		
			struct addrinfo h1, *f1, *q1;
			int s;
			memset(&h1, 0, sizeof h1);
			h1.ai_family = AF_UNSPEC;
			h1.ai_socktype = SOCK_DGRAM;
			h1.ai_flags = AI_PASSIVE; // use my IP			

			// printf("%s\n", file_server_udpport);
			if((rv = getaddrinfo(NULL, file_server_udpport, &h1, &f1)) != 0) {
				fprintf(stderr, "Error: phase 1 getaddrinfo %s\n", gai_strerror(rv));
		        return 1;
			}

			// loop through all the results and bind to the first we can
			for(q1 = f1; q1 != NULL; q1 = q1->ai_next) {
				if ((s = socket(q1->ai_family, q1->ai_socktype, q1->ai_protocol)) == -1) {
		            perror("Error: phase 1 file server socket");
		            continue;
		        }

				if (bind(s, q1->ai_addr, q1->ai_addrlen) == -1) {
					close(s);
					perror("Error: phase 1 file server bind");
					continue;
				}

				break;
			}

			if (q1 == NULL) {
				fprintf(stderr, "Error: phase 1 file server failed to bind socket\n");
				return 2;
			}

			freeaddrinfo(f1);

			printf("Phase 1: File Server %s has UDP port number %s and IP address %s.\n", file_server_id, file_server_udpport, host_addr);
			close(s);
			// printf("%s\n", register_message);

			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_DGRAM;

			if ((rv = getaddrinfo(host_addr, DIRECTORY_PHASE1_PORT, &hints, &directory_info)) != 0) {
		        fprintf(stderr, "Error: phase 1 getaddrinfo %s\n", gai_strerror(rv));
		        return 1;
		    }

		    // loop through all the results and make a socket
		    for(p = directory_info; p != NULL; p = p->ai_next) {
		        if ((phase1_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		            perror("Error: phase 1 file server socket");
		            continue;
		        }

		        break;
		    }

		    if (p == NULL) {
		        fprintf(stderr, "Error: phase 1 file server failed to bind socket\n");
		        return 2;
		    }

		    freeaddrinfo(directory_info);

			// send register message using udp
			if((bc = sendto(phase1_socket, register_message, strlen(register_message)+1, 0, p->ai_addr, p->ai_addrlen)) == -1) {
				perror("Error: phase 1 file server sendto");
	    	  		exit(1);
			}

			printf("Phase 1: The Registration request from File Server %s has been sent to the Directory Server.\n", file_server_id);
			close(phase1_socket);

			/*************************************************** Phase 3 ***************************************************/

			struct addrinfo h3, *f3, *q3;
			memset(&h3, 0, sizeof h3);
			h3.ai_family = AF_UNSPEC;
			h3.ai_socktype = SOCK_STREAM;
			h3.ai_flags = AI_PASSIVE; // use my IP

			if ((rv = getaddrinfo(NULL, file_server_tcpport, &h3, &f3)) != 0) {
				fprintf(stderr, "Error: phase 3 getaddrinfo %s\n", gai_strerror(rv));
				return 1;
			}

			// loop through all the results and bind to the first we can
			for(q3 = f3; q3 != NULL; q3 = q3->ai_next) {
				if ((phase3_socket = socket(q3->ai_family, q3->ai_socktype, q3->ai_protocol)) == -1) {
					perror("Error: phase 3 file server socket");
					continue;
				}
	
				if (setsockopt(phase3_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
					perror("Error: phase 3 setsockopt");
					exit(1);
				}
	
				if (bind(phase3_socket, q3->ai_addr, q3->ai_addrlen) == -1) {
					close(phase3_socket);
					perror("Error: phase 3 file server bind");
					continue;
				}

				break;
			}

			if (q3 == NULL)  {
				fprintf(stderr, "Error: phase 3 file server failed to bind\n");
				return 2;
			}

			if (listen(phase3_socket, BACKLOG) == -1) {
				perror("Error: phase 3 listen");
				exit(1);
			}

			sa.sa_handler = sigchld_handler; // reap all dead processes
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = SA_RESTART;
			if (sigaction(SIGCHLD, &sa, NULL) == -1) {
				perror("Error: phase 3 sigaction");
				exit(1);
			}

			freeaddrinfo(f3); // all done with this structure

			printf("Phase 3: File Server %s has TCP port %s and IP address %s.\n", file_server_id, file_server_tcpport, host_addr);

			// wait for connections
			while(1) {  // main accept() loop

				fromlen = sizeof client_addr;
				if((new_connect = accept(phase3_socket, (struct sockaddr *)&client_addr, &fromlen)) == -1) {
					// perror("Error: phase 3 accept");
					continue;
				}

				if ((bc = recv(new_connect, buffer, MAX_DATA_SIZE-1, 0)) == -1) {
				    perror("Error: phase 3 recv");
				    exit(1);
				}

				if (!fork()) { // this is the child process
					close(phase3_socket); // child doesn't need the listener

					// printf("receive %s\n", buffer);	// Client1 doc1
					char client_name[10];
					char doc_file_name[10];
					getdocfilename(buffer, doc_file_name);
					getclientname(buffer, client_name);

					printf("Phase 3: File Server %s received the request from the %s for the file %s.\n", file_server_id, client_name, doc_file_name);

					// send the file
					if (send(new_connect, doc_file_name, strlen(doc_file_name)+1, 0) == -1)
						perror("Error: phase 3 send");

					printf("Phase 3: File Server %s has sent %s to %s.\n", file_server_id, doc_file_name, client_name);

					close(new_connect);
					exit(0);
				}
				close(new_connect);  // parent doesn't need this
			}

			exit(0);
		}
	}
	while ((wpid = wait(&status)) > 0);	// wait for chile process
	return 0;
}




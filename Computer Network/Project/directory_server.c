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

// struct to doc file info
struct docfileinfo {
	char doc_file_name[10];
	struct docfileinfo *next_file;
};

// struct to store file server info
struct fileserverinfo {
	char file_server_id[10];
	char file_server_name[20];
	char file_server_port[10];
	struct docfileinfo *docfile;
};

// struct to store client info
struct clientinfo {
	char client_id[10];
	char client_name[10];
	int to_file_server_cost[FILE_SERVER_COUNT];
};

void getclientid(char *message, char *client_id) {
	// Client1 doc1
	int name_length = 6;
	char *ptr = strchr(message, ' ');
	int index = ptr - message;
	int length = index - name_length;
	strncpy(client_id, message + name_length, length);
	client_id[length] = '\0';
}

void getclientname(char *message, char *client_name) {
	// Client1 doc1
	char *ptr = strchr(message, ' ');
	int length = ptr - message;
	strncpy(client_name, message, length);
	client_name[length] = '\0';
	// printf("%s\n", client_name);
}

void getrequestfilename(char *message, char *file_name) {
	// Client1 doc1
	char *ptr = strtok(message, " ");
	if(ptr) ptr = strtok(NULL, " ");
	strncpy(file_name, ptr, strlen(ptr)+1);
	// printf("%s\n", file_name);
}

void getfileserverid(char *message, char *file_server_id) {
	// File_Server1 22537
	// File_Server1 2 doc1 doc2
	int name_length = 11;
	char *ptr = strchr(message, ' ');
	int index = ptr - message;
	int length = index - name_length;
	strncpy(file_server_id, message + name_length, length);
	file_server_id[length] = '\0';
	// printf("%s\n", file_server_id);
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
	int phase1_socket, phase2_socket;
	int rv, bc, i, k;
	socklen_t fromlen;
	struct sockaddr_storage file_addr;
	char buffer[MAX_DATA_SIZE];
	struct hostent *he;
	struct in_addr **addr_list;
	char *host_addr;
	FILE *fp;
	struct fileserverinfo fileserver_list[FILE_SERVER_COUNT];
	struct clientinfo client_list[CLIENT_COUNT];

	/*************************************************** Phase 1 ***************************************************/

	// remove directory.txt first
	remove(DIRECTORY_TXT);

	// get host addr
	he = gethostbyname(HOST_NAME);
    addr_list = (struct in_addr **)he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++) {
    	host_addr = inet_ntoa(*addr_list[i]);
    }

	// get host info, make socket, bind it to port 21537
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if((rv = getaddrinfo(NULL, DIRECTORY_PHASE1_PORT, &hints, &directory_info)) != 0) {
		fprintf(stderr, "Error: phase 1 getaddrinfo %s\n", gai_strerror(rv));
        return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = directory_info; p != NULL; p = p->ai_next) {
		if ((phase1_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Error: phase 1 directory server socket");
            continue;
        }

        if (bind(phase1_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(phase1_socket);
            perror("Error: phase 1 directory server bind");
            continue;
        }

        break;
	}

	if (p == NULL) {
        fprintf(stderr, "Error: phase 1 directory server failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(directory_info);

	printf("Phase 1: The Directory Server has UDP port number %s and IP address %s.\n", DIRECTORY_PHASE1_PORT, host_addr);

	i = 0;
	while(i++ < FILE_SERVER_COUNT) {  // waiting for 3 file_server process
		// no need to accept(), just recvfrom():
		fromlen = sizeof file_addr;
		if((bc = recvfrom(phase1_socket, buffer, sizeof buffer, 0, (struct sockaddr *)&file_addr, &fromlen)) == -1) {
			perror("Error: phase 1 directory server recvfrom");
        	exit(1);
		}
		buffer[bc] = '\0';
		char file_server_id[10];	// file server id
		char file_server_name[20];	// file server name
		char file_server_port[10];	// file server port
		getfileserverid(buffer, file_server_id);
		int sid = atoi(file_server_id) - 1;
		getfileservername(buffer, file_server_name);
		getfileserverport(buffer, file_server_port);
		// store file server info into struct
		strncpy(fileserver_list[sid].file_server_id, file_server_id, strlen(file_server_id)+1);
		strncpy(fileserver_list[sid].file_server_name, file_server_name, strlen(file_server_name)+1);
		strncpy(fileserver_list[sid].file_server_port, file_server_port, strlen(file_server_port)+1);
		fileserver_list[sid].docfile = NULL;
		printf("Phase 1: The Directory Server has received request from File Server %s.\n",file_server_id);
		// write directory.txt
		fp = fopen(DIRECTORY_TXT, "a");
		fputs(buffer, fp);
		fputs("\n", fp);
		fclose(fp);
	}
	close(phase1_socket);

	printf("Phase 1: The directory.txt file has been created.\n");

	/*************************************************** Phase 2 ***************************************************/

	// load resource.txt
	fp = fopen(RESOURCE_TXT, "rt");	// open file for reading text
	while(fgets(buffer, MAX_DATA_SIZE, fp) != NULL) {
		// remove '\n\r'
		int length = strlen(buffer)-1;
		if (buffer[length] == '\n') buffer[--length] = '\0';
		if (buffer[length] == '\r') buffer[--length] = '\0';
		// printf("%s\n", buffer);
		// File_Server1 2 doc1 doc2
		char file_server_id[10];	// file server id
		getfileserverid(buffer, file_server_id);
		int sid = atoi(file_server_id) - 1;
		char *ptr = strtok(buffer, " ");
		i = 0;
		while(ptr) {
			// skip file server name and number
			if(i++ < 2) {
				ptr = strtok(NULL, " ");
				continue;
			}
			// printf("%s\n", ptr);
			// need malloc
			struct docfileinfo *docfile = (struct docfileinfo *) malloc (sizeof(struct docfileinfo));
			// store doc file server into file server
			strncpy(docfile->doc_file_name, ptr, strlen(ptr)+1);
			// printf("%s\n", docfile->doc_file_name);
			// set corresponding file server's doc file
			docfile->next_file = fileserver_list[sid].docfile;
			fileserver_list[sid].docfile = docfile;
			// printf("%s\n", fileserver_list[sid].docfile->doc_file_name);
			ptr = strtok(NULL, " ");
		}
	}
	fclose(fp);

	// for(i = 0; i < FILE_SERVER_COUNT; i++) {
	// 	printf("%s: ", fileserver_list[i].file_server_name);
	// 	struct docfileinfo *pdoc = fileserver_list[i].docfile;
	// 	while(pdoc != NULL) {
	// 		printf("%s ", pdoc->doc_file_name);
	// 		pdoc = pdoc->next_file;
	// 	}
	// 	printf("\n");
	// }

	// load topology.txt
	fp = fopen(TOPOLOGY_TXT, "rt");	// open file for reading text
	i = 0;
	while(fgets(buffer, MAX_DATA_SIZE, fp) != NULL) {
		// remove '\n\r'
		int length = strlen(buffer)-1;
		if (buffer[length] == '\n') buffer[--length] = '\0';
		if (buffer[length] == '\r') buffer[--length] = '\0';
		// printf("%s\n", buffer);
		char *ptr = strtok(buffer, " ");
		k = 0;
		while(ptr) {
			int cost = atoi(ptr);
			client_list[i].to_file_server_cost[k++] = cost;
			ptr = strtok(NULL, " ");
		}
		i++;
	}
	fclose(fp);

	// for(i = 0; i < CLIENT_COUNT; i++) {
	// 	int k;
	// 	for(k = 0; k < FILE_SERVER_COUNT; k++) {
	// 		printf("%d ", client_list[i].to_file_server_cost[k]);
	// 	}
	// 	printf("\n");
	// }

	// get host info, make socket, bind it to port 21537
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if((rv = getaddrinfo(NULL, DIRECTORY_PHASE2_PORT, &hints, &directory_info)) != 0) {
		fprintf(stderr, "Error: phase 2 getaddrinfo %s\n", gai_strerror(rv));
        return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = directory_info; p != NULL; p = p->ai_next) {
		if ((phase2_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Error: phase 2 directory server socket");
            continue;
        }

        if (bind(phase2_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(phase2_socket);
            perror("Error: phase 2 directory server bind");
            continue;
        }

        break;
	}

	if (p == NULL) {
        fprintf(stderr, "Error: phase 2 directory server failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(directory_info);

	printf("Phase 2: The Directory Server has UDP port number %s and IP address %s.\n", DIRECTORY_PHASE2_PORT, host_addr);

	i = 0;
	while(i++ < CLIENT_COUNT) {
		// no need to accept(), just recvfrom():
		fromlen = sizeof file_addr;
		if((bc = recvfrom(phase2_socket, buffer, sizeof buffer, 0, (struct sockaddr *)&file_addr, &fromlen)) == -1) {
			perror("Error: phase 2 directory server recvfrom");
	       	exit(1);
		}
		buffer[bc] = '\0';

		// printf("%s\n", buffer);	// Client1 doc1

		char client_id[10];	// client id
		char client_name[20];	// client name
		char request_file_name[10];	// request file
		getclientid(buffer, client_id);
		int cid = atoi(client_id) - 1;
		getclientname(buffer, client_name);
		getrequestfilename(buffer, request_file_name);

		// printf("%s\n", request_file_name);	// doc1

		printf("Phase 2: The Directory Server has received request from Client %s.\n", client_id);

		// search all the file server
		int mincost = -1;
		int contained_server_id;
		for(k = 0; k < FILE_SERVER_COUNT; k++) {
			struct docfileinfo *docfile = fileserver_list[k].docfile;
			// search the file contained
			// printf("request_file_name %s\n", request_file_name);
			while(docfile) {
				if(strcmp(docfile->doc_file_name, request_file_name) == 0) {	// find the doc file
					// printf("find in %s\n", fileserver_list[k].file_server_name);
					// get client to server cost
					int cost = client_list[cid].to_file_server_cost[k];
					// printf("its cost is %d\n", cost);
					// printf("%d\n", cost);
					if(mincost == -1 || cost < mincost) {	// find less cost server which contain that file
						mincost = cost;
						contained_server_id = k;
					}
					break;
				}
				docfile = docfile->next_file;
			}
		}

		char response_message[MAX_DATA_SIZE];
		memset(response_message, 0, sizeof response_message);
		strcat(response_message, fileserver_list[contained_server_id].file_server_name);	// append file server name
		strcat(response_message, " ");
		strcat(response_message, fileserver_list[contained_server_id].file_server_port);	// append file server port

		// printf("%s\n", response_message);

		if((bc = sendto(phase2_socket, response_message, strlen(response_message)+1, 0, (struct sockaddr *)&file_addr, fromlen)) == -1) {
			perror("Error: phase 2 directory server sendto");
	        exit(1);
		}

		printf("Phase 2: File server details has been sent to Client %s.\n", client_id);
	}

	close(phase2_socket);

	printf("Phase 2: End of Phase 2 for the Directory Server.\n");

	return 0;
}




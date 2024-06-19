#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>



void *handle_request(void *socket_desc){
	int fd = *(int *)socket_desc;
	free(socket_desc);
	
	char buffer[1024]={0};

	//recieve msg
	int msg_Read = read(fd, buffer, 1024);
	if (msg_Read<0){
		printf("read failed");
		
	}
	printf("Received HTTP request:\n%s\n", buffer);

	//Extract URL
	char method[16], url[256], protocol[16];
	sscanf(buffer,"%s %s %s", method,url,protocol);
	printf("URL: %s\n", url);

	char response[1024];

	if (strcmp(url, "/") == 0){
		snprintf(response, sizeof(response),"HTTP/1.1 200 OK\r\n\r\n\r\n");
	}

	else if(strncmp(url,"/echo/",6) == 0){
		char *echo_msg = url + 6;
		snprintf(response, sizeof(response),"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",strlen(echo_msg),echo_msg);
	}

	else if(strncmp(url,"/user-agent",11) == 0){
		char *user_agent = strstr(buffer, "User-Agent:");
		if(user_agent){
			user_agent+=12;
			char *eol = strstr(user_agent,"\r\n");
			if(eol)*eol = '\0';
			
		}
		else{
			user_agent = "User-Agent not found";
		}
		printf("user-agent: %s", user_agent);
		snprintf(response, sizeof(response),"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",strlen(user_agent),user_agent);
	}
	
	else{
		snprintf(response, sizeof(response),"HTTP/1.1 404 Not Found\r\n\r\n\r\n");
	}

	write(fd, response, sizeof(response) - 1);

	close(fd);

}

int main() {
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);
	

	printf("Logs from your program will appear here!\n");

	int server_fd, client_addr_len, *fd;
	struct sockaddr_in client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	
	fd = malloc(sizeof(int));
	*fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  	printf("Client connected\n");

	pthread_t thread_id;
	if(pthread_create(&thread_id, NULL, handle_request,(void*)fd) < 0){
		printf("Could not create thread");
		free(fd);
		return 1;
	}

	close(server_fd);

	return 0;
}

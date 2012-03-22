#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "glib.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setup_server_socket(char * ports){
	int sockfd=0;
	struct addrinfo hints;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	int rv=0;
	struct addrinfo *servinfo=NULL;
	if ((rv = getaddrinfo(NULL, ports, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	struct addrinfo *p=NULL;
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		int yes=1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return -2;
	}
	freeaddrinfo(servinfo);

	return sockfd;
}

void main_loop(int sockfd){
	if (listen(sockfd, 10) == -1) {
		perror("listen");
		exit(1);
	}

	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int newfd=0;
	while(1) { // main accept() loop
		sin_size = sizeof(their_addr);
		newfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newfd == -1) {
			perror("accept");
			continue;
		}
		char s[INET6_ADDRSTRLEN];
		inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		void serve_client(int newfd);
		serve_client(newfd);

		shutdown(newfd, SHUT_RDWR);
		close(newfd);
	}
	return;
}

void serve_client(int newfd){
	GIOChannel *gio=NULL;
	gio=g_io_channel_unix_new(newfd);
	assert(gio);

	const gchar *msg="Hello world from server\n";
	gsize written=0;
	g_io_channel_write_chars(gio, msg, strlen(msg), &written, NULL);
	g_io_channel_flush (gio, NULL);

	gsize readlen=0;
	gchar *str=NULL;
	while(G_IO_STATUS_NORMAL==
			g_io_channel_read_line(gio, &str, &readlen, NULL, NULL)){
		printf("client %lu: %s", readlen, str);
		g_free(str); str=NULL;
	}
	fprintf(stderr, "client has gone\n");
	g_io_channel_shutdown(gio, 1, NULL);
	g_io_channel_unref(gio);
	return;
}

int main(int argc, char *argv[]){
	int sockfd=setup_server_socket("4490");
	assert(sockfd>=0);

	main_loop(sockfd);

	return 0;
}

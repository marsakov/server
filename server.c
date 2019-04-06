#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
	
#define TRUE 1 
#define FALSE 0 
#define PORT 8888 

typedef struct	ip
{
	char		ip[16];
	int			packets;
	int			bytes;
}				sockets_ip;

typedef struct	client
{
	int			sd;
	int			id;
}				client_info;

sockets_ip		statistic[30];
int				current_client;
client_info		client_socket[30];
int				max_clients = 30;
char			*message = "Hello from SERVER\n";

int 	ip_exist(char *ip)
{
	for (int i = 0; i < current_client; i++)
		if (!strcmp(statistic[i].ip, ip))
			return (i + 1);
	return (0);
}

void	add_new(char *address_ip)
{
	strcpy(statistic[current_client].ip, address_ip);
	statistic[current_client].packets = 0;
	statistic[current_client].bytes = 0;
	current_client++;
}

int main(int argc , char *argv[]) 
{ 
	int					opt = TRUE;
	int					master_socket, addrlen, new_socket, activity, valread, sd;
	int					max_sd;
	struct sockaddr_in	address;
	char				buffer[1025];
	fd_set				readfds;

	current_client = 0;

	for (int i = 0; i < max_clients; i++)
	{
		client_socket[i].sd = 0;
		client_socket[i].id = 0;
	}
		

	if (!(master_socket = socket(AF_INET, SOCK_STREAM, 0))) 
	{ 
		perror("socket failed");
		exit(EXIT_FAILURE);
	} 
	

	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) 
	{ 
		perror("setsockopt");
		exit(EXIT_FAILURE);
	} 
	

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
		
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) 
	{ 
		perror("bind failed");
		exit(EXIT_FAILURE);
	} 
	printf("Listener on port %d \n", PORT);

	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen");
		exit(EXIT_FAILURE);
	} 

	addrlen = sizeof(address);
	puts("Waiting for connections ...");
		
	while (TRUE) 
	{
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		for (int i = 0 ;i < max_clients ;i++) 
		{ 
			sd = client_socket[i].sd;
			if (sd > 0) 
				FD_SET( sd , &readfds);
			if (sd > max_sd) 
				max_sd = sd;
		} 
	
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
		if (activity < 0 && errno != EINTR) 
			printf("select error");
			
		if (FD_ISSET(master_socket, &readfds)) 
		{ 
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) 
			{
				perror("accept");
				exit(EXIT_FAILURE);
			} 
			
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
			if (send(new_socket, message, strlen(message), 0) != strlen(message)) 
				perror("send");
			printf("Welcome message sent successfully\n");	
			
			if (!ip_exist(inet_ntoa(address.sin_addr)))
				add_new(inet_ntoa(address.sin_addr));

			for (int i = 0; i < max_clients; i++) 
			{
				if (!client_socket[i].sd) 
				{
					client_socket[i].sd = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
					break;
				}
			}
		}
			
		for (int i = 0;i < max_clients;i++) 
		{ 
			sd = client_socket[i].sd;
				
			if (FD_ISSET( sd , &readfds)) 
			{
				if (!(valread = read( sd , buffer, 1024))) 
				{
					getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
					close( sd );
					client_socket[i].sd = 0;
				} 
				else
				{
					buffer[valread] = '\0';
					send(sd, buffer, strlen(buffer), 0 );
					statistic[client_socket[i].id].packets++;
					statistic[client_socket[i].id].bytes += strlen(buffer) - 1;
					printf("\t\tFROM %s RECV %d packets\ttotal bytes = %d\n", statistic[client_socket[i].id].ip, statistic[client_socket[i].id].packets, statistic[client_socket[i].id].bytes);
				} 
			} 
		} 
	} 
		
	return 0;
} 

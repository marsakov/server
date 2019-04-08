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
	unsigned long	ip;
	int				id;
	int				packets;
	int				bytes;
	struct ip		*next;
	struct ip		*prev;
}					sockets_ip;

typedef struct	client
{
	int			sd;
	int			id;
}				client_info;

sockets_ip		*statistic;
int				current_client;
int				current_sd;
client_info		client_socket[30];
int				max_clients = 30;
char			*message = "Hello from SERVER\n";

int 	ip_exist(unsigned long new_ip)
{
	sockets_ip *st = statistic;

	for (int i = 0; i < current_client && st; i++) {
		if (st->ip == new_ip)
			return (st->id + 1);
		st = st->next;
	}
	return (0);
}

sockets_ip *find_elem(int id)
{
	sockets_ip *st = statistic;

	while (st)
	{
		if (st->id == id)
			return (st);
		st = st->next;
	}

	return (0);
}

void	show()
{
	sockets_ip *st = statistic;
	struct in_addr addr;
	printf("---___---___---___---___---___---___---___---___---___---___\n");
	while (st)
	{
		addr.s_addr = st->ip;
		printf("ip = %s | packets = %d | bytes = %d\n", inet_ntoa(addr), st->packets, st->bytes);
		st = st->next;
	}
	printf("---___---___---___---___---___---___---___---___---___---___\n");
}

void	add_new(unsigned long new_ip)
{
	sockets_ip *sprev;
	sockets_ip *snext;
	sockets_ip *st = statistic;
	printf("0\n");

	if (!statistic)
	{
		printf("666\n");
		statistic = malloc(sizeof(sockets_ip));
		statistic->next = 0;
		statistic->prev = 0;
		statistic->ip = new_ip;
		statistic->id = current_client;
		statistic->packets = 0;
		statistic->bytes = 0;
		current_client++;
		return ;
	}
	if (st->ip > new_ip)
	{
		printf("1\n");
		snext = st;
		printf("2\n");
		st = malloc(sizeof(sockets_ip));
		printf("3\n");
		st->next = snext;
		st->id = current_client;
		st->ip = new_ip;
		st->packets = 0;
		st->bytes = 0;
		st->prev = 0;
		st->next->prev = st;
		statistic = st;
		current_client++;
		printf("4\n");
		return ;
	}
	printf("5\n");
	while (st->next && st->next->ip < new_ip)
		st = st->next;
	snext = st->next;
	sprev = st;

	st->next = malloc(sizeof(sockets_ip));
	st = st->next;
	st->id = current_client;
	st->ip = new_ip;
	st->packets = 0;
	st->bytes = 0;
	st->next = snext;
	st->prev = sprev;

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
	int					id_ip;

	current_client = 0;
	current_sd = 0;

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

		for (int i = 0; i < current_sd; i++) 
		{ 
			sd = client_socket[i].sd;
			if (sd > 0) 
				FD_SET( sd , &readfds);
			if (sd > max_sd) 
				max_sd = sd;
		} 
	
		activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);
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
			
			if (!(id_ip = ip_exist(address.sin_addr.s_addr)))
				add_new(address.sin_addr.s_addr);

			for (int i = 0; i < max_clients; i++) 
			{
				if (!client_socket[i].sd) 
				{
					client_socket[i].sd = new_socket;
					client_socket[i].id = (id_ip) ? id_ip - 1 : current_client - 1;
					printf("Adding to list of sockets as %d\n" , i);
					break;
				}
			}
			current_sd++;
		}
			
		for (int i = 0; i < current_sd; i++) 
		{ 
			sd = client_socket[i].sd;
				
			if (FD_ISSET( sd, &readfds)) 
			{
				if (!(valread = read( sd, buffer, 1024))) 
				{
					getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
					close( sd );
					client_socket[i].sd = 0;
				} 
				else
				{
					// printf("\t[%lu]\n", statistic[current_client].ip);
					buffer[valread] = '\0';
					send(sd, buffer, strlen(buffer), 0 );
					sockets_ip *s = find_elem(client_socket[i].id);
					if (!s)
						printf("----!s-----\n");
					s->packets++;
					s->bytes += strlen(buffer) - 1;
					printf("\t\tFROM %lu RECV %d packets\ttotal bytes = %d\n", s->ip, s->packets, s->bytes);
					show();
				} 
			} 
		} 
	}
		
	return 0;
} 

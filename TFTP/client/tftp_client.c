#include "tftp.h"
#include "tftp_client.h"

int connected = 0;
int pack_num = 1;
char mode[10] = "normal";

int main()
{
	char command[256];
	tftp_client_t client;
	memset(&client, 0, sizeof(client));  /* Initializing client structure */

	/* Main loop for command-line interface */
	while (1)
	{
		printf("TFTP menu:\n");
		printf("1. connect\n2. get\n3. put\n4. mode\n5. exit\nEnter your operation: ");
		fgets(command, sizeof(command), stdin);

		/* Remove newline character */
		command[strcspn(command, "\n")] = '\0';
		/* Process the command */
		process_command(&client, command);
		printf("\n");
	}

	return 0;
}

/* Function to process commands */
void process_command(tftp_client_t *client, char *command)
{
	tftp_packet packet;
	memset(&packet, 0, sizeof(packet));

	if(strcasecmp(command, "connect") == 0)
	{
		char IP_address[20];
		printf("Enter the IP address: ");
		fgets(IP_address, sizeof(IP_address), stdin);
		IP_address[strcspn(IP_address, "\n")] = '\0';

		struct sockaddr_in sa;
		/* Checking if the user given address is valid or not */
		if(inet_pton(AF_INET, IP_address, &(sa.sin_addr)) == 0)
		{
			printf("Error: Invalid IP address format!!\nExample: 127.0.0.1\n");
			return;
		}
		else
		{
			connect_to_server(client, IP_address, PORT);
			printf("INFO: Connection to server successful!!\n");
		}
		connected = 1;
	}
	else if(strcasecmp(command, "put") == 0)
	{
		if(connected == 1)
		{
			char filename[30];
			printf("Enter the file name: ");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = '\0';

			int fd = open(filename, O_CREAT | O_EXCL, 0644);
			if(fd != -1)
			{
				unlink(filename);
				printf("Error: Given file is not present!!!\n");
				return;
			}
			else
				put_file(client, filename);
		}
		else
		{
			printf("\nConnection not established with server!!!\nFirst connect with the server\n");
			return;
		}

	}
	else if(strcasecmp(command, "get") == 0)
	{
		if(connected == 1)
		{
			char filename[20];
			printf("Enter the filename youb want to recieve: ");
			fgets(filename, sizeof(filename), stdin);
			filename[strcspn(filename, "\n")] = '\0';

			get_file(client, filename);	
		}
		else
		{
			printf("\nConnection not established with server!!!\nFirst connect with the server\n");
			return;
		}
	}
	else if(strcasecmp(command,"mode") == 0)
	{

		printf("MENU:\n1. Normal\n2. Octet\n3. Netascii\n");
		printf("Enter the Option:");
		fgets(mode, sizeof(mode), stdin);
		mode[strcspn(mode, "\n")] = '\0';

		printf("CURRENT MODE = %s\n",mode);
	}
	else if(strcasecmp(command, "exit") == 0)
	{
		/* Calling the disconnect function */
		printf("Diconnecting from the server!!!\n");
		disconnect(client);
		printf("Exiting....\n");
		exit(0);
	}
	else 
	{
		printf("\nError: Invalid option!!\n");
		return;
	}
}


/* Function to connect the client with the server */
void connect_to_server(tftp_client_t *client, char *ip, int port) 
{
	/* Creating UDP socket */
	client -> sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(client -> sockfd == -1)
	{
		perror("socket");
		return;
	}	

	/* Setting up server address */
	client -> server_addr.sin_family = AF_INET;
	client -> server_addr.sin_port = htons(port);
	client -> server_addr.sin_addr.s_addr = inet_addr(ip);

	client -> server_len = sizeof(client -> server_addr);
}

/* Function to put the file */
void put_file(tftp_client_t *client, char *filename) 
{
	/* Calling the send_request function to send the write request */
	send_request(client -> sockfd, client -> server_addr, filename, WRQ);
}

/* Function to get the file */
void get_file(tftp_client_t *client, char *filename)
{
	/* Calling the Send the request function to send the read request */ 
	send_request(client -> sockfd, client -> server_addr, filename, RRQ);
}

/* Function to disconnect the clien from the server */
void disconnect(tftp_client_t *client)
{
	// close fd
	close(client -> sockfd);
}

/* Function to send the request */
void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
	/* updating the packet number everytime while sending new data*/
	pack_num = 1;

	tftp_packet pck;
	/* clearing the garbage values */
	memset(&pck, 0, sizeof(pck));

	/* initializing the variables */
	pck.opcode = opcode;
	strcpy(pck.body.request.filename, filename);

	/* Copying the mode */
	strcpy(pck.body.request.mode, mode);
	/* sending request to the server */
	sendto(sockfd, &pck, sizeof(pck), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	/* calling the receive_request function  */
	receive_request(sockfd, server_addr, filename, opcode);
}

/* Function to receive the request from the server */
void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode)
{
	tftp_packet pck;
	/* clearing the garbage values */
	memset(&pck, 0, sizeof(pck));

	socklen_t len = sizeof(server_addr);
	/* receiving the acknowledgement */
	recvfrom(sockfd, &pck, sizeof(pck), 0, (struct sockaddr *)&server_addr, &len);

	/* Checking opcode for the condition */
	if(pck.opcode == ACK)
	{
		/* Write request received so sending the file */
		if(pck.body.ack_packet.block_number == WRITE)
		{
			printf("\n****** READY TO SEND ******\n\n");
			send_file(sockfd, server_addr, len, filename);
		}
		/* Read request received so receiveing the file */
		else if(pck.body.ack_packet.block_number == READ)
		{
			printf("\n****** READY TO RECEIVE ******\n\n");
			receive_file(sockfd, server_addr, len, filename);
		}
	}
	/* This is for ERROR */
	else if(pck.opcode == ERROR)
	{
		printf("\nError: \n");
		printf("%s\n\n", pck.body.error_packet.error_msg);
	}
}

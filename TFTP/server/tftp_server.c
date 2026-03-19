#include "tftp.h"

char mode[10] = "normal";
int pack_num = 1;

/* Function to handle the client */
void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet);

int main()
{
	int sockfd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len = sizeof(client_addr);
	tftp_packet packet;

	/* Creating UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sockfd == -1)
	{
		perror("socket");
		return -1;
	}	

	/* Set up server address */

	server_addr.sin_family = AF_INET;                       //to create socket compatible with the IPv4
	server_addr.sin_port = htons(PORT);                     
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   //converting the string to binary(Binary of IP address)

	/* Binding the socket */
	bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

	printf("TFTP Server listening on port %d...\n", PORT);

	/* Main loop to handle incoming requests */
	while (1) 
	{
		int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &client_len);
		if (n < 0) 
		{
			perror("Receive failed or timeout occurred");
			continue;
		}

		//printf("File name received -> %s\n", packet.body.request.filename);
		handle_client(sockfd, client_addr, client_len, &packet);
	}

	close(sockfd);
	return 0;
}

/* Function to handle the client */
void handle_client(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, tftp_packet *packet) 
{
	/* updating the values of mode and pack_number */
	strcpy(mode, packet->body.request.mode);
	pack_num = 1;

	if(packet -> opcode == WRQ)
	{
		/* checking the file is present or not */ 
		int fd = open(packet -> body.request.filename, O_CREAT | O_WRONLY | O_EXCL, 0644);
		if(fd == -1)
		{
			/* if file present clearing the data */
			fd = open(packet -> body.request.filename, O_WRONLY | O_TRUNC);
		}

		/* closing the file */
		close(fd);

		tftp_packet ack;
		memset(&ack, 0, sizeof(ack));
		ack.opcode = ACK;
		ack.body.ack_packet.block_number = WRITE;

		sendto(sockfd, &ack, 4, 0, (struct sockaddr *)&client_addr, client_len);

		printf("\n***** READY TO RECEIVE *****\n\n");

		/* calling the receive file function */
		receive_file(sockfd, client_addr, client_len, packet -> body.request.filename);

		return;
	}
	else if(packet -> opcode == RRQ)
	{
		/* checking if file is present or not */
		int fd = open(packet -> body.request.filename, O_CREAT | O_EXCL, 0644);
		/* opening failed, so file is present */
		if(fd == -1)
		{
			close(fd);
			tftp_packet ack;
			memset(&ack, 0, sizeof(ack));
			ack.opcode = ACK;
			ack.body.ack_packet.block_number = READ;

			/* sending the acknowledgement to the client */
			sendto(sockfd, &ack, 4, 0, (struct sockaddr *)&client_addr, client_len);

			printf("\n***** READY TO SEND *****\n\n");
			
			/* calling Send_file function */
			send_file(sockfd, client_addr, client_len, packet -> body.request.filename);
			return;
		}
		/* If the file created the file is not present */ 
		else
		{
			close(fd);
			unlink(packet -> body.request.filename);
			memset(packet, 0, sizeof(*packet));
			packet->opcode = ERROR;
			
			/* copying the error message */
			strcpy(packet -> body.error_packet.error_msg, "FILE DOES NOT EXIST");
			
			/* Sending the error message to client */
			sendto(sockfd, packet, sizeof(*packet), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
		}
	}
}

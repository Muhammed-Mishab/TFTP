#include "tftp.h"

extern char mode[10];
extern int pack_num;

/* Function to send the file from client to server */
void send_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return;
	}

	tftp_packet pck, ack;
	int bytes;
	pack_num = 1;

	do
	{
		memset(&pck, 0, sizeof(pck));

		/* For the normal mode */
		if (strcasecmp(mode, "normal") == 0)
		{
			bytes = read(fd, pck.body.data_packet.data, 512);
		}

		/* For the octet mode */
		else if (strcasecmp(mode, "octet") == 0)
		{
			bytes = read(fd, pck.body.data_packet.data, 1);
		}

		/* For the netascii mode */
		else if (strcasecmp(mode, "netascii") == 0)
		{
			char buff[512];
			int i = 0;
			char ch;

			while (i < 512 && read(fd, &ch, 1) == 1)
			{
				if (ch == '\n')
					buff[i++] = '\r';

				if (i == 512)
				{
					lseek(fd, -1, SEEK_CUR);
					break;
				}

				buff[i++] = ch;
			}

			memcpy(pck.body.data_packet.data, buff, i);
			bytes = i;
		}

		pck.opcode = DATA;
		pck.body.data_packet.block_number = pack_num;

		/* sending data packet */
		sendto(sockfd, &pck, 4 + bytes, 0, (struct sockaddr *)&client_addr, client_len);

		printf("Sent block %d, bytes = %d\n", pack_num, bytes);

		/* waiting for correct ACK */
		do
		{
			/* Receiving the acknowledgemnt from the server */
			memset(&ack, 0, sizeof(ack));
			recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, &client_len);
		} while (ack.opcode != ACK || ack.body.ack_packet.block_number != pack_num);

		pack_num++;

	} while ((strcasecmp(mode, "octet") == 0 && bytes == 1) || (strcasecmp(mode, "normal") == 0 && bytes == 512) || (strcasecmp(mode, "netascii") == 0 && bytes == 512));

	close(fd);
	printf("\n***** FILE SENT SUCCESSFULLY *****\n");
}


/* Function to receive the file from the server */
void receive_file(int sockfd, struct sockaddr_in client_addr, socklen_t client_len, char *filename)
{
	int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd < 0)
	{
		perror("open");
		return;
	}

	tftp_packet pck;
	int bytes;
	pack_num = 1;

	do
	{
		memset(&pck, 0, sizeof(pck));

		int n = recvfrom(sockfd, &pck, sizeof(pck), 0, (struct sockaddr *)&client_addr, &client_len);

		if (pck.opcode != DATA || pck.body.data_packet.block_number != pack_num)
			continue;

		bytes = n - 4;  

		/* For the normal and netascii mode */
		if (strcasecmp(mode, "normal") == 0 || strcasecmp(mode, "netascii") == 0)
			write(fd, pck.body.data_packet.data, bytes);

		/* For the octet mode */
		else if (strcasecmp(mode, "octet") == 0)
			write(fd, pck.body.data_packet.data, bytes);

		printf("Received block %d, bytes = %d\n", pack_num, bytes);

		memset(&pck, 0, sizeof(pck));
		pck.opcode = ACK;
		pck.body.ack_packet.block_number = pack_num;

		/* Sending the acknowledgement to the server */
		sendto(sockfd, &pck, 4, 0, (struct sockaddr *)&client_addr, client_len);

		pack_num++;

	} while ((strcasecmp(mode, "octet") == 0 && bytes == 1) || ((strcasecmp(mode, "normal") == 0 || strcasecmp(mode, "netascii") == 0) && bytes == 512));

	close(fd);
	printf("\n***** FILE RECEIVED SUCCESSFULLY *****\n");
}

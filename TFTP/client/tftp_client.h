#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H


typedef struct 
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len;
    char server_ip[INET_ADDRSTRLEN];
} tftp_client_t;


/* Function to connect with the server */
void connect_to_server(tftp_client_t *client, char *ip, int port);

/* Function to put the file */
void put_file(tftp_client_t *client, char *filename);

/* Function to get the file */
void get_file(tftp_client_t *client, char *filename);

/* Function to disconnect the client from the server */
void disconnect(tftp_client_t *client);

/* Function to process the command given by the user */
void process_command(tftp_client_t *client, char *command);

/* Function to send the request to the server */
void send_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);

/* Function to receive the request from the server */
void receive_request(int sockfd, struct sockaddr_in server_addr, char *filename, int opcode);

#endif

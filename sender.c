#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define port 8080

int main(int argc, char **argv)
{
    // Ensures the required number of arguments in command line
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <Packet Size> <IP Address> <time-space> <number of packet pairs>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extracts different fields based on usage
    int packetSize = atoi(argv[1]);
    const char *ip_address = argv[2];
    int timeDelay = atoi(argv[3]);
    int numPairs = atoi(argv[4]);

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address or Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // sends packet size to server to facilitate packet identification in future
    char message[5 + packetSize];
    sprintf(message, "%d", packetSize);
    
    ssize_t sent_bytes = sendto(sockfd, message, strlen(message), 0,
                                (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (sent_bytes < 0)
    {
        perror("Failed to send message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    // Start sending the packets to measure time and ultimately C
    for (int i = 0; i < numPairs; i++)
    {
        sprintf(message,"%d",2*i+1); // add the identifier (pkt number) at the beginning
        char tm[4];
        sprintf(tm,"%d",2*i+1);
        message[strlen(tm)] = ' '; // a space after the identifier
        memset(message+1+strlen(tm),'a',packetSize); // fills the packet with 'a's

        // Send first message to the server
        sent_bytes = sendto(sockfd, message, strlen(message), 0,
                            (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Error handling
        if (sent_bytes < 0)
        {
            perror("Failed to send message");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Message sent to %s:%d\n", ip_address, port);

        sprintf(message,"%d",2*i+2); // second packet
        sprintf(tm,"%d",2*i+2);
        message[strlen(tm)] = ' ';
        memset(message+1+strlen(tm),'a',packetSize); // fills the packet

        // Send second message to the server
        sent_bytes = sendto(sockfd, message, strlen(message), 0,
                            (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Error handling
        if (sent_bytes < 0)
        {
            perror("Failed to send message");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Message sent to %s:%d\n", ip_address, port);

        // sleep statement to avoid packet drop and clogging
        usleep(1000*timeDelay);
    }

    // Terminating message
    sprintf(message, "lol");
    sent_bytes = sendto(sockfd, message, strlen(message), 0,
                        (struct sockaddr *)&server_addr, sizeof(server_addr));
    
    // Close the socket
    close(sockfd);

    return 0;
}
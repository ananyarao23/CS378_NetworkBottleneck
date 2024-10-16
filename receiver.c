#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define port 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    // Ensures the required number of arguments in command line
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <output file name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extract file name
    const char* filename = argv[1];

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the given port
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", port);


    // Receives the packet size in message, used later to extract packet identifiers
    char pkt_size[BUFFER_SIZE];
    socklen_t client_len = sizeof(client_addr);
    ssize_t recv_len = recvfrom(sockfd, pkt_size, 1024, 0, (struct sockaddr *)&client_addr, &client_len);
    int size = atoi(pkt_size);


    // declarations for measuring time
    struct timespec start, end;
    double total_time;

    // Variables to keep track of consecutive packet identifiers
    int prev_pkt = -1,curr_pkt; /* There's always two of them */ 

    // open the file (make one if not present already)
    FILE *file = fopen(filename, "w");

    // Error handling
    if (file == NULL) {
        // Check if the file was opened successfully
        perror("Error opening file");
        return 1;
    }
    
    char buffer[size+5];

    // Start receiving the stream of packets
    while(1){

        fflush(stdout);

        ssize_t recv_len = recvfrom(sockfd, buffer, 5+size, 0, (struct sockaddr *)&client_addr, &client_len);
        // recv_len - unit -> bytes 

        // this clock gives time closest to actual time without context switch
        // CREDITS: ChatGPT
        clock_gettime(CLOCK_MONOTONIC_RAW, &end); // ignore the red line (if you see any)

        buffer[recv_len] = '\0';
        printf("Message received:%s\n",buffer);

        // Terminating condition (message sent is "lol...")
        if (buffer[0] == 'l'){
            printf("Process completed");
            break;
        }

        // Extract the identifier
        char str[5];
        for(int i=0;i<recv_len-size-1;i++) str[i] = buffer[i];
        curr_pkt = atoi(str);

        // If it is preceded by the first packets of the same pair number
        // we calculate the time difference

        if ((prev_pkt!= -1) && (curr_pkt == prev_pkt + 1) && (prev_pkt%2 == 1)) {
            total_time = end.tv_nsec - start.tv_nsec + (end.tv_sec - start.tv_sec)*1e9; // unit -> nanosecond
            printf("Time taken between the two packets in microseconds(%d and %d):%f us\n\n",prev_pkt,curr_pkt,total_time);
            double C = recv_len*1e3*8/total_time; // unit -> Mega bits per sec
            fprintf(file,"%f\n",C);
        }
        start = end; // start is the time struct of previous packet, always
        prev_pkt = curr_pkt; 
    }

    // close file
    fclose(file);

    // close socket
    close(sockfd);

    // good night Zzz..

    return 0;
}
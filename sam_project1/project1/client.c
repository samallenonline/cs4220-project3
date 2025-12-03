/*
** client.c -- a stream socket client 
*/

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

// define macros
# define SERVER_PORT 2876 // the port client will be connecting to
# define BUFFER_SIZE 1024 // max number of bytes we can get at once 

int main(int argc, char *argv[])
{
    // define variables to be used 
    int c, s, bytes;
    char file_buffer[BUFFER_SIZE];  // buffer for incoming file
    struct hostent *h;  // info about server
    struct sockaddr_in channel;     // holds IP address 

    // ensure the user provides the correct number of arguments
    if (argc != 3)
    {
        printf("\n> usage: client server-name file-name");
        exit(-1);
    }

    // look up host address 
    h = gethostbyname(argv[1]);
    if (!h)
    {
        printf("\n> gethostbyname failed to locate %s", argv[1]); // could not find host
        exit(-1);
    }

    // CREATE SOCKET
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
    {
        printf("\n> socket call failed");
        exit(-1);
    }
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port = htons(SERVER_PORT);

    // CONNECT TO SERVER
    c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
    if (c < 0)
    {
        printf("\n> connection failed");
        exit(-1);
    }

    // connection is now established
    // send filename to server including 0 byte at the end 
    write(s, argv[2], strlen(argv[2]) + 1);

    // open file for writing in client_files directory
    FILE *file = fopen("client_files/received_file", "wb");     // create/open file to be sent back
    if (file == NULL) 
    {
        printf("\n> failed to open file ");
        exit(-1);
    }

    // for debugging purposes - make sure we can write to the file
    // const char *test_str = "test";
    // fwrite(test_str, 1, 4, file);
    // fflush(file);

    char expected_seq = 1;

    while(1)
    {
        // obtain the frame
        char frame_buffer[1 + BUFFER_SIZE];
        bytes = read(s, frame_buffer, sizeof(frame_buffer));
        if (bytes < 0) break;

        // extract sequence number 
        char sequence_num = frame_buffer[0];

        // extract data
        int data_bytes = bytes - 1;
    
        // frame has been received
        printf("\n> received frame %d with %d bytes", sequence_num, data_bytes);

        if (sequence_num == expected_seq)
        {
            // new frame- write data and send ACK to server 
            printf("\n> writing %d bytes to the file...", data_bytes);
            if (data_bytes > 0)
            {
                memcpy(file_buffer, frame_buffer + 1, data_bytes);
                size_t written_data = fwrite(file_buffer, 1, data_bytes, file);
                printf("\n> wrote %zu bytes", written_data);
                fflush(file);
            }

            write(s, &sequence_num, sizeof(sequence_num)); // send ACK
            printf("\n> sent ACK %d", sequence_num); // confirmation

            // toggle sequence number 
            expected_seq = (expected_seq == 0) ? 1 : 0;

            // check for EOF (indicates empty frame)
            if (data_bytes == 0)
            {
                // EOF detected
                printf("\n> file transfer has completed ");
                break;
            }
        }
        else
        {
            // duplicate frame— resend ACK for previous frame
            char prev_ack = (expected_seq == 0) ? 1 : 0;
            write(s, &prev_ack, sizeof(prev_ack));
            printf("\n> resending ACK %d", prev_ack);
        }
    }
    fclose(file);
    printf("\n> file has been closed");
    close(s);
    printf("\n> socket has been closed");
    return 0;
}
/*
** server.c -- a stream socket server 
*/

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

// define macros
#define SERVER_PORT 2876 // the port client will be connecting to
#define QUEUE_SIZE 10 // how many pending connections queue will hold
#define FRAME_SIZE 1024
#define BUFFER_SIZE FRAME_SIZE // block transfer size

// define struct to be used for ARQ frames 
struct Frame 
{
    unsigned char seq_num;
    char data[FRAME_SIZE];
};

int main(int argc, char *argv[])
{
    int s, b, l, sa, bytes, on = 1;
    char filename_buffer[BUFFER_SIZE]; // buffer for filename
    char file_buffer[BUFFER_SIZE];  // buffer for outgoing file
    struct sockaddr_in channel; // holds IP address

    // build address structure to bind to socket 
    memset(&channel, 0, sizeof(channel));   // zero channel
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    // passive open - wait for connection
    // CREATE SOCKET
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) 
    {
        printf("\n> socket call failed 0");     // error message, socket call failed
        exit(-1);
    }
    else
    {
        printf("\n> socket call successful");
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

    // BIND TO SOCKET
    b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
    if (b < 0)
    {
        printf("\n> bind failed 0");    // error message, bind failed
        exit(-1);
    }   
    else
    {
        printf("\n> bind to socket successful");
    }

    // LISTEN FOR INCOMING CONNECTIONS 
    l = listen(s, QUEUE_SIZE);      // specify queue size
    if (l < 0)
    {
        printf("\n> listen failed 0");
        exit(-1);
    }
    else
    {
        printf("\n> listen successful");
    }

    // socket is now set up and bound - wait for connection and process it 
    while (1)
    {
        sa = accept(s, 0, 0);      // block for connection request
        if (sa < 0)
        {
            printf("\n> accept failed 0");
            exit(-1);
        }

        int filename_bytes = read(sa, filename_buffer, BUFFER_SIZE);     // read file name from socket
        // error handling for retrieving filename 
        if (filename_bytes <= 0)
        {
            printf("> error: failed to read filename");
            close(sa);
            continue; // go back to accept() to accept a new client
        }
        filename_buffer[filename_bytes] = '\0';

        // get and return file 
        FILE *file = fopen(filename_buffer, "rb");     // open file to be sent back
        if (file == NULL) 
        {
            printf("\n> open failed");
            close(sa);
            continue; // go back to accept() to accept a new client 
        }

        char sequence_num = 1;
        while (1)
        {
            bytes = fread(file_buffer, 1, BUFFER_SIZE, file);     // read from file
            printf("\n> returned %d bytes", bytes);

            // check for EOF
            if (bytes == 0)
            {
                printf("\n> EOF reached");
                // send EOF frame to client
                char eof_frame[1] = {sequence_num};
                write(sa, eof_frame, sizeof(eof_frame));
                // indicate that the transfer is complete
                printf("\n> transfer complete- closing connection");
                break;
            }

            // check for errors
            if (bytes < 0)
            {
                printf("\n> error: error reading file");
                break;
            }  
            
            printf("\n> sending frame %d with %d bytes", sequence_num, bytes);    

            struct Frame frame;         // create a frame object using the defined struct
            frame.seq_num = sequence_num;      // initialize the sequence number
            memcpy(frame.data, file_buffer, bytes);     // copy over the buffer into the data field of the frame

            int frame_acked = 0;
            while (!frame_acked)
            {
                // create buffer to hold frame
                char transmission_buffer[sizeof(frame.seq_num) + bytes];
                memcpy(transmission_buffer, &frame.seq_num, sizeof(frame.seq_num));
                memcpy(transmission_buffer + sizeof(frame.seq_num), frame.data, bytes);
                
                // send frame
                int bytes_sent = write(sa, transmission_buffer, sizeof(transmission_buffer));
                if (bytes_sent != sizeof(transmission_buffer))
                {
                    printf("\n> error: failed to send entire frame");
                }

                // use select() to start a timer
                fd_set readfds;
                struct timeval timeout;

                timeout.tv_sec = 5;     // set up timeout
                timeout.tv_usec = 0;

                FD_ZERO(&readfds);
                FD_SET(sa, &readfds);       // add socket to watch list
                int ready = select(sa + 1, &readfds, NULL, NULL, &timeout);     // wait for data with timeout

                // determine expected client ACK
                char expected_ack = sequence_num;
                char received_ack;
    
                // check output of select()
                if (ready > 0)      // data is ready to read
                {
                    int bytes_ack = read(sa, &received_ack, sizeof(received_ack)); // read from socket to obtain client ACK
                    // error handling for reading ACK
                    if (bytes_ack != sizeof(received_ack)) 
                    {
                        printf("\n> error: failed to read ACK from client");
                    }
                    // if correct ACK received within timeout, send next frame
                    if (received_ack == expected_ack)
                    {
                        sequence_num = (sequence_num == 0) ? 1 : 0;     // toggle sequence number
                        frame_acked = 1;    // set to 1, frame has been ackowledged by client!
                        printf("\n> frame ackowledged by client");
                    }
                    // if no ACK/incorrect ACK is received, resend same frame
                    else
                    {
                        printf("\n> client ACK is incorrect/not recieved— resending frame");
                        // no need to resend manually because of loop
                    }
                }
                // timeout occured, resend same frame
                else if (ready == 0)
                {
                    printf("\n> timeout occured— resending frame");
                    // resend frame
                    int bytes_sent = write(sa, transmission_buffer, sizeof(transmission_buffer));
                    if (bytes_sent != sizeof(transmission_buffer))
                    {
                        printf("\n> error: failed to send entire frame");
                    }
                    
                }
                // error occured
                else
                {
                    printf("\n> an error has occured");
                    exit(-1);
                }
            }
        }
        fclose(file);      // close file
        printf("\n> file has been closed");
        close(sa);      // close connection
        printf("\n> connection has been closed");
        close(s);   // close socket
        printf("\n> socket has been closed");
        break;
    }
    return 0;
}
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define FRAME_SIZE 32 // in bytes
#define PORT_NUMBER "2260"
#define ACK_SIZE 1

int main(void) {
	
	struct addrinfo config;
	memset(&config, 0, sizeof(config));
	config.ai_family = AF_UNSPEC; // IPv4 and IPv6 are both acceptable
	config.ai_socktype = SOCK_STREAM; // Use TCP
	config.ai_flags = AI_PASSIVE; // Accept connections from any IP address

	struct addrinfo* connectionConfig;
	memset(&connectionConfig, 0, sizeof(connectionConfig));

	// Create a config for connections using the specified ip, port, and config settings
	int status = getaddrinfo("127.0.0.1", PORT_NUMBER, &config, &connectionConfig);
	if (status == 0) {

		// Create socket with the specified behavior
		int socketDescriptor = socket(connectionConfig->ai_family,
										connectionConfig->ai_socktype,
										connectionConfig->ai_protocol);

		status = connect(socketDescriptor, connectionConfig->ai_addr, connectionConfig->ai_addrlen);
		printf("Connecting to server...\n");
		if (status == 0) {

			FILE* file = fopen("client_files/recieved.txt", "wb");

			// Make sure everything's initalized to prevent possible garbage values from appearing
			char frame[FRAME_SIZE] = {'\0'};
			char expectedSequence = '0';
			char sequenceNumber = '\0';

			// While server hasn't sent a request to end
			while(strncmp(frame, "D", ACK_SIZE) != 0) {

				// Empty frame after each loop to prepare for the next recieved call
				memset(frame, 0, sizeof(frame));

				// Get next frame and its sequence number; double-check it's not a request to end
				int bytesRecieved = recv(socketDescriptor, frame, FRAME_SIZE + 1, 0);
				if (strncmp(frame, "D", ACK_SIZE) != 0) {

					// If nothing is recieved, the server isn't functioning properly or closed the connection
					if (bytesRecieved == 0) {
						printf("Lost connection to server, closing connection...\n");
						fclose(file);
						close(socketDescriptor);
						return 0; // Close client, since it only works if a server exists
					}

					sequenceNumber = frame[FRAME_SIZE];
					printf("sequence number from frame: %c\n", sequenceNumber);
					printf("expected sequence number: %c\n", expectedSequence);

					// Check that frame's sequence matches the client's next expected sequence
					if (sequenceNumber == expectedSequence) {

						// Write to file, send acknowledgement
						char* message = frame;
						message[FRAME_SIZE] = '\0'; // Remove sequence number before storage
						fprintf(file, message);

						// Convert sequenceNumber into a string for sending
						char stringSequence[ACK_SIZE + 1] = {'\0'};
						stringSequence[0] = sequenceNumber;

						send(socketDescriptor, stringSequence, ACK_SIZE, 0);
						printf("Sent ACK: %s\n", stringSequence);
				
						// Toggle the expected sequence number
						if (expectedSequence == '0') {
							expectedSequence = '1';
						}
						else {
							expectedSequence = '0';
						}
				
					}
					else {

						// Convert expectedSequence into string for sending
						char stringExpected[ACK_SIZE + 1] = {'\0'};
						stringExpected[0] = expectedSequence;

						// Request resend, because recieved frame was a duplicate or not fully recieved
						send(socketDescriptor, stringExpected, ACK_SIZE, 0);
						printf("Mismatched sequence number, requesting retransmission from server...\n");
						printf("Sent ACK: %s\n", stringExpected);
					}
				} // end if not request to end

			} // end while not request to end

			printf("Server request to end recieved, sending acknowledgement...\n");
											
			// Send final acknowledgement so server knows client recieved the request to end
			send(socketDescriptor, "D", ACK_SIZE, 0);
			printf("Sent ACK: %s\n", "D");

			fclose(file);
			close(socketDescriptor);

		} // end if succesful connection
		else {
			printf("Error: Something went wrong when connecting server; status: %d\n", status);
		}
	} // end if valid addrinfo
	else {
		printf("Error: Something went wrong when obtaining connection config; status: %d\n", status);
	}

	return 0;

}

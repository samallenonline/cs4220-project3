#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#define FRAME_SIZE 32 // in bytes
#define PORT_NUMBER "2260"
#define MAX_WAITING_CONNECTIONS 5
#define WAIT_TIME 5
#define ACK_SIZE 1

int main(void) {

	struct addrinfo config;
	memset(&config, 0, sizeof(config));
	config.ai_family = AF_UNSPEC; // IPv4 and IPv6 are both acceptable
	config.ai_socktype = SOCK_STREAM; // Use TCP
	config.ai_flags = AI_PASSIVE; // Accept connections from any IP address

	struct addrinfo* connectionConfig;
	memset(&connectionConfig, 0, sizeof(connectionConfig));

	// Stores address of an incoming connection
	struct sockaddr_storage clientAddress;
	socklen_t clientAddressSize;

	// Create a config for connections using the specified ip, port, and config settings
	int status = getaddrinfo("127.0.0.1", PORT_NUMBER, &config, &connectionConfig);
	if (status == 0) {

		// Create socket with the specified behavior
		int socketDescriptor = socket(connectionConfig->ai_family,
									 	connectionConfig->ai_socktype,
										connectionConfig->ai_protocol);

		// Associate socket with the address/port pair in ai_addr
		status = bind(socketDescriptor, connectionConfig->ai_addr, connectionConfig->ai_addrlen);
		if (status == 0) {

			// Listen for incoming connections over the specified socket; allow only certain number in queue
			status = listen(socketDescriptor, MAX_WAITING_CONNECTIONS);
			printf("Server listening...\n");
			if (status == 0) {

				clientAddressSize = sizeof(clientAddress);
				errno = 0;
				int acceptedConnection = accept(socketDescriptor, (struct sockaddr*)&clientAddress, &clientAddressSize);
				if (errno <= 0) {

					FILE* file = fopen("input_file.txt", "rb");
					if (file != NULL) {

						char buffer[FRAME_SIZE + 1] = {'\0'}; // Store a frame from the file
						unsigned int frameNumber = 0; // Which frame is being processed
						char sequence = '0'; // Flag used to help maintain frame order
						char ackRecieved[ACK_SIZE] = {'\0'}; // Acknowledgement recieved from client
						int readBytes = fread(buffer, 1, FRAME_SIZE, file); // Used when checking to see how much of file
																																//	is left without moving through the file
						
						// While there is more in the file, read into buffer frame-by-frame
						while (readBytes > 0) {

							// Double-check file was fully read to prevent off-by-one error
							if (readBytes > 0) {

								if (frameNumber % 2 == 0) { // is even
									sequence = '0';
								}
								else {
									sequence = '1';
								}
								printf("Current sequence number: %c\n", sequence);
						
								buffer[FRAME_SIZE] = sequence;

								// Wait until an ack is recieved for the current frame, resend if endTime is exceeded
								bool correctAck = false;
								while (correctAck == false) {
							
									// Send current frame to client
									int bytesSent = send(acceptedConnection, buffer, FRAME_SIZE + 1, 0);
									if (bytesSent >= FRAME_SIZE + 1) {

										// Make sure ackRecieved is always initalized and empty before sending next frame
										for (unsigned int i = 0; i < ACK_SIZE; i++) {
											ackRecieved[i] = '\0';
										}

										clock_t endTime = clock() + WAIT_TIME;
										clock_t currentTime = clock();

										// Loop until the client sends a valid ACK or until the timeout ends
										while ((ackRecieved[0] != sequence) && (currentTime <= endTime)) {

											// Get acknowledgement from client
											int bytesRecieved = recv(acceptedConnection, ackRecieved, ACK_SIZE, 0);

											// If nothing is returned, either the client is malfunctioning or it closed the connection
											if (bytesRecieved == 0) {
												printf("Lost connection to client, closing server connection...\n");
												fclose(file);
												close(acceptedConnection);
												close(socketDescriptor);
												return 0; // We're only dealing with single client-server, so just close server too
											} // end if

											printf("Client ACK recieved: %s\n", ackRecieved);
						
											currentTime = clock();
									
											// If the correct ACK recieved, continue to next frame, else resend current frame
											if (ackRecieved[0] == sequence) {
												frameNumber++;
												correctAck = true;
											} // end if
											else {
												printf("Bad ACK recieved, resending frame...\n");
											}

										} // end while

									} // end if
									else {
										printf("Error: Something went wrong while sending data; expected %d, sent %d\n", FRAME_SIZE + 1, bytesSent);
									}
							
								} // end while

								// Empty buffer and ackRecieved for next frame call
								memset(buffer, 0, sizeof(buffer));
								memset(ackRecieved, 0, sizeof(ackRecieved));

								// Read the next frame from the file
								readBytes = fread(buffer, 1, FRAME_SIZE, file);

							} // end if done reading file

						} // end while reading file

						printf("File fully sent, conducting final closing request\n");

						// Request connection close from client; wait until client sends back a final ack
						int bytesSent = send(acceptedConnection, "D", ACK_SIZE, 0);
						while (bytesSent < ACK_SIZE) {
							bytesSent = send(acceptedConnection, "D", ACK_SIZE, 0);
							printf("Frame sent is: %s\n", "D");
						}
						while (strncmp(ackRecieved, "D", ACK_SIZE) != 0) {
							recv(acceptedConnection, ackRecieved, ACK_SIZE, 0);
							printf("Client ACK recieved: %s\n", ackRecieved);
						}

					} // end if file exists
					else {
						printf("Error: Something went wrong while opening file; check it exists\n");
					}

					fclose(file);
					close(acceptedConnection);

				} // end if valid accept
				else {
					printf("Error: Something went wrong when accepting a connection; errno: %d\n", errno);
				}
			} // end if valid listen
			else {
				printf("Error: Something went wrong while listening for connections; status: %d\n", status);
			}
		} // end if valid bind
		else {
			printf("Error: Something went wrong while binding to socket; status: %d\n", status);
		}

		close(socketDescriptor);

	} // end if valid addrinfo
	else {
		printf("Error: Something went wrong while retrieving connection config; status: %d\n", status);
	}

	return 0;

} // end main()

Stefan Mital
I have neither given nor received unauthorized assistance on this work

There are two methods to build and run the program:
1. To build and run program on one machine:
	make all							|Build program
	./serverside &				|Run serverside binary as a background process
	./clientside					|Run clientside binary

2. To build and run program on two machines (RECOMMENDED):
	First boot up two instances on localhost. For redcloud, just ssh in from two different terminals.
	On one machine:
		make all						|Build program
		./serverside				|Run serverside binary
	On the other machine:
		./clientside				|Run clientside binary

To close the program, either:
	- run it to completion, or 
	- close either the server or the client, and the other will detect this and close automatically.

Description of the program and its parts:
This program creates a basic client-server which sends a single file from the server to the client via fixed byte frames, which then gets put into the client_files/ subdirectory. The architecture implements the Stop-and-Wait ARQ protocol to control flow.
server.c
	Implements the server: listens for a connection, then sends over the file input_file.txt.
client.c
	Implements the client: Attempts a connection, then accepts a file, sending an ACK for each frame of data.
	Creates the file recieved.txt in the client_files subdirectory, which matches the accepted file.
serverside
	The compiled binary of server.c. Use this to start the server BEFORE running clientside.
clientside
	The compiled binary of client.c. Use this to start the client AFTER running serverside.
input_file.txt
	The test file sent to the client from the server.
client_files/recieved.txt
	The file recieved by the client. Should match input_file.txt.
Makefile
	Contains the functionality of the "make" commands. Implements make all, make server, make client, and make clean.

Challenges:
A big part of this assignment's difficulty are what I like to call, "this only happens in C". A lot of the issues I ran into had to do with memory management and pointers, such as the very specific ways the calls required pointers to structs to do anything or the fact that \0 has to be manually entered at the end of strings. Another example is I had a lot of issues with the closing ack being recieved but not interpreted correctly. It turns out the issue is I wasn't using the "safe" version of strcmp, which is strncmp. It was reading something it wasn't supposed to and causing a mismatch, so this was fixed by using strncmp instead. This caused an infinite loop, but I fixed it by double-checking all my strings were initialized with \0. I also had some difficulty getting the closing acknowledgement to register on both sides. I got this to work by rearranging the scope of the ackRecieved variable on the server's end, and on the client's end by reseting frame at the start of every loop instead of the end, so the server's end request wouldn't get wiped.

One very big problem I ran into that plagued me right up to the deadline is that the client always got garbage at the end of its recieved frames which didn't exist on the server's side. I properly implemented '\0' at the end of strings on both client and server, verified I am sending the correct number of bytes, made sure I was clearing the buffer before sending anything over to the client, and it was still broken. I finally fixed it by making sure that when they went into the file, the sequence number would get removed.

Resources:
Used while developing the basic client-server setup; visible in both server.c and client.c:
	https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html

Used for remembering how to read from and write to files in C, since it's different in every language; the first source is visible in client.c and the second in server.c:
	https://www.w3schools.com/c/c_files_write.php
	https://www.geeksforgeeks.org/c/fread-function-in-c/

Used the advice from AndersK's answer to read in 1 byte at a time FRAME_SIZE times instead of FRAME_SIZE bytes 1 time 1 at a time while using fread(); this is visible in server.c on line 62:
	https://stackoverflow.com/questions/26412611/reading-a-file-16-bytes-at-a-time-in-ci

In addition, various Linux man pages were used, such as the one for getaddrinfo and errno.

CS4220-001 Computer Networks
Project 1 
Sam Allen
-------------------------------------------------------------------------------------
I have neither given nor received unauthorized assistance on this work.

i. How to build and run the program 
To run this program and successfully transfer data from server to client, 
follow the steps provided below:

1. Open a terminal window 
2. Use your UCCS credentials to ssh into the blanca.uccs.edu server 
(ensure you are first connected to the UCCS VPN, if running from outside the campus)
3. Once connected, create the directory structure using the provided files:

~ > cs4220-networks
        > project1
            > server.c
            > client.c
            > Makefile
            > transmit_me.txt
            > client_files

If you do not want to use the provided transmit_me.txt file, please create/move 
the file you wish to to transport into the project1 directory.

4. Enter the command ```make clean```
5. Enter the command ```make all```
6. Enter the command ```./server```

Your server should now be up and running.

7. Open another terminal window (do not close the previous one)
8. Use your UCCS credentials to ssh into the blanca.uccs.edu server 
9. Once connected, navigate to ~/cs4220-networks/project1
10. Enter the command ```make clean```
11. Enter the command ```make all```
12. Enter the command ```./client localhost transmit_me.txt```

You should see print statements which trace the interactions between the 
client and server. Now when you navigate to project1/client_files/received_file, 
you will see that the file has been copied from server to client. Success!

ii. Components of my submission 
The components of my submission are as follows:
- client.c
The client program which connects to a specified host server and requests a 
specified file from the server. Utilizes the Stop-and-Wait ARQ protocol.

- server.c
The server program which connects to clients and sends over the requested
data. Utilizes the Stop-and-Wait ARQ protocol. 

- Makefile 
Automates building, compiling, and cleaning tasks for the client and server. 

- TestCase.txt
Provides documentation of my testing process for this program, including 
details about my testing approach, results, and observations.

- transmit_me.txt
An example text file which can be used in testing. This file contains 4852
bytes worth of data.

- README.txt
Provides information about various aspects of my submission, including how to 
build and run the program, the components that comprise it, challenges I've 
faced in completing the project, lessons learned, and additional notes about 
resources used to complete the project.

iii. Challenges 
I've faced several challenges in completing this project, mostly relating to 
implementing the Stop-and-Wait ARQ protocol and ensuring that the data was 
transferred and written correctly. Although I have a decent understanding of the 
concepts involved in this project, I was unsure how to apply these concepts 
in C programming to create a working client and server. I had no issues in 
the set-up of my client and server, including creating a TCP socket and initiating 
the various states such as listen, accept, and connect. This is mainly because 
I had a lot of example programs to reference to help me understand the structure
of the program and how the components interact.

To get extra support in implementing the Stop-and-Wait ARQ and verify that my 
current work was correct, I visited Dr. Sullivan during her office hours. She 
answered my questions and cleared up some uncertainty, as well as provided me with 
example programs that implement Stop-and-Wait ARQ. These examples helped me greatly 
in understanding how the protocol would be implemented in context with the rest of
the program. These resources are as follows: 

"Stop and Wait ARQ using C Language" by BunksAllowed
https://www.bunksallowed.com/2023/02/stop-and-wait-arq-using-c-language.html

"Client Server Program in C to Simulate Selective Repeat ARQ" by Computing for Beginners
https://computingforbeginners.blogspot.com/2014/07/client-server-program-in-c-to-simulate.html

Another challenge I faced related to a bug in my code where the data seemed to be 
transferring from server to client correctly, but it was not being written to the 
appropriate file. 

I noticed that for a text file with a large amount of data, my program seemed to write the
last half of the text to the specified location, and for a small text file, it didn't write 
at all. To determine the issue, I first added several debugging statements which printed 
information about the frame received by the client, and the data that was I was attempting 
to write to the file. Then, I added more debugging statements which printed the sequence 
number of the frame and the number of the acknowledgement sent. 

At this point, I noticed that the first frame would always fail to be received by the 
server, and my client would resend the first acknowledgment. I then realized that the first 
frame was not being received by the server at all, and that was why only some of the data 
(1024 bytes, the size of my buffer, to be exact) was not being written to the file. After 
some investigation, I discovered that the root of the issue was that the value for my 
expected sequence number on the server side and the sequence number on the client side 
were inconsistent! This was a simple mistake, but it caused me several issues that I wasn't 
able to fully diagnose right away. With the help of my debugging statements, I was able 
to resolve it.

iv. Lessons learned 

I learned many lessons in completing this project. Some of these lessons are described
below:

- Always remove compiled binaries before running any of the programs. Forgetting this 
step can result in inaccurate results which may complicate the debugging process or 
cause you to think there are bugs when there really aren't any!

- This project reinforced the lesson that debugging statements and descriptive 
error handling messages are incredibly important and provide a lot of insight
about program behavior that help the troubleshooting process immensely. 

- Reviewing many example programs and interacting with the program in real-time
through testing has helped me gain a much deeper understanding of the concepts
involved and how they work. 

v. Notes
To support me in completing this assignment, I used several of the provided 
resources as well as additional resources I've found online. 

For example, to write my server.c and client.c files, I heavily referenced 
the technique and organization of the example programs which are provided 
in section 6.1.4 of the text Computer Networks by Tanenbaum, Feamster, and 
Wetherall. I also re-read sections 6.1.3 and 6.5.2 of this textbook to 
solidify my understanding of the involved concepts. Additionally, I referenced 
the section "Client-Server Background" from the text Beej's Guide to Network 
Programming, which contains example programs for a simple stream server and 
client.

Since this was my first time creating a Makefile, I read some articles and 
watched videos online to help me understand how they work and the most 
appropriate way to write them. The specific resources I used are as follows:

"A Simple Makefile Tutorial" by Colby Computer Science
https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

"How to Create a Simple Makefile - Introduction to Makefiles" by Paul Programming
https://www.youtube.com/watch?v=_r7i5X0rXJk

Furthermore, I also met with Professor Sullivan during her office hours to 
review my work and clear up any confusion or misunderstandings I had about 
completing the project. I did not have any discussions with others regarding 
this assignment. 
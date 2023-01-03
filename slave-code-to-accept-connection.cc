// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080

int main(int argc, char const* argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char* hello = "Hello from client";
	char buffer[1024] = { 0 };
    while (true)
      {
        //printf("trying to connect \n"); 
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr))
		< 0) {
		printf("\n Error: Connect failed \n");
		return -1;

	}
        while ((valread=read(sock,buffer, sizeof(buffer)-1)) > 0)
	{
		buffer[valread] = 0;
                std::cout << buffer;        
     	}
	close(sock);
      }
    sleep(100);
     
}  

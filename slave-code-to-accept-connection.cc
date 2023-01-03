// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define PORT 8080
#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>
//These are time libraries to enable the calculation of timestamps in milliseconds since epoch
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

int main(int argc, char const* argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char* hello = "Hello from client";
	char buffer[1024] = { 0 };
    while (true)
      {
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
		// Append current timestamp to the values received
	                std::cout << buffer;
                auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                cout << "time: " << millisec_since_epoch << endl;        
     	}
	close(sock);
      }
    sleep(100);
     
}  

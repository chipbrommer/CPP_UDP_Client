// CPP_UDP_Client.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "Source/udp_client.h"

int main()
{
	std::cout << "Hello CMake." << std::endl;
	Essentials::Communications::UDP_Client* udp = new Essentials::Communications::UDP_Client();
	std::cout << Essentials::Communications::UdpClientVersion;
	udp->Configure("127.0.0.1",8080,8081);
	std::cout << udp->GetLastError();

	return 0;
}

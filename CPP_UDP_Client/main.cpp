// CPP_UDP_Client.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "Source/udp_client.h"

//#define BROADCAST_TEST
#define MULTICAST_TEST

int main()
{
	std::cout << "Hello CMake." << std::endl;
	Essentials::Communications::UDP_Client* udp = new Essentials::Communications::UDP_Client();
	std::cout << Essentials::Communications::UdpClientVersion;

#ifdef BROADCAST_TEST
	if (udp->EnableBroadcast(8080) < 0)
	{
		std::cout << udp->GetLastError();
	}
	std::string buffer = "Hello World!";
	std::string buffer2 = "Hello From Broadcasting!";

#elif defined UNICAST_TEST
#elif defined MULTICAST_TEST
	if (udp->EnableMulticast("239.255.0.1", 8888) < 0)
	{
		std::cout << udp->GetLastError();
	}

	if (udp->AddMulticastGroup("239.255.0.2", 8880) < 0)
	{
		std::cout << udp->GetLastError();
	}

	std::string buffer = "Hello Group!";
	std::string buffer2 = "Hello From Multicasting!";
#endif // BROADCAST_TEST

	int sendcount = 0;

	for (;;)
	{
#ifdef BROADCAST_TEST
		if (udp->Send(buffer.c_str(), buffer.length(), Essentials::Communications::SendType::BROADCAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}

		if (udp->SendBroadcast(buffer2.c_str(), buffer2.length()) < 1)
		{
			std::cout << "FAIL 2: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
		}
#elif defined UNICAST_TEST
#elif defined MULTICAST_TEST
		if (udp->Send(buffer.c_str(), buffer.length(), Essentials::Communications::SendType::MULTICAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}

		if (udp->SendMulticast(buffer2.c_str(), buffer2.length()) < 1)
		{
			std::cout << "FAIL 2: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
		}
#endif // BROADCAST_TEST

	}

	return 0;
}

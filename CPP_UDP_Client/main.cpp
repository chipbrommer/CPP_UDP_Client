// CPP_UDP_Client.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "Source/udp_client.h"

#define BROADCAST_TEST

int main()
{
	std::cout << "Hello CMake." << std::endl;
	Essentials::Communications::UDP_Client* udp = new Essentials::Communications::UDP_Client();
	std::cout << Essentials::Communications::UdpClientVersion;
	//udp->Configure("127.0.0.1",8080);
	//udp->Open();
	//std::cout << udp->GetLastError();
	udp->EnableBroadcast(8080);
	std::cout << udp->GetLastError();

	std::string buffer = "Hello Receiver!";
	std::string buffer2 = "Hello Again!";

	struct msg
	{
		int8_t iA;
		int8_t iB;
		int32_t test;
	};

	msg MSG = { 1,1,36 };

	int sendcount = 0;

	for (;;)
	{
		//if (udp->Send((char*)&MSG, sizeof(msg)) < 1)
		//{
		//	std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		//}
		//else
		//{
		//	sendcount++;
		//}

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
#endif // BROADCAST_TEST

	}

	return 0;
}

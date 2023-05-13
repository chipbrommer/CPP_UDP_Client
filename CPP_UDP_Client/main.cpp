// CPP_UDP_Client.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "Source/udp_client.h"

int main()
{
	std::cout << "Hello CMake." << std::endl;
	Essentials::Communications::UDP_Client* udp = new Essentials::Communications::UDP_Client();
	std::cout << Essentials::Communications::UdpClientVersion;
	//udp->Configure("127.0.0.1",8080);
	//udp->Open();
	//std::cout << udp->GetLastError();
	udp->EnableBroadcast(5700);
	std::cout << udp->GetLastError();

	char buffer[10] = { 0 };

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

		if (udp->Send(buffer, sizeof(buffer), Essentials::Communications::SendType::BROADCAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
		}
	}

	return 0;
}

#include <iostream>
#include "Source/udp_client.h"

#define UNICAST_SEND_TEST
//#define BROADCAST_SEND_TEST
//#define MULTICAST_SEND_TEST
//#define UNICAST_RECV_TEST
//#define BROADCAST_RECV_TEST
//#define BROADCAST_RECV_SPECIFIC_TEST
//#define MULTICAST_RECV_TEST
//#define MULTICAST_RECV_SPECIFIC_TEST

int main()
{
	std::cout << "Hello CMake." << std::endl;
	Essentials::Communications::UDP_Client* udp = new Essentials::Communications::UDP_Client();
	std::cout << Essentials::Communications::UdpClientVersion;

#ifdef BROADCAST_SEND_TEST
	if (udp->EnableBroadcastSender(8080) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}
	std::string buffer = "Hello World!";
	std::string buffer2 = "Hello From Broadcasting!";

#elif defined UNICAST_SEND_TEST
	if (udp->ConfigureThisClient("127.0.0.1", 5002) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	if (udp->SetUnicastDestination("127.0.0.1", 5001) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	if (udp->OpenUnicast() < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	std::string buffer = "Hello Server!";
	std::string buffer2 = "Hello From Client!";
#elif defined MULTICAST_SEND_TEST
	if (udp->EnableMulticast("239.255.0.1", 8888) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	if (udp->AddMulticastGroup("239.255.0.2", 8880) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	std::string buffer = "Hello Group!";
	std::string buffer2 = "Hello From Multicasting!";
#elif defined MULTICAST_RECV_TEST
	if (udp->EnableMulticast("239.255.0.8", 8099) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
		return -1;
	}

	if (udp->EnableMulticast("239.255.0.7", 8098) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
		return -1;
	}

	char inbuffer[200];
	int size = sizeof(inbuffer);
#elif defined UNICAST_RECV_TEST
	if (udp->ConfigureThisClient("127.0.0.1", 8000) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	if (udp->SetDestination("127.0.0.1", 8001) < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	if (udp->Open() < 0)
	{
		std::cout << udp->GetLastError() << std::endl;
	}

	char buffer[200];
	int size = sizeof(buffer);
#elif defined BROADCAST_RECV_TEST || defined BROADCAST_RECV_SPECIFIC_TEST
	udp->AddBroadcastListener(8000);
	udp->AddBroadcastListener(8002);

	char buffer[200];
	int size = sizeof(buffer);
#endif // TESTS

	int sendcount = 0;

	for (;;)
	{
#ifdef BROADCAST_SEND_TEST
		if (udp->Send(buffer.c_str(), (uint32_t)buffer.length(), Essentials::Communications::SendType::BROADCAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}

		if (udp->SendBroadcast(buffer2.c_str(), (uint32_t)buffer2.length()) < 1)
		{
			std::cout << "FAIL 2: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
			std::cout << "Sent: " << sendcount << std::endl;
		}
#elif defined UNICAST_SEND_TEST
		if (udp->Send(buffer.c_str(), buffer.length(), Essentials::Communications::SendType::UNICAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}

		if (udp->SendUnicast(buffer2.c_str(), buffer2.length()) < 1)
		{
			std::cout << "FAIL 2: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
			std::cout << "Sent: " << sendcount << std::endl;
		}
#elif defined MULTICAST_SEND_TEST
		if (udp->Send(buffer.c_str(), (uint32_t)buffer.length(), Essentials::Communications::SendType::MULTICAST) < 1)
		{
			std::cout << "FAIL: " << udp->GetLastError() << std::endl;
		}

		if (udp->SendMulticast(buffer2.c_str(), (uint32_t)buffer2.length()) < 1)
		{
			std::cout << "FAIL 2: " << udp->GetLastError() << std::endl;
		}
		else
		{
			sendcount++;
			std::cout << "Sent: " << sendcount << std::endl;
		}
#elif defined UNICAST_RECV_TEST
		int bytesReceived = udp->ReceiveUnicast(buffer, size);
		if (bytesReceived == -1)
		{
			std::cout << udp->GetLastError() << std::endl;
		}
		else if(bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::cout << "Received data from: " << udp->GetIpOfLastReceive() << ":" << udp->GetPortOfLastReceive() << ": " << buffer << std::endl;
		}
		else
		{
			std::cout << "No Data." << std::endl;
		}
#elif defined BROADCAST_RECV_TEST
		int16_t p = 0;
		int bytesReceived = udp->ReceiveBroadcast(buffer, size, p);

		if (bytesReceived == -1)
		{
			std::cout << udp->GetLastError() << std::endl;
		}
		else if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::cout << "Received data from " << p << ": " << buffer << std::endl;
		}
		else
		{
			std::cout << "No Data." << std::endl;
		}
#elif defined BROADCAST_RECV_SPECIFIC_TEST
		int16_t p = 8000;
		int bytesReceived = udp->ReceiveBroadcastFromListenerPort(buffer, size, p);

		if (bytesReceived == -1)
		{
			std::cout << udp->GetLastError() << std::endl;
		}
		else if (bytesReceived > 0)
		{
			buffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::cout << "Received data from " << p << ": " << buffer << std::endl;
		}
		else
		{
			std::cout << "No Data." << std::endl;
		}
#elif defined MULTICAST_RECV_TEST
		std::string recvFrom{};
		int bytesReceived = udp->ReceiveMulticast(inbuffer, size, recvFrom);
		std::string buffer2 = "Recieved!";

		if (bytesReceived == -1)
		{
			std::cout << udp->GetLastError() << std::endl;
		}
		else if (bytesReceived > 0)
		{
			inbuffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::cout << "Received data from " << recvFrom << ": " << inbuffer << std::endl;

			if (inbuffer != buffer2)
			{
				udp->SendMulticast(buffer2.c_str(), (uint32_t)buffer2.length());
			}
		}
		else
		{
			std::cout << "No Data." << std::endl;
		}
#ifdef WIN32
		Sleep(1500);
#else
		sleep(1);
#endif

#endif // TESTS
	}

	return 0;
}

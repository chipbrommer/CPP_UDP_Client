///////////////////////////////////////////////////////////////////////////////
//!
//! @file		udp_client.cpp
//! 
//! @brief		Implementation of the udp client class
//! 
//! @author		Chip Brommer
//! 
//! @date		< 04 / 30 / 2023 > Initial Start Date
//!
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//
//  Includes:
//          name                        reason included
//          --------------------        ---------------------------------------
#include	"udp_client.h"				// UDP Client Class
//
///////////////////////////////////////////////////////////////////////////////

namespace Essentials
{
	namespace Communications
	{
		UDP_Client::UDP_Client()
		{
			mTitle				= "UDP Client";
			mLastError			= UdpClientError::NONE;
			mDestinationAddr	= {};
			mClientAddr			= {};
			mBroadcastAddr		= {};
			mLastReceiveInfo	= new Endpoint();

#ifdef WIN32
			mWsaData			= {};
#endif
			mSocket				= INVALID_SOCKET;
			mBroadcastSocket	= INVALID_SOCKET;
		}

		UDP_Client::UDP_Client(const std::string& clientsAddress, const int16_t clientsPort)
		{
			if (ValidateIP(clientsAddress) == -1)
			{
				mLastError = UdpClientError::BAD_ADDRESS;
			}

			if (ValidatePort(clientsPort) == false)
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			// Setup mClientAddr
			memset(reinterpret_cast<char*>(&mClientAddr), 0, sizeof(mClientAddr));
			mClientAddr.sin_family = AF_INET;
			mClientAddr.sin_port = htons(clientsPort);
			if (inet_pton(AF_INET, clientsAddress.c_str(), &(mClientAddr.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
			}

			mTitle				= "TCP Client";
			mLastError			= UdpClientError::NONE;
			mDestinationAddr	= {};
			mBroadcastAddr		= {};

#ifdef WIN32
			mWsaData			= {};
#endif
			mSocket				= INVALID_SOCKET;
			mBroadcastSocket	= INVALID_SOCKET;
		}

		UDP_Client::~UDP_Client()
		{
			CloseUnicast();
			CloseBroadcast();
			CloseMulticast();

#ifdef WIN32
			WSACleanup();
#endif
		}

		int8_t UDP_Client::ConfigureThisClient(const std::string& address, const int16_t port)
		{
			if (ValidateIP(address) == -1)
			{
				mLastError = UdpClientError::BAD_ADDRESS;
				return -1;
			}

			if (ValidatePort(port) == false)
			{
				mLastError = UdpClientError::BAD_PORT;
				return -1;
			}

			// Setup mClientAddr
			memset(reinterpret_cast<char*>(&mClientAddr), 0, sizeof(mClientAddr));
			mClientAddr.sin_family = AF_INET;
			mClientAddr.sin_port = htons(port);
			if (inet_pton(AF_INET, address.c_str(), &(mClientAddr.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::CONFIGURATION_FAILED;
				return -1;
			}

			return 0;
		}

		int8_t  UDP_Client::SetDestination(const std::string& address, const int16_t port)
		{
			if (ValidateIP(address) == -1)
			{
				mLastError = UdpClientError::BAD_ADDRESS;
				return -1;
			}

			if (ValidatePort(port) == false)
			{
				mLastError = UdpClientError::BAD_PORT;
				return -1;
			}

			// Setup mDestinationAddr
			memset(reinterpret_cast<char*>(&mDestinationAddr), 0, sizeof(mDestinationAddr));
			mDestinationAddr.sin_family = AF_INET;
			mDestinationAddr.sin_port = htons(port);
			if (inet_pton(AF_INET, address.c_str(), &(mDestinationAddr.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::SET_DESTINATION_FAILED;
				return -1;
			}

			return 0;
		}

		int8_t UDP_Client::EnableBroadcast(const int16_t port)
		{
			if(mBroadcastSocket != INVALID_SOCKET)
			{
				mLastError = UdpClientError::BROADCAST_ALREADY_ENABLED;
				return -1;
			}

			if (!ValidatePort(port))
			{
				mLastError = UdpClientError::BAD_PORT;
				return -1;
			}

			// Open the broadcast socket
			mBroadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);

			if (mBroadcastSocket == -1)
			{
				mLastError = UdpClientError::BROADCAST_SOCKET_OPEN_FAILURE;
				return -1;
			}

			// Setup broadcast addr
			memset(reinterpret_cast<char*>(&mBroadcastAddr), 0, sizeof(mBroadcastAddr));
			mBroadcastAddr.sin_family = AF_INET;
			mBroadcastAddr.sin_port = htons(port);
			mBroadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

			// set broadcast option
			int8_t broadcast = 1;
			if (setsockopt(mBroadcastSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0)
			{
				mLastError = UdpClientError::ENABLE_BROADCAST_FAILED;
				return -1;
			}

			// success
			return 0;
		}

		int8_t UDP_Client::DisableBroadcast()
		{
			if (mBroadcastSocket == INVALID_SOCKET)
			{
				mLastError = UdpClientError::BROADCAST_NOT_ENABLED;
				return -1;
			}

			if (setsockopt(mBroadcastSocket, SOL_SOCKET, SO_BROADCAST, (char*)0, sizeof(char*)) < 0)
			{
				mLastError = UdpClientError::DISABLE_BROADCAST_FAILED;
				return -1;
			}

			// success
			memset(reinterpret_cast<char*>(&mBroadcastAddr), 0, sizeof(mBroadcastAddr));
			CloseBroadcast();
			return 0;
		}

		int8_t UDP_Client::EnableMulticast(const std::string& groupIP, const int16_t groupPort)
		{
			return AddMulticastGroup(groupIP, groupPort);
		}

		int8_t UDP_Client::DisableMulticast()
		{
			if (mMulticastSockets.size() < 1)
			{
				mLastError = UdpClientError::BROADCAST_NOT_ENABLED;
				return -1;
			}

			// success
			CloseMulticast();
			return 0;
		}

		int8_t UDP_Client::AddMulticastGroup(const std::string& groupIP, const int16_t groupPort)
		{
			if (ValidateIP(groupIP) == -1)
			{
				mLastError = UdpClientError::BAD_ADDRESS;
				return -1;
			}

			if (ValidatePort(groupPort) == false)
			{
				mLastError = UdpClientError::BAD_PORT;
				return -1;
			}

			// Create a UDP socket
			SOCKET tempSock = socket(AF_INET, SOCK_DGRAM, 0);

			if (tempSock == INVALID_SOCKET)
			{
				mLastError = UdpClientError::BAD_MULTICAST_ADDRESS;
				return 1;
			}

			// Set up the multicast group address and port
			sockaddr_in endpoint{};
			endpoint.sin_family = AF_INET;
			endpoint.sin_port = htons(groupPort);  // Multicast group port

			if (inet_pton(AF_INET, groupIP.c_str(), &(endpoint.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::ENABLE_MULTICAST_FAILED;
				return -1;
			}

			// Join the multicast group
			// mreq.imr_interface.s_addr = Interface to join on (use INADDR_ANY to join on all interfaces)
			ip_mreq mreq{};
			mreq.imr_interface.s_addr = INADDR_ANY;

			if (inet_pton(AF_INET, groupIP.c_str(), &(mreq.imr_multiaddr.s_addr)) <= 0)
			{
				mLastError = UdpClientError::BAD_MULTICAST_ADDRESS;
				return -1;
			}

			if (setsockopt(tempSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) == SOCKET_ERROR)
			{
				closesocket(tempSock);
				return -1;
			}

			mMulticastEndpoints.push_back(endpoint);
			mMulticastSockets.push_back(tempSock);

			return true;
		}

		int8_t UDP_Client::Open()
		{
			if (mSocket != -1)
			{
				mLastError = UdpClientError::CLIENT_ALREADY_CONNECTED;
				return -1;
			}

#ifdef WIN32
			if (WSAStartup(MAKEWORD(2, 2), &mWsaData) != 0)
			{
				mLastError = UdpClientError::WINSOCK_FAILURE;
				return -1;
			}
#endif 

			mSocket = socket(AF_INET, SOCK_DGRAM, 0);

			if (mSocket == -1)
			{
				mLastError = UdpClientError::SOCKET_OPEN_FAILURE;
				return -1;
			}

			if (bind(mSocket,(sockaddr*)&mClientAddr, sizeof(mClientAddr)) < 0)
			{
				mLastError = UdpClientError::BIND_FAILED;
				return -1;
			}

			return 0;
		}

		int8_t UDP_Client::Send(const char* buffer, const uint32_t size, const SendType type)
		{
			switch (type)
			{
			case SendType::UNICAST:		return SendUnicast(buffer, size);	break;
			case SendType::BROADCAST:	return SendBroadcast(buffer, size); break;
			case SendType::MULTICAST:	return SendMulticast(buffer, size); break;
			default: return -1;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::SendUnicast(const char* buffer, const uint32_t size)
		{
			// verify socket and then send datagram
			if (mSocket != INVALID_SOCKET)
			{
				int32_t numSent = 0;
				numSent = sendto(mSocket, buffer, size, 0, (struct sockaddr*)&mDestinationAddr, sizeof(mDestinationAddr));

				if (numSent == -1)
				{
					mLastError = UdpClientError::SEND_FAILED;
					return -1;
				}

				// return success
				return numSent;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::SendUnicast(const char* buffer, const uint32_t size, const std::string& ipAddress, const int16_t port)
		{
			// verify socket and then send datagram
			if (mSocket != INVALID_SOCKET)
			{
				if (ValidateIP(ipAddress) == -1)
				{
					mLastError = UdpClientError::BAD_ADDRESS;
					return -1;
				}

				if (ValidatePort(port) == false)
				{
					mLastError = UdpClientError::BAD_PORT;
					return -1;
				}

				// Setup mDestinationAddr
				sockaddr_in sentTo{};
				memset(reinterpret_cast<char*>(&sentTo), 0, sizeof(sentTo));
				sentTo.sin_family = AF_INET;
				sentTo.sin_port = htons(port);
				if (inet_pton(AF_INET, ipAddress.c_str(), &(sentTo.sin_addr)) <= 0)
				{
					mLastError = UdpClientError::SET_DESTINATION_FAILED;
					return -1;
				}

				int32_t numSent = 0;
				numSent = sendto(mSocket, buffer, size, 0, (struct sockaddr*)&sentTo, sizeof(sentTo));

				if (numSent == -1)
				{
					mLastError = UdpClientError::SEND_FAILED;
					return -1;
				}

				// return success
				return numSent;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::SendBroadcast(const char* buffer, const uint32_t size)
		{
			// verify socket and then send datagram
			if (mBroadcastSocket != INVALID_SOCKET)
			{
				int32_t numSent = 0;
				numSent = sendto(mBroadcastSocket, buffer, size, 0, (struct sockaddr*)&mBroadcastAddr, sizeof(sockaddr_in));

				if (numSent == -1)
				{
					mLastError = UdpClientError::SEND_BROADCAST_FAILED;
					return -1;
				}

				// return success
				return numSent;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::SendMulticast(const char* buffer, const uint32_t size)
		{
			// verify socket and then send datagram
			if (mMulticastSockets.size() > 0)
			{
				int32_t numSent = 0;
				for (int i = 0; i < mMulticastSockets.size(); i++)
				{
					numSent = sendto(mMulticastSockets[i], buffer, size, 0, (struct sockaddr*)&mMulticastEndpoints[i], sizeof(sockaddr_in));

					if (numSent < 0)
					{
						mLastError = UdpClientError::SEND_MULTICAST_FAILED;
						return false;
					}
				}

				return numSent;
			}

			return -1;
		}

		int8_t UDP_Client::SendMulticast(const char* buffer, const uint32_t size, const std::string& groupIP)
		{
			// verify socket and then send datagram
			if (mMulticastSockets.size() > 0)
			{

			}

			return -1;
		}

		int8_t UDP_Client::Receive(void* buffer, const uint32_t maxSize)
		{
			// Store the data source info
			sockaddr_in sourceAddress{};
			int addressLength = sizeof(sourceAddress);

			// Hold return value from receiving
			int32_t sizeRead = 0;

			// Receive datagram over UDP
#if defined WIN32
			sizeRead = static_cast<int>(recvfrom(mSocket, reinterpret_cast<char*>(buffer), maxSize, 0, (struct sockaddr*)&sourceAddress, &addressLength));
#else
			sizeRead = static_cast<int>(recvfrom(mSocket, buffer, maxSize, 0, (struct sockaddr*)&sourceAddress, reinterpret_cast<socklen_t*>(&addressLength)));
#endif

			// Check for error
			if (sizeRead == -1)
			{
				mLastError = UdpClientError::READ_FAILED;
				return -1;
			}

			// if data was received, store the port and ip for history. 
			if (sizeRead > 0)
			{
				mLastReceiveInfo->port = ntohs(sourceAddress.sin_port);

				char addy[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &(sourceAddress.sin_addr), addy, INET_ADDRSTRLEN) != NULL)
				{
					mLastReceiveInfo->ipAddress = addy;
				}
			}

			// return size read
			return sizeRead;
		}

		int8_t UDP_Client::Receive(void* buffer, const uint32_t maxSize, std::string& recvFromAddr, int16_t& recvFromPort)
		{
			return -1;
		}

		int8_t UDP_Client::ReceiveMulticast(void* buffer, const uint32_t maxSize, std::string& multicastGroup)
		{
			return -1;
		}

		void UDP_Client::CloseUnicast()
		{
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
		}

		void UDP_Client::CloseBroadcast()
		{
			closesocket(mBroadcastSocket);
			mBroadcastSocket = INVALID_SOCKET;
		}

		void UDP_Client::CloseMulticast()
		{
			for (const auto& socket : mMulticastSockets)
			{
				closesocket(socket);
			}

			mMulticastEndpoints.clear();
			mMulticastSockets.clear();
		}

		std::string UDP_Client::GetIpOfLastReceive()
		{
			return mLastReceiveInfo->ipAddress;
		}

		int16_t UDP_Client::GetPortOfLastReceive()
		{
			return mLastReceiveInfo->port;
		}

		std::string UDP_Client::GetLastError()
		{
			return UdpClientErrorMap[mLastError];
		}
	
		int8_t UDP_Client::ValidateIP(const std::string& ip)
		{
			sockaddr_in sa4 = {};
			sockaddr_in6 sa6 = {};

			// Check if it's a valid IPv4 address
			if (inet_pton(AF_INET, ip.c_str(), &(sa4.sin_addr)) == 1)
			{
				return 1;
			}

			// Check if it's a valid IPv6 address
			if (inet_pton(AF_INET6, ip.c_str(), &(sa6.sin6_addr)) == 1)
			{
				return 2;
			}

			return -1;  // Invalid IP address
		}

		bool UDP_Client::ValidatePort(const int16_t port)
		{
			return (port >= 0 && port <= 65535);
		}

	}
}
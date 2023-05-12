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
			mTitle = "TCP Client";
			mLastError = UdpClientError::NONE;
			mAddress = "";
			mSendPort = -1;
			mRecvPort = -1;
			mBroadcastInfo = new Endpoint();
			mMulticastInfo = new Endpoint();
			mLastReceiveInfo = new Endpoint();
			mBroadcastEnabled = false;
			mMulticastEnabled = false;

#ifdef WIN32
			mWsaData = {};
			mSocket = INVALID_SOCKET;
#else
			mSocket = -1;
#endif
		}

		UDP_Client::UDP_Client(const std::string& address, const int16_t sendPort, const int16_t recvPort, uint32_t bufferSize)
		{
			if (ValidateIP(address) >= 0)
			{
				mAddress = address;
			}
			else
			{
				mLastError = UdpClientError::BAD_ADDRESS;
			}

			if (ValidatePort(sendPort) == true)
			{
				mSendPort = sendPort;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			if (ValidatePort(recvPort) == true)
			{
				mRecvPort = recvPort;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			mTitle = "TCP Client";
			mLastError = UdpClientError::NONE;
			mBroadcastInfo = new Endpoint();
			mMulticastInfo = new Endpoint();
			mLastReceiveInfo = new Endpoint();
			mBroadcastEnabled = false;
			mMulticastEnabled = false;

#ifdef WIN32
			mWsaData = {};
			mSocket = INVALID_SOCKET;
#else
			mSocket = -1;
#endif
		}

		UDP_Client::~UDP_Client()
		{
			Close();
		}

		int8_t UDP_Client::EnableBroadcast(const std::string& multicastIP, const int16_t port)
		{
			if(!mBroadcastEnabled)
			{
				if (setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, (char*)1, sizeof(char*)) < 0)
				{
					//std::cerr << "Failed to enable broadcast." << std::endl;
					return -1;
				}
			}

			return 0;
		}

		int8_t UDP_Client::DisableBroadcast()
		{
			if (!mBroadcastEnabled)
			{
				return -1;
			}
			return -1;
		}

		int8_t UDP_Client::EnableMulticast(const std::string& multicastIP, const int16_t port)
		{
			if (!mMulticastEnabled)
			{
				struct ip_mreq mreq;
				mreq.imr_multiaddr.s_addr = inet_addr(multicastIP.c_str());
				mreq.imr_interface.s_addr = htonl(INADDR_ANY);
				if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
				{
					//std::cerr << "Failed to enable multicast." << std::endl;
					return -1;
				}
			}
			else
			{
				DisableMulticast();
				EnableMulticast(multicastIP, port);
			}

			return 0;
		}

		int8_t UDP_Client::DisableMulticast()
		{
			return -1;
		}

		int8_t UDP_Client::Configure(const std::string& address, const int16_t sendPort, const int16_t recvPort)
		{
			if (ValidateIP(address) >= 0)
			{
				mAddress = address;
			}
			else
			{
				mLastError = UdpClientError::BAD_ADDRESS;
			}

			if (ValidatePort(sendPort) == true)
			{
				mSendPort = sendPort;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			if (ValidatePort(recvPort) == true)
			{
				mRecvPort = recvPort;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			return 0;
		}

		int8_t UDP_Client::AddMulticastEndpoint(const std::string& address, const int16_t port)
		{
			int ipType = ValidateIP(address);

			if (ipType == 1 || ipType == 2) 
			{
				if (ValidatePort(port)) 
				{
					Endpoint endpoint(address, port);
					mMulticastEndpoints.push_back(endpoint);
				}
				else 
				{
					//std::cerr << "Invalid port for multicast endpoint: " << address << ":" << port << std::endl;
					return -1;
				}
			}
			else 
			{
				//std::cerr << "Invalid multicast endpoint: " << address << std::endl;
				return -1;
			}
		}

		int8_t UDP_Client::Open()
		{
			if (mAddress == "\n" || mAddress.empty())
			{
				mLastError = UdpClientError::ADDRESS_NOT_SET;
				return -1;
			}

			if (mSendPort == -1)
			{
				mLastError = UdpClientError::PORT_NOT_SET;
				return -1;
			}

			if (mRecvPort == -1)
			{
				mLastError = UdpClientError::PORT_NOT_SET;
				return -1;
			}

#ifdef WIN32
			if (WSAStartup(MAKEWORD(2, 2), &mWsaData) != 0)
			{
				mLastError = UdpClientError::WINSOCK_FAILURE;
				return -1;
			}

			mSocket = socket(AF_INET, SOCK_STREAM, 0);

			if (mSocket == INVALID_SOCKET)
			{
				mLastError = UdpClientError::WINDOWS_SOCKET_OPEN_FAILURE;
				WSACleanup();
				return -1;
			}
#else
			mSocket = socket(AF_INET, SOCK_STREAM, 0);

			if (mSocket == -1)
			{
				mLastError = UdpClientError::LINUX_SOCKET_OPEN_FAILURE;
				return -1;
			}
#endif
			// Set up server details
			sockaddr_in serverAddress{};
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_port = htons(mSendPort);
			if (inet_pton(AF_INET, mAddress.c_str(), &(serverAddress.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
				return -1;
			}

			// Connect to the server
			if (connect(mSocket, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
			{
				mLastError = UdpClientError::CONNECTION_FAILED;
				Close();
				return -1;
			}

			return 0;
		}

		int8_t UDP_Client::Send(const char* buffer, const uint8_t size, const SendType type)
		{
			int sizeSent = send(mSocket, buffer, size, 0);
			if (sizeSent < 0)
			{
				mLastError = UdpClientError::SEND_FAILED;
				return -1;
			}

			return sizeSent;
		}

		int8_t UDP_Client::SendBroadcast(const char* buffer, const uint8_t size)
		{
			if (!mBroadcastEnabled)
			{
				return -1;
			}

			return -1;
		}

		int8_t UDP_Client::SendMulticast(const char* buffer, const uint8_t size)
		{
			if (!mMulticastEnabled)
			{
				return -1;
			}

			return -1;
		}

		int8_t UDP_Client::Receive(void* buffer, const uint8_t maxSize)
		{
			int sizeRead = recv(mSocket, (char*)buffer, maxSize, 0);
			if (sizeRead < 0)
			{
				mLastError = UdpClientError::READ_FAILED;
				return -1;
			}

			// Copy temp buffer into param buffer and return size read. 
			return sizeRead;
		}

		void UDP_Client::Close()
		{
#ifdef WIN32
			closesocket(mSocket);
			WSACleanup();
			mSocket = INVALID_SOCKET;
#else
			close(mSocket);
			mSocket = -1;
#endif
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
			struct sockaddr_in sa4;
			struct sockaddr_in6 sa6;

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
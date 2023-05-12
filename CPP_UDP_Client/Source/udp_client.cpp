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
			mTitle = "UDP Client";
			mLastError = UdpClientError::NONE;
			mAddress = "";
			mPort = -1;
			mBroadcastAddr = {};
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

		UDP_Client::UDP_Client(const std::string& address, const int16_t port)
		{
			if (ValidateIP(address) >= 0)
			{
				mAddress = address;
			}
			else
			{
				mLastError = UdpClientError::BAD_ADDRESS;
			}

			if (ValidatePort(port) == true)
			{
				mPort = port;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
			}

			mTitle = "TCP Client";
			mLastError = UdpClientError::NONE;
			mBroadcastAddr = {};
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
			CloseBroadcast();
		}

		int8_t UDP_Client::EnableBroadcast(const std::string& address, const int16_t port)
		{
			if(!mBroadcastEnabled)
			{
#ifdef WIN32
				if (WSAStartup(MAKEWORD(2, 2), &mWsaData) != 0)
				{
					mLastError = UdpClientError::WINSOCK_FAILURE;
					return -1;
				}
#endif 

				// Open the broadcast socket
				mBroadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);

				if (mBroadcastSocket == -1)
				{
					mLastError = UdpClientError::SOCKET_OPEN_FAILURE;
					return -1;
				}

				// Setup broadcast addr
				memset(reinterpret_cast<char*>(&mBroadcastAddr), 0, sizeof(mBroadcastAddr));
				mBroadcastAddr.sin_family = AF_INET;
				mBroadcastAddr.sin_port = htons(mPort);

				if (address == "")
				{
					mBroadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
				}
				else
				{
					// set broadcast address
					if (inet_pton(AF_INET, address.c_str(), &(mBroadcastAddr.sin_addr)) <= 0)
					{
						mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
						return -1;
					}
				}

				// set broadcast option
				int broadcast = 1;
				if (setsockopt(mBroadcastSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0)
				{
					mLastError = UdpClientError::ENABLE_BROADCAST_FAILED;
					return -1;
				}

				// success
				mBroadcastEnabled = true;
				return 0;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::DisableBroadcast()
		{
			if (mBroadcastEnabled)
			{
				if (setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, (char*)0, sizeof(char*)) < 0)
				{
					mLastError = UdpClientError::DISABLE_BROADCAST_FAILED;
					return -1;
				}

				// success
				mBroadcastEnabled = false;
				memset(reinterpret_cast<char*>(&mBroadcastAddr), 0, sizeof(mBroadcastAddr));
				CloseBroadcast();
				return 0;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::EnableMulticast(const std::string& multicastIP, const int16_t port)
		{
			if (!mMulticastEnabled)
			{
				ip_mreq mreq = {};

				if (inet_pton(AF_INET, multicastIP.c_str(), &(mreq.imr_multiaddr.s_addr)) <= 0)
				{
					mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
					return -1;
				}

				mreq.imr_interface.s_addr = htonl(INADDR_ANY);
				if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0)
				{
					mLastError = UdpClientError::ENABLE_MULTICAST_FAILED;
					return -1;
				}

				// success
				mMulticastEnabled = true;
				return 0;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::DisableMulticast()
		{
			if (mMulticastEnabled)
			{
				if (setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)0, sizeof(char*)) < 0)
				{
					mLastError = UdpClientError::DISABLE_MULTICAST_FAILED;
					return -1;
				}

				// succss
				mMulticastEnabled = false;
				return 0;
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::Configure(const std::string& address, const int16_t port)
		{
			if (ValidateIP(address) >= 0)
			{
				mAddress = address;
			}
			else
			{
				mLastError = UdpClientError::BAD_ADDRESS;
				return -1;
			}

			if (ValidatePort(port) == true)
			{
				mPort = port;
			}
			else
			{
				mLastError = UdpClientError::BAD_PORT;
				return -1;
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
					sockaddr_in endpoint;
					memset(&endpoint, 0, sizeof(endpoint));
					endpoint.sin_family = AF_INET;
					endpoint.sin_port = htons(port);

					if (inet_pton(AF_INET, address.c_str(), &(endpoint.sin_addr)) <= 0) 
					{
						mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
						return -1;
					}

					// success
					mMulticastEndpoints.push_back(endpoint);
					return 0;
				}
				else 
				{
					mLastError = UdpClientError::BAD_PORT;
					return -1;
				}
			}
			else 
			{
				mLastError = UdpClientError::BAD_ADDRESS;
				return -1;
			}

			// Default return
			return -1;
		}

		int8_t UDP_Client::Open()
		{
			if (mSocket != -1)
			{
				mLastError = UdpClientError::CLIENT_ALREADY_CONNECTED;
				return -1;
			}

			if (ValidateIP(mAddress) == -1)
			{
				mLastError = UdpClientError::ADDRESS_NOT_SET;
				return -1;
			}

			if (!ValidatePort(mPort))
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
#endif 

			mSocket = socket(AF_INET, SOCK_DGRAM, 0);

			if (mSocket == -1)
			{
				mLastError = UdpClientError::SOCKET_OPEN_FAILURE;
				return -1;
			}

			// Setup destination addr
			memset(reinterpret_cast<char*>(&mDestinationAddr), 0, sizeof(mDestinationAddr));
			mDestinationAddr.sin_family = AF_INET;
			mDestinationAddr.sin_port = htons(mPort);

			// set destination address
			if (inet_pton(AF_INET, mAddress.c_str(), &(mDestinationAddr.sin_addr)) <= 0)
			{
				mLastError = UdpClientError::ADDRESS_NOT_SUPPORTED;
				return -1;
			}

			return 0;
		}

		int8_t UDP_Client::Send(const char* buffer, const uint32_t size, const SendType type)
		{
			switch (type)
			{
			case SendType::BROADCAST: return SendBroadcast(buffer, size); break;
			case SendType::MULTICAST: return SendMulticast(buffer, size); break;
			case SendType::UNICAST:	// Intientional fall through
			default:					
				{
				// Send datagram over UDP
				int32_t numSent = 0;
				numSent = sendto(mSocket, buffer, size, 0, (struct sockaddr*)&mDestinationAddr, sizeof(sockaddr_in));

				if (numSent == -1)
				{
					mLastError = UdpClientError::SEND_FAILED;
					return -1;
				}

				// return success
				return numSent;
				}
			}

			// default return
			return -1;
		}

		int8_t UDP_Client::SendBroadcast(const char* buffer, const uint32_t size)
		{
			if (mBroadcastEnabled && mBroadcastSocket != -1)
			{
				int32_t numSent = 0;
				numSent = sendto(mBroadcastSocket, buffer, size, 0, (struct sockaddr*)&mBroadcastAddr, sizeof(mBroadcastAddr));

				if (numSent == -1)
				{
					mLastError = UdpClientError::SEND_BROADCAST_FAILED;
					return -1;
				}

				// return success
				return numSent;
			}

			return -1;
		}

		int8_t UDP_Client::SendMulticast(const char* buffer, const uint32_t size)
		{
			if (mMulticastEnabled)
			{
				int32_t numSent = 0;
				for (const auto& endpoint : mMulticastEndpoints) 
				{
					numSent = sendto(mSocket, buffer, size, 0, (struct sockaddr*)&endpoint, sizeof(endpoint));

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

		int8_t UDP_Client::Receive(void* buffer, const uint32_t maxSize)
		{
			// Store the data source info
			sockaddr_in sourceAddress;
			int addressLength = sizeof(sourceAddress);

			// Hold return value from receiving
			int32_t sizeRead = 0;

			// Receive datagram over UDP
#if defined WIN32
			sizeRead = static_cast<int>(recvfrom(mSocket, reinterpret_cast<char*>(buffer), maxSize, 0, (struct sockaddr*)&sourceAddress, &addressLength));
#elif defined __linux___
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

		void UDP_Client::Close()
		{
#ifdef WIN32
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			WSACleanup();
#else
			close(mSocket);
			mSocket = -1;
#endif
		}

		void UDP_Client::CloseBroadcast()
		{
#ifdef WIN32
			closesocket(mBroadcastSocket);
			mBroadcastSocket = INVALID_SOCKET;
#else
			close(mBroadcastSocket);
			mBroadcastSocket = -1;
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
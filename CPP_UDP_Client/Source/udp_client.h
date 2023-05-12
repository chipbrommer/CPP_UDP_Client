///////////////////////////////////////////////////////////////////////////////
//!
//! @file		udp_client.h
//! 
//! @brief		A cross platform class to handle UDP communication.
//! 
//! @author		Chip Brommer
//! 
//! @date		< 04 / 30 / 2023 > Initial Start Date
//!
/*****************************************************************************/
#pragma once
///////////////////////////////////////////////////////////////////////////////
//
//  Includes:
//          name                        reason included
//          --------------------        ---------------------------------------
#ifdef WIN32
#include <stdint.h>						// Standard integer types
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
const int SD_BOTH = SHUT_RDWR;
#define closesocket(s) close(s)
#endif
#include <map>							// Error enum to strings.
#include <string>						// Strings
#include <regex>						// Regular expression for ip validation
//
//	Defines:
//          name                        reason defined
//          --------------------        ---------------------------------------
#ifndef     CPP_UDP_CLIENT				// Define the cpp UDP client class. 
#define     CPP_UDP_CLIENT
//
///////////////////////////////////////////////////////////////////////////////

namespace Essentials
{
	namespace Communications
	{
		constexpr static uint8_t	UDP_CLIENT_VERSION_MAJOR	= 0;
		constexpr static uint8_t	UDP_CLIENT_VERSION_MINOR	= 1;
		constexpr static uint8_t	UDP_CLIENT_VERSION_PATCH	= 0;
		constexpr static uint8_t	UDP_CLIENT_VERSION_BUILD	= 0;
		constexpr static uint32_t	UDP_CLIENT_MAX_BUFFER_SIZE	= 65536;

		static std::string UdpClientVersion = "UDP Client v" +
			std::to_string((uint8_t)UDP_CLIENT_VERSION_MAJOR) + "." +
			std::to_string((uint8_t)UDP_CLIENT_VERSION_MINOR) + "." +
			std::to_string((uint8_t)UDP_CLIENT_VERSION_PATCH) + " - b" +
			std::to_string((uint8_t)UDP_CLIENT_VERSION_BUILD) + ".\n";

		/// <summary>enum for error codes</summary>
		enum class UdpClientError : uint8_t
		{
			NONE,
			BAD_ADDRESS,
			ADDRESS_NOT_SET,
			BAD_PORT,
			PORT_NOT_SET,
			CLIENT_ALREADY_CONNECTED,
			FAILED_TO_CONNECT,
			WINSOCK_FAILURE,
			SOCKET_OPEN_FAILURE,
			ADDRESS_NOT_SUPPORTED,
			CONNECTION_FAILED,
			SEND_FAILED,
			READ_FAILED,
			ENABLE_MULTICAST_FAILED,
			DISABLE_MULTICAST_FAILED,
			ENABLE_BROADCAST_FAILED,
			DISABLE_BROADCAST_FAILED,
			SEND_MULTICAST_FAILED,
			SEND_BROADCAST_FAILED,
		};

		/// <summary>Error enum to string map</summary>
		static std::map<UdpClientError, std::string> UdpClientErrorMap
		{
			{UdpClientError::NONE,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::NONE) + ": No error.")},
			{UdpClientError::BAD_ADDRESS,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::BAD_ADDRESS) + ": Bad address.")},
			{UdpClientError::ADDRESS_NOT_SET,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::ADDRESS_NOT_SET) + ": Address not set.")},
			{UdpClientError::BAD_PORT,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::BAD_PORT) + ": Bad port.")},
			{UdpClientError::PORT_NOT_SET,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::PORT_NOT_SET) + ": Port not set.")},
			{UdpClientError::FAILED_TO_CONNECT,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::FAILED_TO_CONNECT) + ": Failed to connect.")},
			{UdpClientError::WINSOCK_FAILURE,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::WINSOCK_FAILURE) + ": Winsock creation failure.")},
			{UdpClientError::SOCKET_OPEN_FAILURE,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::SOCKET_OPEN_FAILURE) + ": Socket open failure.")},
			{UdpClientError::ADDRESS_NOT_SUPPORTED,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::ADDRESS_NOT_SUPPORTED) + ": Address not supported.")},
			{UdpClientError::CONNECTION_FAILED,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::CONNECTION_FAILED) + ": Connection failed.")},
			{UdpClientError::SEND_FAILED,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::SEND_FAILED) + ": Send failed.")},
			{UdpClientError::READ_FAILED,
			std::string("Error Code " + std::to_string((uint8_t)UdpClientError::READ_FAILED) + ": Read failed.")},
		};

		/// <summary>Represents an endpoint for a connection</summary>
		struct Endpoint
		{
			std::string ipAddress = "";
			int16_t	port = 0;
		};

		/// <summary>Send Type for the Send Function.</summary>
		enum class SendType : uint8_t
		{
			UNICAST,
			BROADCAST,
			MULTICAST,
		};

		/// <summary>A multi-platform class to handle UDP communications.</summary>
		class UDP_Client
		{
		public:
			/// <summary>Default Constructor</summary>
			UDP_Client();

			/// <summary>Constructor to receive an address and port</summary>
			UDP_Client(const std::string& address, const int16_t port);

			/// <summary>Default Deconstructor</summary>
			~UDP_Client();

			/// <summary>A function to enable broadcasting</summary>
			/// <param name="address"> -[in]- Address to broadcast on, send empty string to broadcast on all addresses.</param>
			/// <param name="port"> -[in]- Port to broadcast on</param>
			//// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t EnableBroadcast(const std::string& address, const int16_t port);

			/// <summary>Disables broadcast and cleans up</summary>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t DisableBroadcast();

			/// <summary></summary>
			/// <param name="multicastIP"> -[in]- Address of multicast group.</param>
			/// <param name="port"> -[in]- Port of multicast sender.</param>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t EnableMulticast(const std::string& multicastIP, const int16_t port);

			/// <summary>Disables multicast and cleans up</summary>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t DisableMulticast();

			/// <summary>Configure the client</summary>
			/// <param name="address"> -[in]- Address of the server</param>
			/// <param name="port"> -[in]- Port of the client</param>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t Configure(const std::string& address, const int16_t port);

			/// <summary>Add an enpoint to the list of multicast recepients.</summary>
			/// <param name="ipAddress"> -[in]- IP address</param>
			/// <param name="port"> -[in]- Port</param>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t AddMulticastEndpoint(const std::string& ipAddress, const int16_t port);

			/// <summary>Opens the UDP socket at the set address and ports</summary>
			/// <returns>0 if successful, -1 if fails. Call Serial::GetLastError to find out more.</returns>
			int8_t Open();

			/// <summary>Send a message. Default is normal, can specify Broadcase or Multicast for ease of use.</summary>
			/// <param name="buffer"> -[in]- Buffer to be sent</param>
			/// <param name="size"> -[in]- Size to be sent</param>
			/// <returns>0+ if successful (number bytes sent), -1 if fails. Call UDP_Client::GetLastError to find out more.</returns>
			int8_t Send(const char* buffer, const uint32_t size, const SendType type = SendType::UNICAST);

			/// <summary>Send a broadcast message</summary>
			/// <param name="buffer"> -[in]- Buffer to be sent</param>
			/// <param name="size"> -[in]- Size to be sent</param>
			/// <returns>0+ if successful (number bytes sent), -1 if fails. Call UDP_Client::GetLastError to find out more.</returns>
			int8_t SendBroadcast(const char* buffer, const uint32_t size);

			/// <summary>Send a multicast message.</summary>
			/// <param name="buffer"> -[in]- Buffer to be sent</param>
			/// <param name="size"> -[in]- Size to be sent</param>
			/// <returns>0+ if successful (number bytes sent), -1 if fails. Call UDP_Client::GetLastError to find out more.</returns>
			int8_t SendMulticast(const char* buffer, const uint32_t size);

			/// <summary>Receive data from a server</summary>
			/// <param name="buffer"> -[out]- Buffer to place received data into</param>
			/// <param name="maxSize"> -[in]- Maximum number of bytes to be read</param>
			/// <returns>0+ if successful (number bytes received), -1 if fails. Call UDP_Client::GetLastError to find out more.</returns>
			int8_t Receive(void* buffer, const uint32_t maxSize);

			/// <summary>Closes the client and cleans up</summary>
			void Close();

			/// <summary>Closes the broadcast client and cleans up</summary>
			void CloseBroadcast();

			/// <summary>Get the ip address of the last received message.</summary>
			/// <returns>If valid, A string containing the IP address; else an empty string. Call UDP_Client::GetLastError to find out more.</returns>
			std::string GetIpOfLastReceive();

			/// <summary>Get the port number of the last received message.</summary>
			/// <returns>The port number, else -1 on error. Call UDP_Client::GetLastError to find out more.</returns>
			int16_t GetPortOfLastReceive();

			/// <summary>Get the last error in string format</summary>
			/// <returns>The last error in a formatted string</returns>
			std::string GetLastError();

		protected:
		private:
			/// <summary>Validates an IP address is IPv4 or IPv6</summary>
			/// <param name="ip"> -[in]- IP Address to be validated</param>
			/// <returns>1 if valid ipv4, 2 if valid ipv6, else -1 on fail</returns>
			int8_t ValidateIP(const std::string& ip);

			/// <summary>Validates a port number is between 0-65535</summary>
			/// <param name="port"> -[in]- Port number to be validated</param>
			/// <returns>true = valid, false = invalid</returns>
			bool ValidatePort(const int16_t port);

			// Variables
			std::string					mTitle;					// Title for this utility when using CPP_Logger
			UdpClientError				mLastError;				// Last error for this utility
			std::string					mAddress;				// The clients IP address
			int16_t						mPort;					// The Port to send on. 
			sockaddr_in					mDestinationAddr;		// Destination sockaddr
			sockaddr_in					mBroadcastAddr;			// Broadcast sockaddr
			Endpoint*					mMulticastInfo;			// Multicast group
			Endpoint*					mLastReceiveInfo;		// Last receive endpoint info
			std::vector<sockaddr_in>	mMulticastEndpoints;	// List of multicast endpoints
			bool						mBroadcastEnabled;		// Flag for enabled/disabled broadcast
			bool						mMulticastEnabled;		// Flag for enabled/disabled multicast

#ifdef WIN32
			WSADATA						mWsaData;				// Winsock data
#endif
			SOCKET						mSocket;				// socket FD
			SOCKET						mBroadcastSocket;		// Broadcast socket FD
		};
	}
}

#endif		// CPP_UDP_CLIENT
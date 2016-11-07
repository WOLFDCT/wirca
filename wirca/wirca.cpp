// Windows IRC Agent
// 2016-11-06 DW

#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char **argv)
{

	// Version string
	std::string Version = "wirca v0.1 - A Windows IRC Agent";

	// Print version string always
	cout << Version << endl << endl;

	// Check argument count
	if (argc < 5) {
		cout << "Usage: " << argv[0] << " <HOST> <PORT> <USER> <CHANNEL>" << endl;
		return 1;
	}

	// Buffers
	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];
	char suffix[512];

	// Rename arguments
	std::string HOST = argv[1];
	std::string PORT = argv[2];
	std::string USER = argv[3];
	std::string CHAN = argv[4];

	// IRC protocol connection strings
	std::string SEND_NICK = "NICK " + USER + "\r\n";
	std::string SEND_USER = "USER " + USER + " 127.0.0.1 bla :" + USER + "\r\n";
	std::string SEND_JOIN = "JOIN " + CHAN + "\r\n";

	int firstloop = 0;

	// Main loop
	for (;;) {

		if (firstloop > 0) {
			// Sleep for 60 seconds to keep the loop under control
			cout << "sleeping for 60 seconds ..." << endl;
			Sleep(60 * 1000);
		}

		firstloop = 1;

		// Initialize Winsock
		WSADATA wsaData;
		int iResult;
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			cout << "WSAStartup Failed: " << iResult << endl;
			continue;
		}

		// addrinfo object
		struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(&HOST[0], &PORT[0], &hints, &result);
		if (iResult != 0) {
			cout << "getaddrinfo failed: " << iResult << endl;
			WSACleanup();
			continue;
		}

		SOCKET ConnectSocket = INVALID_SOCKET;

		//Attempt to conncet to the first address returned by the call to getaddrinfo
		ptr = result;

		//Create a socket for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		//Set socket options (Timeout for recv())
		DWORD timeout = 600 * 1000;
		if (setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)))
		{
			perror("setsockopt");
			cout << "error setting socket option" << endl;
			continue;
		}

		if (ConnectSocket == INVALID_SOCKET) {
			cout << "Error at socket(): " << WSAGetLastError() << endl;
			freeaddrinfo(result);
			WSACleanup();
			continue;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if (ConnectSocket == INVALID_SOCKET) {
			cout << "unable to connect to " << HOST << endl;
			WSACleanup();
			continue;
		}

		// Send NICK data
		cout << ">> " << SEND_NICK << endl;
		iResult = send(ConnectSocket, SEND_NICK.c_str(), SEND_NICK.length(), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			WSACleanup();
			continue;
		}
		cout << "bytes sent: " << iResult << endl;

		// Send USER data
		cout << ">> " << SEND_USER << endl;
		iResult = send(ConnectSocket, SEND_USER.c_str(), SEND_USER.length(), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			WSACleanup();
			continue;
		}
		cout << "bytes sent: " << iResult << endl;

		// Send JOIN data
		cout << ">> " << SEND_JOIN << endl;
		iResult = send(ConnectSocket, SEND_JOIN.c_str(), SEND_JOIN.length(), 0);
		if (iResult == SOCKET_ERROR) {
			cout << "send failed: " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			WSACleanup();
			continue;
		}
		cout << "bytes sent: " << iResult << endl;

		// Receive data until the server closes the connection
		do {
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				cout << "bytes received: " << iResult << endl;
				cout << recvbuf << endl;
				if (sscanf_s(recvbuf, "PING :%s", suffix, 512) > 0) {
					sprintf_s(recvbuf, "PONG :%s\r\n", suffix);
					send(ConnectSocket, recvbuf, strlen(recvbuf), 0);
					cout << "response sent: " << recvbuf << endl;
				}
				memset(recvbuf, 0, sizeof recvbuf);
			}
			else if (iResult == 0)
				cout << "connection closed" << endl;
			else
				cout << "recv failed: " << WSAGetLastError() << endl;
		} while (iResult > 0);
	}

	return 0;
}



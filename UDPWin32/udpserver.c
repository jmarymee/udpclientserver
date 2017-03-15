/*
Simple UDP Server
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 512  //Max length of buffer
#define PORT 5683   //The port on which to listen for incoming data

int PrintReceivedData(char *buf, int len, IN_ADDR addr, USHORT port);

typedef int*(*fnPtrDisplayRoutine)(char *buf, int len, IN_ADDR addr, USHORT port);

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;
	//int recCode = 0;
	int nError = 0;

	fnPtrDisplayRoutine display = &PrintReceivedData; //fnptr to the dsiplay routine

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//Now put into non-blocking mode
	u_long iMode = 1; //nonzero for non-blocking, zero (default) for blocking
	int ccode = ioctlsocket(s, FIONBIO, &iMode);

	//keep listening for data
	while (1)
	{
		printf("Waiting for data...\n");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		//if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		//{
		//	printf("recvfrom() failed with error code : %d", WSAGetLastError());
		//	exit(EXIT_FAILURE);
		//}

		recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
		if (recv_len == 0)
		{
			nError = WSAGetLastError();
			switch (nError)
			{
			case WSAEWOULDBLOCK:
				continue;
				break;
			default:
				printf("Error on receive. Value is: %d\n", nError);
				break;
			}
		}
		else if (recv_len > 0)
		{
			int t = display(buf, 512, si_other.sin_addr, si_other.sin_port);

			//print details of the client/peer and the data received
			//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			//printf("Data: %s\n", buf);

			//now reply the client with the same data
			if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				//exit(EXIT_FAILURE);
			}
		}
		Sleep(1000);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}

int PrintReceivedData(char *buf, int len, IN_ADDR addr, USHORT port)
{
	//print details of the client/peer and the data received
	printf("Received packet from %s:%d\n", inet_ntoa(addr), ntohs(port));
	printf("Data: %s\n", buf);

	return 21;
}

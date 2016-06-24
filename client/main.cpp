//#include <winsock2.h>
#include <windows.h>
#include <conio.h>
#include <string>
#include <deque>
#include "main.h"

#pragma comment(lib,"Ws2_32.lib")

enum PacketType
{
	PACKET_TYPE_SIGN = 1,
	PACKET_TYPE_SIGN_RES = 2,

	PACKET_TYPE_CHATTING = 4,

};


#pragma pack(push,1 )
struct PacketBase
{
	int type;
	int length;
};

struct SignPacket : PacketBase
{
	char id[32];
	char password[32];
};
struct SignResPacket : PacketBase
{
	enum ResPacketType
	{
		SUCCESS = 0,
		INVALID_ACCOUNT = 1,
		DUPLICATE_CONNECT = 2
	};

	int responseType;
	char message[128];
};
struct MessagePacket : PacketBase
{
	MessagePacket()
	{
		memset(message, 0, sizeof(message));
		memset(sendId, 0, sizeof(sendId));
		memset(whisperId, 0, sizeof(whisperId));

		isWhisper = false;
	}
	char message[1024];
	char sendId[32];
	bool isWhisper;
	char whisperId[32];
};

#pragma pack(pop)
SOCKET serverSocket;
std::deque<std::string> printText;

#define BUFFER_SIZE 4096

void gotoxy(int x, int y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

HANDLE consoleHandler = GetStdHandle(STD_OUTPUT_HANDLE);
int wherex()
{
	if (consoleHandler == INVALID_HANDLE_VALUE)
		return 0;
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(consoleHandler, &screenInfo);
	return screenInfo.dwCursorPosition.X + 1;
}
int wherey()
{
	if (consoleHandler == INVALID_HANDLE_VALUE)
		return 0;
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(consoleHandler, &screenInfo);
	return screenInfo.dwCursorPosition.Y + 1;
}


void Clear()
{
	for (int i = 0; i < 15; ++i)
	{
		gotoxy(0, i);
		printf("                                                                   ");
	}
}
void Print()
{
	int prevX = wherex()-1;
	int prevY = wherey() - 1;
	Clear();
	gotoxy(0, 0);
	for (int i = 0; i < printText.size(); ++i)
	{
		printf("%s\n", printText[i].c_str());
	}
	gotoxy(prevX, prevY);
}

void PushPrintText(std::string string)
{
	if (printText.size() > 15)
	{
		printText.pop_front();
	}

	printText.push_back(string);

	Print();
}

DWORD WINAPI RecvThread(void* param)
{
	char message[BUFFER_SIZE] = { 0 };
	char temp[BUFFER_SIZE];
	while (true)
	{
		int recvLength = recv(serverSocket, message, BUFFER_SIZE - 1, 0);

		if (recvLength > 0 )
		{
			PacketBase* packetBase = (PacketBase*)message;

			if (packetBase->type == PacketType::PACKET_TYPE_CHATTING)
			{
				MessagePacket* messagePacket = (MessagePacket*)message;

				if (messagePacket->isWhisper)
				{
					sprintf_s(temp, "%s(s) : %s", messagePacket->sendId, messagePacket->message);
					PushPrintText(temp);
				}
				else
				{
					sprintf_s(temp, "%s : %s", messagePacket->sendId, messagePacket->message);
					PushPrintText(temp);
				}
			}
			else
			{
				PushPrintText("Client : 정의되지 않은 패킷이 들어옴");
			}
		}
	}
	return 0;
}

void main()
{
	std::deque<std::string> messageList;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		_getch();
		return;
	}
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket create error \n");
		_getch();
		return;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(serverSocket, (sockaddr*)& serverAddr, sizeof(serverAddr) ) == SOCKET_ERROR )
	{
		printf("서버 접속 실패 \n");
		_getch();
		return;
	}


	char message[BUFFER_SIZE] = { 0 };
	int index = 0;


	std::string id;
	std::string prevMessage;

	//로그인 및 회원 가입
	{
		while (true)
		{
			SignPacket packet;
			packet.type = PacketType::PACKET_TYPE_SIGN;
			packet.length = sizeof(packet);

			printf("ID : ");
			scanf_s("%s", packet.id, 32);
			fflush(stdin);
			printf("Pass : ");
			scanf_s("%s", packet.password, 32);

			if (strlen(packet.id) == 0 || strlen(packet.password) == 0)
			{
				printf("아이디와 비밀번호를 입력하세요\n");
				continue;
			}

			id = packet.id;

			send(serverSocket, (char*)&packet, packet.length, 0);
			
			int recvLength = recv(serverSocket, message, BUFFER_SIZE - 1, 0);

			if (recvLength > 0)
			{
				SignResPacket* resPacket = (SignResPacket*)message;

				if (resPacket->responseType == SignResPacket::SUCCESS)
				{
					system("cls");
					PushPrintText(resPacket->message);

					break;
				}
				else if (resPacket->responseType == SignResPacket::INVALID_ACCOUNT)
				{
					printf("비밀번호가 틀렸습니다\n");
				}
				else if (resPacket->responseType == SignResPacket::DUPLICATE_CONNECT)
				{
					printf("이미 접속중 입니다\n");
				}
			}
		}
	}


	//recv thread 시작
	DWORD threadid = 0;
	CreateThread(nullptr, 0, RecvThread, nullptr, 0, &threadid);

	while (true)
	{
		fflush(stdin);
		gotoxy(0, 23);
		printf("                                                                   ");
		gotoxy(0, 23);
		printf("enter message : ");
		gets_s(message, BUFFER_SIZE);

		if (strlen(message) <= 0)
		{
			continue;
		}


		MessagePacket packet;

		packet.type = PacketType::PACKET_TYPE_CHATTING;
		packet.length = sizeof(MessagePacket);

		//명령어 체크
		if (strcmp(message, "/remsg") == 0)
		{
			strcpy_s(message, prevMessage.c_str());
		}

		if (strcmp(message, "/q") == 0)
		{
			closesocket(serverSocket);
			PushPrintText("Server : 서버와의 연결을 종료합니다.");
			break;
		}
		else if (strncmp(message, "/r ", 3) == 0)
		{
			char tok[BUFFER_SIZE];
			strcpy_s(tok, message);
			char* left = NULL;
			char* t = strtok_s(tok, " ", &left);
			char* whisperId = strtok_s(left, " ", &left);

			packet.isWhisper = true;
			strcpy_s(packet.whisperId, whisperId);
			strcpy_s(packet.message, left);
			PushPrintText(id + "(s) : " + packet.message);
		}
		else
		{
			strcpy_s(packet.message, message);
			PushPrintText(id + " : " + packet.message);
		}

		prevMessage = message;
		send(serverSocket, (char*)&packet, packet.length, 0);
	}

	_getch();
}
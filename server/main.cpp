#include <windows.h>
#include <stdio.h>
#include <string>
#include <map>

#pragma comment(lib,"Ws2_32.lib")

#define BUFFER_SIZE 4096

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

struct ClientInformation
{
	SOCKET socket;
	char id[32];
};

struct AccountInformation
{
	char id[32];
	char password[32];
};

void main()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return;

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket create error \n");
		return;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("bind error \n");
		return;
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("listen error \n");
	}

	//����� ���� �������� ��� �о �޸𸮷� �ø���.
	std::map<std::string, AccountInformation> accountInformation;
	{

		FILE* file = NULL;
		fopen_s(&file, "account.txt", "rb");

		if (file)
		{
			printf("����� ���� ����Ʈ\n");
			while (feof(file) == 0)
			{
				AccountInformation information;

				fscanf_s(file, "%s %s\n", information.id, 32, information.password, 32);

				printf("%s \n", information.id);

				accountInformation.insert(std::make_pair(information.id, information));
			}

			fclose(file);
		}
		else
		{
			printf("���� ���� ������ ���ų�, �б� ����\n");
		}
	}

	fd_set read;
	fd_set temp;
	TIMEVAL time;

	FD_ZERO(&read);
	FD_SET(serverSocket, &read);

	char buffer[BUFFER_SIZE] = { 0, };

	std::map<int, ClientInformation> connectedClientMap;

	printf("���� ����\n");
	while (true)
	{
		temp = read;
		time.tv_sec = 5;
		time.tv_usec = 0;

		if (select(0, &temp, 0, 0, &time) == SOCKET_ERROR)
		{
			printf("select error \n");
			return;
		}

		for (int index = 0; index < read.fd_count; ++index)
		{
			if (FD_ISSET(read.fd_array[index], &temp))
			{
				if (serverSocket == read.fd_array[index])
				{
					//���� ��û

					SOCKADDR_IN clientAddr;
					int length = sizeof(clientAddr);

					SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &length);
					FD_SET(clientSocket, &read);

					printf("���ο� Ŭ���̾�Ʈ ���� : %d \n", clientSocket);

				}
				else
				{
					int recvLength = recv(read.fd_array[index], buffer, BUFFER_SIZE - 1, 0);

					if (recvLength <= 0)
					{
						// ����
						auto itr = connectedClientMap.begin();
						auto itrEnd = connectedClientMap.end();

						for (; itr != itrEnd; ++itr)
						{
							if (itr->second.socket == read.fd_array[index])
							{
								printf("Ŭ���̾�Ʈ ���� ���� : %s[%d]\n", itr->second.id, itr->first);
								connectedClientMap.erase(itr);
								break;
							}
						}
						closesocket(temp.fd_array[index]);
						FD_CLR(read.fd_array[index], &read);
					}
					else
					{
						PacketBase* packetBase = (PacketBase*)buffer;

						if (packetBase->length == recvLength)
						{
							if (packetBase->type == PACKET_TYPE_SIGN)
							{
								SignPacket* packet = (SignPacket*)buffer;


								auto itr = accountInformation.find(packet->id);

								if (itr == accountInformation.end())
								{
									//������ ���� �����ؾ���
									AccountInformation information;
									strcpy_s(information.id, packet->id);
									strcpy_s(information.password, packet->password);

									accountInformation.insert(std::make_pair(information.id, information));

									//������ �� �����ϰ� ����.
									FILE* file = NULL;
									fopen_s(&file, "account.txt", "wb");

									if (file)
									{
										for (auto itr = accountInformation.begin(); itr != accountInformation.end(); ++itr)
										{
											fprintf_s(file, "%s %s\n", itr->second.id, itr->second.password);
										}

										printf("%d���� ���� ���� ����\n", accountInformation.size());

										fclose(file);
									}
								}
								else if (strcmp(itr->second.password, packet->password) != 0)
								{
									//��й�ȣ Ʋ��
									SignResPacket resPacket;

									resPacket.type = PacketType::PACKET_TYPE_SIGN_RES;
									resPacket.length = sizeof(resPacket);
									resPacket.responseType = SignResPacket::INVALID_ACCOUNT;

									send(read.fd_array[index], (char*)&resPacket, resPacket.length, 0);
									continue;
								}
								else
								{
									//�ߺ� �α��� üũ

									auto itr = connectedClientMap.begin();
									auto itrEnd = connectedClientMap.end();

									bool find = false;
									for (; itr != itrEnd; ++itr)
									{
										if (strcmp(itr->second.id, packet->id) == 0)
										{
											find = true;
											break;
										}
									}
									
									if (find)
									{
										SignResPacket resPacket;

										resPacket.type = PacketType::PACKET_TYPE_SIGN_RES;
										resPacket.length = sizeof(resPacket);
										resPacket.responseType = SignResPacket::DUPLICATE_CONNECT;

										send(read.fd_array[index], (char*)&resPacket, resPacket.length, 0);
										continue;
									}

								}
								//�α��� ����
								ClientInformation information;

								strcpy_s(information.id, packet->id);
								information.socket = read.fd_array[index];

								connectedClientMap.insert(std::make_pair(information.socket, information));

								SignResPacket resPacket;

								resPacket.type = PacketType::PACKET_TYPE_SIGN_RES;
								resPacket.length = sizeof(resPacket);
								resPacket.responseType = SignResPacket::SUCCESS;
								sprintf_s(resPacket.message, "%s�� ȯ���մϴ�.", packet->id);

								send(read.fd_array[index], (char*)&resPacket, resPacket.length, 0);
							}
							else if (packetBase->type == PACKET_TYPE_CHATTING)
							{
								auto itr = connectedClientMap.find(read.fd_array[index]);
								
								if (itr == connectedClientMap.end())
								{
									//�α������� ���� ������ ä�� ��Ŷ�� ������ ��
									continue;
								}

								MessagePacket* messagePacket = (MessagePacket*)buffer;

								MessagePacket message;
								message.type = PacketType::PACKET_TYPE_CHATTING;
								message.length = sizeof(MessagePacket);

								message.isWhisper = false;

								strcpy_s(message.sendId, itr->second.id);
								strcpy_s(message.message, messagePacket->message);

								if (messagePacket->isWhisper)
								{
									//�ӼӸ�
									message.isWhisper = true;
									
									auto itr = connectedClientMap.begin();
									auto itrEnd = connectedClientMap.end();

									bool find = false;
									for (; itr != itrEnd; ++itr)
									{
										if (strcmp(itr->second.id, messagePacket->whisperId) == 0)
										{
											send(itr->first, (char*)&message, message.length, 0);
											find = true;
											break;
										}
									}

									if (find == false)
									{
										//������ ������ ���� ��?

									}
								}
								else
								{
									//�ش� ������ ������ �ٸ� �����鿡�� ��� ������
									
									
									for (auto itr = connectedClientMap.begin(); itr != connectedClientMap.end(); ++itr)
									{
										if (itr->first != read.fd_array[index])
										{
											send(itr->first, (char*)&message, message.length, 0);
										}
									}
									continue;
								}

							}
						}
						else
						{
							//��Ŷ�� �� �޾ƿ� �ߺ�ó���� �ؾ���
						}
					}
				}
			}
		}
	}
}

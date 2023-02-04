// Client.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <vector>
#include <string.h>

//#include"stdafx.h"
#include <WINSOCK2.H>
#include <windows.h>

#pragma comment(lib,"WS2_32.lib")
using namespace std;


#define DES_IP "127.0.0.1"
#define DES_PORT 5188
//#define Getsomething(pt)	  ((pt)[0]<<24|(pt)[1]<16|(pt)[2]<<8|(pt)[3])

char* buf;

std::ofstream createfile(int num) {
	ofstream my_file("data/" + std::to_string(num) + ".csv");
	return my_file;
}

void receivedata(SOCKET sock, unsigned int s, int num) {
	std::vector<float> result;
	std::vector<int> res2;

	char recv_buf[10000];
	int size;

	std::ofstream my_file = createfile(num);

	printf("\nReceiving %u bytes of data\n", s);

	for(;;) {
		if((size = recv(sock, recv_buf, sizeof(recv_buf), 0)) > 0) {
			for(unsigned int i=0; i<size; i++) {
				result.push_back((recv_buf[i] - '0')/105.000);
			}

			for(unsigned int j=0; j<50000; j+=50) {
				res2.push_back(j);
			}

			int i,j;
			my_file << "Time, Amplitude (V)\n";

			for(i=0, j=0;i<result.size()-1, j<9999;++i, j+=10){
				cout<<(float)result[i]<<",";
				my_file << j << "," << (float)result[i] << "\n";
			}
			if(i==result.size()-1) {
				cout << (float)result[i];
				my_file << j << "," << (float)result[i] << "\n";
			}
			printf("\n");
			break;
		}
		else if(size==0) {
			cout << WSAGetLastError << endl;
			if(result.size() < s) {
				printf("premature close, expected %u, only got %u\n", s, result.size());
				break;
			}
		}
		else {
			perror("recv\n");
			exit(1);
		}
	}
}

int main()
{	
	//cout << "Opening OWON software\n\n";
	//ShellExecute(NULL, "open", "C:\\Program Files (x86)\\OWON\\VDS_S2\\launcher.exe", NULL, NULL, SW_SHOWDEFAULT);

	SOCKET sock;
	char inputbuf[] = "*ADC?CH1";//�����ַ��洢��
	SOCKADDR_IN ser_addr;
	WSADATA wsaData;
	WORD wVersionRequired;
	wVersionRequired = MAKEWORD(2,2);
	// step   始  
	if (WSAStartup(wVersionRequired,&wsaData)!=0)
	{
		cout<<WSAGetLastError()<<endl;
		//cout << "  始  失 埽 5   乇 \n";
		cout << "Failed to initialize. Closed after 5 seconds\n";
		Sleep(5000);
		return 1;
	}

	char timebase[] = ":TIMebase:SCAle 50us";
	char scale[] = ":CHANnel1:SCAle 5";
	//char offset[] = ":TIMebase:HOFFset +500";
	char mem[] = ":ACQuire:MDEPth 1K";

	send(sock, timebase, strlen(timebase), 0);
	send(sock, scale, strlen(scale), 0);
	//send(sock, offset, strlen(offset), 0);
	send(sock, mem, strlen(mem), 0);

	int c = 1;

	while(1) {
		do {
			sock = socket( AF_INET, SOCK_STREAM, 0 );

			ser_addr.sin_addr.S_un.S_addr = inet_addr(DES_IP);
			ser_addr.sin_port = htons(DES_PORT);
			ser_addr.sin_family = AF_INET;
			memset(ser_addr.sin_zero,0,8);

			if (connect(sock,(SOCKADDR *)&ser_addr,sizeof(SOCKADDR_IN))!=0)
			{
				int nError = WSAGetLastError();
				cout << "Connect the server failure\n";
				//continue;		
			}	

			int SentBytes = send(sock, inputbuf, strlen(inputbuf), 0);

			//SentBytes = send(sock, buf, strlen(buf), 0);

			if (SentBytes == SOCKET_ERROR||SentBytes==0)
			{
				//cout << "��������ʧ�ܣ�\n\n";
				cout << "Failure to send data!\n\n";
				//continue;
			}

			//step ����ָ��

			
			if(!strcmp(mem, ":ACQuire:MDEPth 1K")) {
				receivedata(sock, 1000, c);
			}
			if(!strcmp(mem, ":ACQuire:MDEPth 10K")) {
				receivedata(sock, 10000, c);
			}
			if(!strcmp(mem, ":ACQuire:MDEPth 100K")) {
				receivedata(sock, 100000, c);
			}
			if(!strcmp(mem, ":ACQuire:MDEPth 1M")) {
				receivedata(sock, 1000000, c);
			}
			if(!strcmp(mem, ":ACQuire:MDEPth 5M")) {
				receivedata(sock, 5000000, c);
			}


			Sleep(500);

			c++;

		} while (c>1);
	}
    
	closesocket(sock);
	WSACleanup();
	return 0;

}
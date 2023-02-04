// Client.cpp : Defines the entry point for the console application.
//

//#include"stdafx.h"
#include <iostream>
#include <WINSOCK2.H>
#include <cstring>
#pragma comment(lib,"WS2_32.lib")
using namespace std;


#define DES_IP "127.0.0.1"
#define DES_PORT 5188
#define GetU32(pt)	  ((pt)[0]<<24|(pt)[1]<<16|(pt)[2]<<8|(pt)[3])

char* buf;

void prepareSendBuf(char inputbuf[]){

 		if(strcmp(buf,"0") ==0 )
			buf="*IDN?"; 
		else if(strcmp(buf,"1") ==0 )
			buf="*ADC?CH1";
		else if(strcmp(buf,"2") ==0 )
			buf="*RDM?";
		else if(strcmp(buf,"3") ==0 )
			buf="*RUNStop";
		else if(strcmp(buf,"4") ==0 )
			buf=":CHANnel2:DISPlay OFF;:CHANnel2:DISPlay?";
		else if(strcmp(buf,"5") ==0 )
			buf=":MEASure:SOURce CH1;:MEASure:ADD PERiod;:MEASure:ADD MAX";
		else if(strcmp(buf,"6") ==0 )
			buf=":MEASure1:MAX?";
		else if(strcmp(buf,"7") ==0 )
			buf=":TIMebase:HOFFset?";
		else if(strcmp(buf,"8") ==0 )
			buf=":TIMebase:HOFFset 150"; 
		else if(strcmp(buf,"9") ==0 )
			buf=":ACQuire:MDEPth?";
		else if(strcmp(buf,"10") ==0 )
			buf=":ACQuire:MDEPth 10K";
		else if(strcmp(buf,"11") ==0 )
			buf=":TRIGger:TYPE?";
		else if(strcmp(buf,"12") ==0 )
			buf=":TRIGger:TYPE ALTernate";
		else if(strcmp(buf,"13") ==0 )
			buf=":TRIGger:MODE?"; 
		else if(strcmp(buf,"14") ==0 )
			buf=":TRIGger:MODE NORMal";
		else if(strcmp(buf,"15") ==0 )
			buf=":TRIGger:SINGle:EDGE:LEVel?";
		else if(strcmp(buf,"16") ==0 )
			buf=":TRIGger:SINGle:EDGE:LEVel 25";
		else if(strcmp(buf,"17") ==0 )
			buf=":TRIGger:ALT:EDGE:LEVel?";
		else if(strcmp(buf,"18") ==0 )
			buf=":TRIGger:ALT:EDGE:LEVel 50";
 		else
 			buf=inputbuf;
}

 void dealFeedbackString(SOCKET sock){
			int RecvBytes;
			char strRecv[1024] = {0};
			if ((RecvBytes = recv(sock,strRecv,sizeof(strRecv),0)) != SOCKET_ERROR)
			{
				strRecv[RecvBytes] = 0;
				//cout << "接收 " << RecvBytes<<"字节:  ";
				cout << "receive " << RecvBytes<<"bytes:  ";
				cout<<strRecv<<endl;
			}
			else
			{
			    cout << WSAGetLastError() <<endl;
				//cout << "接收数据失败\n";
				cout << "Receive data failure\n";
			}
}

int dealNormalADC(SOCKET sock)
{			
			int RecvBytes;
			char lenRecv[4]={0};

			if ((RecvBytes = recv(sock,lenRecv,sizeof(lenRecv),0)) != SOCKET_ERROR)
			{
				unsigned char* uRecv=(unsigned char*)lenRecv;
				int adcLen=GetU32(uRecv);//接收4个字节,组合成波形长度
				//cout<<"解析大小:"<<adcLen;
				if(adcLen<0)
				{
					//cout<<"错误！没有接收到数据"<<endl;
					cout<<"Error! Not receive any data"<<endl;
					return 1;
				}
				Sleep(50);
				adcLen=sizeof(char)*adcLen;
				//cout<<" , 分配大小："<<adcLen <<endl;
				char* adcRecv=(char *) malloc(adcLen);
				if ((RecvBytes = recv(sock,adcRecv,adcLen,0)) != SOCKET_ERROR)
				{
					//cout<<"接收"<<RecvBytes <<"字节的ADC: "<<endl;
					cout<<"Receive"<<RecvBytes <<"bytes ADC: "<<endl;
					//adcRecv[RecvBytes] = 0;
					//uRecv=(unsigned char*)adcRecv;
					for(int i=0;i<adcLen;++i){
						cout<<(int)adcRecv[i]<<",";
					}
					if(RecvBytes==adcLen)
						cout<<"over"<<endl;
					cout<<endl;
				}else
				{
					cout << WSAGetLastError() <<endl;
					//cout << "接收ADC数据失败！\n";
					cout << "Receive ADC data failed!\n";
				}
				free(adcRecv);

			}else
			{
			    cout << WSAGetLastError() <<endl;
				//cout << "adc数据长度异常！\n";
				cout << "ADC  data length is abnormal!\n";
			}	
	return 0;
}
int dealRemoteDeepMemoryADC(SOCKET sock)
{
			int RecvBytes;
			char lenRecv[4]={0};
			dealFeedbackString(sock);

			if ((RecvBytes = recv(sock,lenRecv,sizeof(lenRecv),0)) != SOCKET_ERROR)
			{
				unsigned char* uRecv=(unsigned char*)lenRecv;
				int adcLen=GetU32(uRecv);//接收4个字节,组合成波形长度
				//cout<<"解析大小:"<<adcLen<<endl;
				if(adcLen<0)
				{
					//cout<<"错误！没有接收到DM数据"<<endl;
					cout<<"Error! Did not receive the DM data"<<endl;
					dealFeedbackString(sock);
					return 1;//break
				}
				Sleep(50);
				adcLen=sizeof(char)*adcLen;
				//cout<<"DM总长:"<< adcLen <<endl;
				
				const char * filename="dm.bin";//
				FILE * ft= fopen(filename, "wb");//


				int p=0 , q=0 , bl=2048 , failTime=0;
				while (p < adcLen && failTime<100) 
				{
					if (bl > adcLen - p)
						bl = adcLen - p;

					char* adcRecv=(char *) malloc(bl);
					if ((RecvBytes = recv(sock,adcRecv,bl,0)) != SOCKET_ERROR)
					{
						//cout<<"本次接收"<<RecvBytes <<"字节,总计:"<<p+RecvBytes<<endl;
						cout<<"The receiving"<<RecvBytes <<"bytes,total:"<<p+RecvBytes<<endl;
						//handle adcRecv buffer.
						if(ft){
							//fseek(ft,p,SEEK_SET);
							int writeBytes=	fwrite(adcRecv,sizeof(char),bl,ft);//strlen(adcRecv)
							q += writeBytes;
							//cout<<"写入"<<writeBytes<<"字节,总计:"<<q<<endl;
							cout<<"write "<<writeBytes<<"bytes,total::"<<q<<endl;
						}else
							//cout<<"写入失败"<<endl;						
							cout<<"Write failed"<<endl;


						p += RecvBytes;
																	

						if(p==adcLen)
							cout<<"over"<<endl;
							
					}else
					{
						failTime++;
						cout << WSAGetLastError() <<endl;
						cout << "Receive the ADC data failed!\n";
					}					
					free(adcRecv);

				}
				fclose( ft ) ;

			}else
			{
			    cout << WSAGetLastError() <<endl;
				//cout << "adc数据长度异常！\n";
				cout << "ADC  data length is abnormal!\n";
			}
			
			dealFeedbackString(sock);
			//cout << "Saved @ 本程序Client.exe同级或上一级目录内,名字是dm.bin\n";
			cout << "Saved @  the same level as Client.exe or at the higher level directory,name is dm.bin\n";
	return 0;
}



int main()
{
	SOCKET sock;
	char inputbuf[1024];//输入字符存储区
	SOCKADDR_IN ser_addr;
	WSADATA wsaData;
	WORD wVersionRequired;
	wVersionRequired = MAKEWORD(2,2);
	// step 初始化
	if (WSAStartup(wVersionRequired,&wsaData)!=0)
	{
		cout<<WSAGetLastError()<<endl;
		//cout << "初始化失败！5秒后关闭\n";
		cout << "Failed to initialize. Closed after 5 seconds\n";
		Sleep(5000);
		return 1;
	}
	//cout << "初始化成功，输入\"exit\"断开连接！输入\"help\"显示简单指令表\n";
	cout << "Initialization is successful, input\"exit\" to disconnect! \nInput \"help\"to show simple order list\n";


		
	
	do
	{
		//if (sock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_IP,NULL,0,1))
		// step 创建套接字两种方式
		sock = socket( AF_INET, SOCK_STREAM, 0 );
		// if (sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)==INVALID_SOCKET)
		// {
		// 		cout << "创建套接字失败！\n\n";
		// 		return 1;
		// }

		ser_addr.sin_addr.S_un.S_addr = inet_addr(DES_IP);
		ser_addr.sin_port = htons(DES_PORT);
		ser_addr.sin_family = AF_INET;
		memset(ser_addr.sin_zero,0,8);
		//step 连接
		if (connect(sock,(SOCKADDR *)&ser_addr,sizeof(SOCKADDR_IN))!=0)
		{
			int nError = WSAGetLastError();
			cout << "Connect the server failure\n";
			//continue;		
		}	


		//step 发送指令

		int SentBytes;
		//int RecvBytes;
		//char strRecv[1024] = {0};
		//char lenRecv[4]={0};
		
		prepareSendBuf(":TIMebase:SCAle 50ns");
		
		cout << "Send> ";
 		cin.getline(inputbuf,sizeof(inputbuf));
		buf=inputbuf;
		prepareSendBuf(inputbuf);
		if(strcmp(buf,"exit") ==0 || strcmp(buf,"EXIT") ==0 ) break;
		if(strcmp(buf,"help") ==0 || strcmp(buf,"HELP") ==0 ) { coutOrderList(); continue;}

		SentBytes = send( sock,buf, strlen(buf), 0 );
		if (SentBytes == SOCKET_ERROR||SentBytes==0)
		{
			//cout << "发送数据失败！\n\n";
			cout << "Failure to send data!\n\n";
			continue;
		}

		
		if(memcmp(buf,"*ADC?",5) ==0 || memcmp(buf,"*adc?",5) ==0 )
		{	
			//step 进入NormalADC模式
			int rsp=dealNormalADC(sock);
			if(rsp==1)
				continue;// break
		}		
		else if(memcmp(buf,"*LDM?",5) ==0 || memcmp(buf,"*ldm?",5) ==0 )
		{
			//step 完整深数据接收，存放在本地服务器
			dealLocalDeepMemoryADC(sock);
		}
		else if(memcmp(buf,"*RDM?",5) ==0 || memcmp(buf,"*rdm?",5) ==0 )
		{
			//step 完整深数据接收，存放从服务器远程传输到客户端(即Client.exe同级或上一级目录)
			int rsp=dealRemoteDeepMemoryADC(sock);
			if(rsp==1)
				continue;// break
		}else{
			//step 进入接收反馈的模式;
			dealFeedbackString(sock);
		}

		Sleep(500);

	} while (1);
    closesocket(sock);
	WSACleanup();
	return 0;
}

	




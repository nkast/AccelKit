/*
    'accelKit'
    Copyright (C) 2010  Nikos Kastellanos

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
    e-mail: nkastellanos@gmail.com
*/

#include "WServer.h"
#include <Windows.h>
#include <string.h>


//A SOCKET is simply a typedef for an unsigned int.
SOCKET server;
//WSADATA is a struct that is filled up by the call to WSAStartup
WSADATA wsaData;
//The sockaddr_in specifies the address of the socket
SOCKADDR_IN local;
char lszThreadParam[3];

double wsoutx,wsouty,wsoutz;

void WServerStart(int port)
{
	  HANDLE hThread;
	  
      DWORD dwGenericThread;

	wsoutx=0,wsouty=-1,wsoutz=0;

      lszThreadParam[0] = port;
      hThread = CreateThread(NULL,0,WServerThread,&lszThreadParam,0,&dwGenericThread);

	 if(hThread == NULL)
      {
            DWORD dwError = GetLastError();
            //cout<<"SCM:Error in Creating thread"<<dwError<<endl ;
            return;
       }
	return;
}


DWORD WINAPI WServerInit(int port)
{
    //WSAStartup initializes the program for calling WinSock.
    //The first parameter specifies the highest version of the 
    //WinSock specification, the program is allowed to use.
    int wsaret=WSAStartup(0x101,&wsaData);

    //WSAStartup returns zero on success.
    //If it fails we exit.
    if(wsaret!=0)
    {
        return 0;
    }
	
	//Now we populate the sockaddr_in structure
    local.sin_family=AF_INET; //Address family
    local.sin_addr.s_addr=INADDR_ANY; //Wild card IP address
    local.sin_port=htons((u_short)port); //port to use

    //the socket function creates our SOCKET
    server=socket(AF_INET,SOCK_STREAM,0);

    //If the socket() function fails we exit
    if(server==INVALID_SOCKET)
    {
        return 0;
    }

    //bind links the socket we just created with the sockaddr_in 
    //structure. Basically it connects the socket with 
    //the local address and a specified port.
    //If it returns non-zero quit, as this indicates error
	if(bind(server,(SOCKADDR*)&local,sizeof(local))!=0)
    {
        return 0;
    }

    //listen instructs the socket to listen for incoming 
    //connections from clients. The second arg is the backlog
    if(listen(server,10)!=0)
    {
        return 0;
    }

	return -1;
}

DWORD WINAPI WServerListen()
{
	//we will need variables to hold the client socket.
    //thus we declare them here.
	SOCKET client;	
    SOCKADDR_IN from;
    int fromlen=sizeof(from);

	char temp[1024*4];
	int rcvcount=0;
	char *res;

    //accept() will accept an incoming
    //client connection
    client=accept(server, (struct sockaddr*)&from,&fromlen);

	// consume the request header
	while(TRUE)
	{
		rcvcount=recv(client,temp,1024*4,0);
		//if(rcvcount==0) continue;
		//res=strstr(temp,"\r\n\r\n");
		break;
	}

	//we simply send this string to the client
    //sprintf(temp,"Your IP is %s\r\n",inet_ntoa(from.sin_addr));    
    //send(client,temp,strlen(temp),0);

	//send dummy http header
	sprintf(temp,"HTTP/1.0 200 OK\r\n");
    send(client,temp,strlen(temp),0);
	sprintf(temp,"Server: HTTPaccel/1.0\r\n");
    send(client,temp,strlen(temp),0);
	sprintf(temp,"Content-Type: application/octet-stream\r\n");
    send(client,temp,strlen(temp),0);
	sprintf(temp,"Expires:  Thu, 01 Dec 1994 10:00:00 GMT\r\n");
    send(client,temp,strlen(temp),0);
	sprintf(temp,"Pragma: no-cache\r\n");
    send(client,temp,strlen(temp),0);
	sprintf(temp,"\r\n"); //end
    send(client,temp,strlen(temp),0);	
	//Date: Sat, 27 Nov 2004 10:19:07 GMT
	//Content-Length: 10476
	
	
	sprintf(temp,"%f,%f,%f",wsoutx,wsouty,wsoutz); //end
    send(client,temp,strlen(temp),0);
	

    //cout << "Connection from " << inet_ntoa(from.sin_addr) <<"\r\n";
	
    //close the client socket
    closesocket(client);

	return TRUE;
}


DWORD WINAPI WServerCleanup()
{
	//closesocket() closes the socket and releases the socket descriptor
    closesocket(server);

    //originally this function probably had some use
    //currently this is just for backward compatibility
    //but it is safer to call it as I still believe some
    //implementations use this to terminate use of WS2_32.DLL 
    WSACleanup();
}


DWORD WINAPI WServerThread(LPVOID iValue)
{
	int port = ((char*)iValue)[0];

	if(!WServerInit(port)) return FALSE;

	while(TRUE)	//we are looping endlessly
    {
		WServerListen();
    }

	WServerCleanup();
    
    return 0;
}



/*
   WinAFL - A simple binary to test winAFL ability perform fuzzing over network:
   -------------------------------------------------------------
   Written and maintained by Maksim Shudrak <mxmssh@gmail.com>
   Copyright 2018-2019 Salesforce Inc. All Rights Reserved.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */
#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#endif
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#endif

#define DEFAULT_PORT 7714
#define BUFSIZE 4096

/* TODO: test for TCP */

void error(const char *msg) {
#ifdef WIN32
    printf("[ERROR] %s %d\n", msg, WSAGetLastError());
#else
    printf("[ERROR] %s\n", msg);
#endif
    exit(-1);
}


struct sockaddr_in serveraddr;	  /* server's addr */

void recv_func(int sockfd)
{	
    char *buf;
    struct sockaddr_in clientaddr;	  /* client addr */
    int clientlen = sizeof(clientaddr);
    int n = 0;

    buf = (char *)malloc(BUFSIZE);

    /* receiving over UDP */
    n = read(sockfd, buf, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
    if (n < 0)
        error("ERROR in recvfrom");

    if (buf[0] == 'P') {
        if (buf[1] == 'W') {
            if (buf[2] == 'N') {
                if (buf[3] == 'I') {
                    if (buf[4] == 'T') {
                        printf("Found it!\n");
                        ((void(*)())0x0)();
                    }
                }
            }
        }
    }

    printf("Received %d bytes, content = %s\n", n, buf);
    free(buf);
}

int main(int argc, char** argv)
{
    int sockfd;
    int portno = DEFAULT_PORT;
    int optval;

#ifdef WIN32
    static WSADATA wsaData;
    static int iResult;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    optval = 1;
    /* UDP */
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(int));

    memset((char *)&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    while (1)
        recv_func(sockfd);
    return 0;
}

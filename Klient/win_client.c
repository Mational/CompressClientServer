#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    WORD WRequiredVersion;
    WSDATA WData;
    SOCKET SSocket;
    int nConnect;
    int nBytes;
    struct sockaddr_in stServerAddr;
    struct hostent* lpstServerEnt;
    char chBuf[BUF_SIZE];

    WRequiredVersion = MAKEWORD(2,0);
    WSAStartup(WRequiredVersion, &WData);
    gethostbyname(argv[1]);
    SSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    stServerAddr.sin_family = AF_INET;
    memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);
    stServerAddr.sin_port = htons(atoi(argv[2]));
    connect(SSocket, (struct sockaddr*)&stServerAddr, sizeof(struct sockaddr));
    nBytes = recv(SSocket, chBuf, sizeof(chBuf), 0);
    chBuf[nBytes] = '\x0';
    printf("Data from SERVER [%s]:\t%s", argv[1], chBuf);
    closesocket(SSocket);
    WSACleanup();
    return 0;
}
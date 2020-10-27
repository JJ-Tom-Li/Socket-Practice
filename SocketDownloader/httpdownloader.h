//
//  httpdownloader.h
//  HttpSocketTest
//
//  Created by user on 2020/10/21.
//

#ifndef httpdownloader_h
#define httpdownloader_h
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
// HTTP_RESPONSE_BUFFER_SIZE小於10 recv()會出錯
#define HTTP_RESPONSE_BUFFER_SIZE 0xfff
#define HTTP_REQUEST_SIZE 0xfff
#define HTTP_HEADER_SIZE 0xffff
#define SUCCESS 0
#define FAIL -1

typedef struct _httpDownloadInfo{
    int sockfd;
    int headerRequestSize;
    int bodyRequestSize;
    char *url;
    char *outputFileName;
    char *hostName;
    char *filePath;
    char portNum[50];
    char IPaddress[INET_ADDRSTRLEN];
    char headerRequest[HTTP_REQUEST_SIZE];
    char bodyRequest[HTTP_REQUEST_SIZE];
    char HTTPheader[HTTP_HEADER_SIZE];
    FILE *outputFile;
    ssize_t headerSize;
    ssize_t fileSize;
    ssize_t downloadedFileSize;
    struct sockaddr_in socketInfo;
}HttpDownloadInfo;

int HTTPDownloader(char*, char*);
int HttpDownloadInfoInit(HttpDownloadInfo*, char*, char*);
void DeleteHttpFromUrl(HttpDownloadInfo*);
void GetHostName(HttpDownloadInfo*);
void GetFilePath(HttpDownloadInfo*);
void GetPortNum(HttpDownloadInfo*);
void FormRequests(HttpDownloadInfo*);
int GetHostIP(HttpDownloadInfo*);
int ConnectSocket(HttpDownloadInfo*);
int GetHeader(HttpDownloadInfo*);
ssize_t GetHeaderSize(HttpDownloadInfo*);
ssize_t GetFileSize(HttpDownloadInfo*);
int DeleteHeaderFromGet(HttpDownloadInfo*);
int DownloadFile(HttpDownloadInfo*);
#include "httpdownloader_f.h"

#endif /* httpdownloader_h */

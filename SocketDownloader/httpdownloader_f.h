//
//  httpdownloader_f.h
//  SocketDownloader
//
//  Created by user on 2020/10/27.
//

#ifndef httpdownloader_f_h
#define httpdownloader_f_h

int HTTPDownloader(char *url, char *outputFileName){
    HttpDownloadInfo *httpDownloadInfo = malloc(sizeof(HttpDownloadInfo));
    if (HttpDownloadInfoInit(httpDownloadInfo, url, outputFileName) != SUCCESS) {
        printf("初始化時發生錯誤.\n");
        return FAIL;
    }
    
    GetHostName(httpDownloadInfo);
    GetFilePath(httpDownloadInfo);
    GetPortNum(httpDownloadInfo);
    printf("port:%s, host:%s, file:%s\n",httpDownloadInfo->portNum, httpDownloadInfo->hostName, httpDownloadInfo->filePath);
    FormRequests(httpDownloadInfo);
    httpDownloadInfo->headerRequestSize = (int) strlen(httpDownloadInfo->headerRequest);
    httpDownloadInfo->bodyRequestSize = (int) strlen(httpDownloadInfo->bodyRequest);
    
    if (GetHostIP(httpDownloadInfo) != SUCCESS) {
        printf("取得host IP時發生問題.\n");
        return FAIL;
    }
    printf("HostName:%s,IP:%s\n", httpDownloadInfo->hostName, httpDownloadInfo->IPaddress);
    
    // 設定socket連線.
    if (ConnectSocket(httpDownloadInfo) == FAIL) {
        printf("Socket connect 失敗.\n");
        return FAIL;
    }
    printf("Socket connect 成功.\n");
    
    if (GetHeader(httpDownloadInfo) != SUCCESS) {
        printf("取得 http header 時發生問題.\n");
        return FAIL;
    }
    GetHeaderSize(httpDownloadInfo);
    GetFileSize(httpDownloadInfo);
    if (httpDownloadInfo->fileSize == 0) {
        printf("取得檔案大小時發生問題.\n");
        return FAIL;
    }
    
    // GET request前段會包含HTTP header，需要將其刪掉
    printf("將 HTTP GET request 中的 header刪除...");
    if (DeleteHeaderFromGet(httpDownloadInfo) != SUCCESS) {
        printf("在刪除 header 時發生問題.\n");
        return FAIL;
    }
    printf("成功!\n");

    httpDownloadInfo->outputFile = fopen(outputFileName, "wb");
    if (httpDownloadInfo->outputFile == NULL) {
        printf("%s檔案建立失敗.\n",outputFileName);
        return FAIL;
    }
    printf("下載中...");
    if (DownloadFile(httpDownloadInfo) == SUCCESS) {
        printf("成功!\n");
    }
    else{
        printf("失敗.\n");
        return FAIL;
    }
    
    close(httpDownloadInfo->sockfd);
    fclose(httpDownloadInfo->outputFile);
    free(httpDownloadInfo->url);
    free(httpDownloadInfo->outputFileName);
    free(httpDownloadInfo->hostName);
    free(httpDownloadInfo->filePath);
    free(httpDownloadInfo);
    return SUCCESS;
}

int HttpDownloadInfoInit(HttpDownloadInfo* httpDownloadInfo, char *url, char *outputFileName){
    httpDownloadInfo->sockfd = 0;
    httpDownloadInfo->headerRequestSize = 0;
    httpDownloadInfo->bodyRequestSize = 0;
    httpDownloadInfo->url = malloc(strlen(url) + 1);
    httpDownloadInfo->outputFileName = malloc(strlen(outputFileName) + 1);
    httpDownloadInfo->hostName = malloc(strlen(url) + 1);
    httpDownloadInfo->filePath = malloc(strlen(url) + 1);
    if (httpDownloadInfo->url == NULL
        || httpDownloadInfo->outputFileName == NULL
        || httpDownloadInfo->hostName == NULL
        || httpDownloadInfo->filePath == NULL){
        printf("malloc error.\n");
        return FAIL;
    }
    strcpy(httpDownloadInfo->url, url);
    strcpy(httpDownloadInfo->outputFileName, outputFileName);
    strcpy(httpDownloadInfo->hostName, "");
    strcpy(httpDownloadInfo->filePath, "");
    strcpy(httpDownloadInfo->portNum, "");
    httpDownloadInfo->headerSize = 0;
    httpDownloadInfo->fileSize = 0;
    httpDownloadInfo->downloadedFileSize = 0;
    
    return SUCCESS;
}

void DeleteHttpFromUrl(HttpDownloadInfo *httpDownloadInfo){
    if (strstr(httpDownloadInfo->url, "http://") != NULL) {
        char *tmp = malloc(strlen(httpDownloadInfo->url) + 1);
        strcpy(tmp, httpDownloadInfo->url + strlen("http://"));
        strcpy(httpDownloadInfo->url, tmp);
        free(tmp);
    }
}

void GetHostName(HttpDownloadInfo *httpDownloadInfo){
    int i;
    DeleteHttpFromUrl(httpDownloadInfo);
    for (i = 0; i < strlen(httpDownloadInfo->url); i++) {
        if (httpDownloadInfo->url[i] == ':' || httpDownloadInfo->url[i] == '/') {
            break;
        }
        httpDownloadInfo->hostName[i] = httpDownloadInfo->url[i];
    }
    httpDownloadInfo->hostName[i] = '\0';
}

void GetFilePath(HttpDownloadInfo *httpDownloadInfo){
    DeleteHttpFromUrl(httpDownloadInfo);
    char *filePathStart = strchr(httpDownloadInfo->url, '/');
    if (filePathStart == NULL) {
        strcpy(httpDownloadInfo->filePath, "/");
    }
    else{
        strcpy(httpDownloadInfo->filePath, filePathStart);
    }
}

void GetPortNum(HttpDownloadInfo *httpDownloadInfo){
    int i;
    DeleteHttpFromUrl(httpDownloadInfo);
    char *portNumStart = strchr(httpDownloadInfo->url, ':');
    if (portNumStart == NULL) {
        // 預設http port = 80
        strcpy(httpDownloadInfo->portNum, "80");
    }
    else{
        for (i = 1; (i < strlen(portNumStart)) && portNumStart[i] != '/'; i++) {
            httpDownloadInfo->portNum[i-1] = portNumStart[i];
        }
        httpDownloadInfo->portNum[i] = '\0';
    }
}

void FormRequests(HttpDownloadInfo *httpDownloadInfo){
    char *headerRequestFmt = "HEAD %s HTTP/1.1\r\n";
    char *bodyRequestFmt = "GET %s HTTP/1.1\r\n";
    char *hostFmt = "Host: %s\r\n";
    char *CRLF = "\r\n";
    size_t bufferLen = strlen(httpDownloadInfo->hostName) + 8;
    char *host = (char*)malloc(bufferLen);
    
    // 組成 HTTP HEAD request字串.
    sprintf(httpDownloadInfo->headerRequest, headerRequestFmt, httpDownloadInfo->filePath);
    sprintf(host, hostFmt, httpDownloadInfo->hostName);
    strcat(httpDownloadInfo->headerRequest, host);
    strcat(httpDownloadInfo->headerRequest, CRLF);
    
    // 組成 HTTP GET request字串.
    sprintf(httpDownloadInfo->bodyRequest, bodyRequestFmt, httpDownloadInfo->filePath);
    strcat(httpDownloadInfo->bodyRequest, host);
    strcat(httpDownloadInfo->bodyRequest, CRLF);
    free(host);
}

// 得到的IP字串會儲存在IPaddress中
int GetHostIP(HttpDownloadInfo *httpDownloadInfo){
    struct addrinfo hints;
    struct addrinfo *result;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    
    if ( getaddrinfo(httpDownloadInfo->hostName, httpDownloadInfo->portNum, &hints, &result) != SUCCESS){
        printf("getaddrinfo error.\n");
        return FAIL;
    }
    inet_ntop(AF_INET, &(((struct sockaddr_in *)result->ai_addr)->sin_addr), httpDownloadInfo->IPaddress, INET_ADDRSTRLEN);
    
    free(result);
    return SUCCESS;
}

int ConnectSocket(HttpDownloadInfo *httpDownloadInfo){
    bzero(&(httpDownloadInfo->socketInfo), sizeof(httpDownloadInfo->socketInfo));
    httpDownloadInfo->socketInfo.sin_family = PF_INET;
    httpDownloadInfo->socketInfo.sin_addr.s_addr = inet_addr(httpDownloadInfo->IPaddress);
    httpDownloadInfo->socketInfo.sin_port = htons(atoi(httpDownloadInfo->portNum));

    httpDownloadInfo->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(httpDownloadInfo->sockfd, (struct sockaddr *)&(httpDownloadInfo->socketInfo), sizeof(httpDownloadInfo->socketInfo)) != SUCCESS) {
        return FAIL;
    }
    return SUCCESS;
}

int GetHeader(HttpDownloadInfo* httpDownloadInfo){
    ssize_t responseSize = 0;
    char response[HTTP_RESPONSE_BUFFER_SIZE];
    
    if (send(httpDownloadInfo->sockfd, httpDownloadInfo->headerRequest, strlen(httpDownloadInfo->headerRequest), 0) != httpDownloadInfo->headerRequestSize) {
        printf("發送request失敗.\n");
        return FAIL;
    }
    httpDownloadInfo->HTTPheader[0] = '\0';
    
    //printf("Head response:\n");
    while (1) {
        responseSize = recv(httpDownloadInfo->sockfd, response, HTTP_RESPONSE_BUFFER_SIZE, 0);
        if (responseSize == 0) {
            return SUCCESS;
        }
        if (responseSize < 0) {
            printf("接收response時發生問題.\n");
            return FAIL;
        }
        
        // recv() 不會在response結尾加上\0，須自行加上
        response[responseSize] = '\0';
        
        strcat(httpDownloadInfo->HTTPheader, response);
        // HTTP header會以 \r\n\r\n 作為結束.
        if (strstr(httpDownloadInfo->HTTPheader, "\r\n\r\n") != NULL) {
            return SUCCESS;
        }
    }
    return SUCCESS;
}

ssize_t GetHeaderSize(HttpDownloadInfo* httpDownloadInfo){
    ssize_t size = (ssize_t)strlen(httpDownloadInfo->HTTPheader);
    httpDownloadInfo->headerSize = size;
    return size;
}

// 使用HTTP header中的Content-Length資訊來取得檔案大小．
ssize_t GetFileSize(HttpDownloadInfo* httpDownloadInfo){
    ssize_t size = 0;
    char ssize[100];
    int numberLength = 0;
    
    if (strstr(httpDownloadInfo->HTTPheader, "404 Not Found") != NULL) {
        printf("404 Not Found. 檔案不存在.\n");
        return FAIL;
    }
    
    if (strstr(httpDownloadInfo->HTTPheader, "400 Bad Request") != NULL) {
        printf("400 Bad Request. 網址有誤.\n");
        return FAIL;
    }
    
    char *sContentLength = strstr(httpDownloadInfo->HTTPheader, "Content-Length: ");
    if (sContentLength == NULL) {
        printf("header中不存在Content-Length資訊. 可能是檔案不存在或header有誤.\n");
        return FAIL;
    }
    
    char *endOfContentLength = strstr(sContentLength, "\r\n");
    numberLength = (int)(endOfContentLength - sContentLength -  strlen("Content-Length: "));
    
    // 取得 "Content-Length: "後面的數字並轉換成ssize_t型態儲存．
    strncpy(ssize, sContentLength + strlen("Content-Length: "), numberLength);
    size = (ssize_t) atoi(ssize);
    httpDownloadInfo->fileSize = size;
    return size;
}

// 將HTTP GET request中的header部分去除
int DeleteHeaderFromGet(HttpDownloadInfo* httpDownloadInfo){
    ssize_t responseSize = 0;
    ssize_t headerSize = httpDownloadInfo->headerSize;
    
    if (send(httpDownloadInfo->sockfd, httpDownloadInfo->bodyRequest, strlen(httpDownloadInfo->bodyRequest), 0) != httpDownloadInfo->bodyRequestSize) {
        printf("發送request失敗.\n");
        return FAIL;
    }
    
    while (headerSize != 0) {
        char response[headerSize];
        responseSize = recv(httpDownloadInfo->sockfd, response, headerSize, 0);
        switch (responseSize) {
            case SUCCESS:
                printf("End\n");
                return SUCCESS;
                break;
            case FAIL:
                printf("接收response時發生問題.\n");
                return FAIL;
                break;
            default:
                headerSize -= responseSize;
                break;
        }
    }
    return SUCCESS;
}

int DownloadFile(HttpDownloadInfo* httpDownloadInfo){
    ssize_t responseSize = 0;
    ssize_t downloadedFileSize = 0;
    char response[HTTP_RESPONSE_BUFFER_SIZE];
    
    while (downloadedFileSize < httpDownloadInfo->fileSize) {
        responseSize = recv(httpDownloadInfo->sockfd, response, HTTP_RESPONSE_BUFFER_SIZE, 0);
        switch(responseSize){
            case SUCCESS:
                return SUCCESS;
                break;
            case FAIL:
                printf("接收response時發生問題.\n");
                return FAIL;
                break;
            default:
                downloadedFileSize += responseSize;
                response[responseSize] = '\0';
                printf("\n[ResponseSize:%zd]-----\n",responseSize);
                printf("[完成度 :(%zd/%zd)]-----\n",downloadedFileSize,httpDownloadInfo->fileSize);
                fwrite(response, sizeof(char), responseSize, httpDownloadInfo->outputFile);
                break;
        }
    }
    return SUCCESS;
}
#endif /* httpdownloader_f_h */

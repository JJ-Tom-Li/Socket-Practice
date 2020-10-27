//
//  downloaders.h
//  HttpSocketTest
//
//  Created by user on 2020/10/21.
//

#ifndef downloaders_h
#define downloaders_h
#include "httpdownloader.h"
#define NUMBER_OF_PROTOCOLS 4

typedef struct _protocol{
    char name[50];
    int (*Downloader)(char *url, char *outputFileName);
} Protocol;

Protocol protocols[NUMBER_OF_PROTOCOLS] = {
    {"http://", HTTPDownloader},
    {"https://", NULL},
    {"ftp://", NULL},
    {"telnet://", NULL}
};

int Download(char*, char*, char*);
int GetProtocol(char*);
int DownloadByProtocol(int, char*, char*, char*);
void MergeFolderAndFileName(char*, char*, char*);

int Download(char *url, char* outputFileName, char *downloadFolder){
    int p = GetProtocol(url);
    
    return DownloadByProtocol(p, url, outputFileName, downloadFolder);
}

int GetProtocol(char *url){
    int i;
    
    for (i = 0; i < NUMBER_OF_PROTOCOLS; i++) {
        if (strstr(url, protocols[i].name) != NULL) {
            printf("Protocol:%s\n",protocols[i].name);
            return i;
        }
    }
    return FAIL;
}

int DownloadByProtocol(int p, char *url, char *_outputFileName, char *downloadFolder){
    
    if (p == FAIL) {
        printf("未知的通訊協定.\n");
        return FAIL;
    }
    
    char outputFileName[1500] = "";
    MergeFolderAndFileName(outputFileName, downloadFolder, _outputFileName);
    
    if (protocols[p].Downloader == NULL) {
        printf("%s downloader 尚未完成.\n", protocols[p].name);
        return FAIL;
    }
    else if (protocols[p].Downloader(url, outputFileName) == SUCCESS) {
        printf("%s 下載成功! 檔案資料夾: %s  \n", protocols[p].name,  downloadFolder);
        return SUCCESS;
    }
    else{
        printf("%s downloader 下載失敗!\n",protocols[p].name);
        return FAIL;
    }
}

void MergeFolderAndFileName(char *outputFileName, char *downloadFolder, char *orgOutputFileName){
    strcat(outputFileName, downloadFolder);
    strcat(outputFileName, orgOutputFileName);
}
#endif /* downloaders_h */

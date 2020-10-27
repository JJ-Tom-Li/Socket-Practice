//
//  main.c
//  HttpSocketTest - Download file though HTTP socket.
//
//  Created by user on 2020/10/16.
//

#include <stdio.h>
#include <stdlib.h>
#include "downloaders.h"

int main(int argc, const char * argv[]) {
    
    char url[50000];
    char outputFileName[1000];
    char downloadFolder[1000];
    
    printf("請輸入檔案下載網址:");
    scanf("%s", url);
    printf("請指定下載資料夾:");
    scanf("%s", downloadFolder);
    strcat(downloadFolder,"/");
    printf("請命名檔案:");
    scanf("%s", outputFileName);
    
    if (Download(url, outputFileName, downloadFolder) == FAIL) {
        return FAIL;
    }
    
    return SUCCESS;
}

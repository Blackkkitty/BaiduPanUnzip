#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32  
#include <direct.h>  
#include <io.h>  
#elif _LINUX  
#include <stdarg.h>  
#include <sys/stat.h>  
#endif  
  
#ifdef _WIN32  
#define ACCESS _access  
#define MKDIR(a) _mkdir((a))  
#elif _LINUX  
#define ACCESS access  
#define MKDIR(a) mkdir((a),0755)  
#endif  

typedef long long int64;
#define FLAG_F 0x04034b50
#define FLAG_D 0x02014b50
#define BFSIZE 0x10000

int CreatDir(char *pDir)  {  
    int iRet, iLen, i;  
    char* pszDir;  
  
    if(NULL == pDir) return 0;

    pszDir = strdup(pDir);  
    iLen = strlen(pszDir);  
  
    // 创建中间目录  
    for (i = 0;i < iLen;i ++){  
        if (pszDir[i] == '\\' || pszDir[i] == '/'){   
            pszDir[i] = '\0';  
            //如果不存在,创建  
            iRet = ACCESS(pszDir,0);  
            if (iRet != 0){  
                iRet = MKDIR(pszDir);  
                if (iRet != 0) return -1;
            }  
            //支持linux,将所有\换成/  
            pszDir[i] = '/';  
        }   
    }  

    iRet = MKDIR(pszDir);  
    free(pszDir);  
    return iRet;  
}  

int unzip(char *filename) {
    int sz, fnlen, flag;
    char *fn, *buffer;
    buffer = (char *)malloc(BFSIZE);
    FILE *fh = fopen64(filename, "rb");
    fseeko64(fh, 0, SEEK_SET);
    while (true) {
        sz = fnlen = 0;

        // 读取文件标识
        fread(&flag, 1, 4, fh);
        if (flag != FLAG_F) break;
        
        // 读取文件大小
        fseeko64(fh, 14, SEEK_CUR);
        fread(&sz, 1, 4, fh);

        // 读取文件名长度
        fseeko64(fh, 4, SEEK_CUR);
        fread(&fnlen, 1, 2, fh);

        // 读取文件名
        fseeko64(fh, 2, SEEK_CUR);
        fn = (char *)malloc(fnlen + 1);
        fread(fn, 1, fnlen, fh);
        fn[fnlen] = '\0';
        
        printf(" - %s [Size: %d]\n", fn, sz);
        // 读取并写入文件（如果文件大小为0，则创建目录）
        if (sz == 0) {
            CreatDir(fn);
        } else {
            FILE *outfh = fopen(fn, "wb+");
            for (; sz >= BFSIZE; sz -= BFSIZE) {
                fread(buffer, 1, BFSIZE, fh);
                fwrite(buffer, 1, BFSIZE, outfh);
            }
            if (sz) {
                fread(buffer, 1, sz, fh);
                fwrite(buffer, 1, sz, outfh);
            }
            fclose(outfh);
        }
        
        free(fn);
    }
    fclose(fh);
    free(buffer);
    return 0;
}

int main(int argc, char *argv[]) {
    if (1 >= argc)
        goto Help;
    else {
        for (int i = 1; i < argc; i++) {
            printf("Unzip File: %s\n", argv[i]);
            unzip(argv[i]);
        }
        printf("Finished!\n");
        return 0;
    }

Help:;
    puts("\nUsage: bdy_unzip filename_1.zip filename_2.zip\n");
    return 0;
}
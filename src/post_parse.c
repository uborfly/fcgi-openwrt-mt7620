#include "post_parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "get_parse.h"
#include "fcgi_stdio.h"
#include "utils.h"
#include "json-c/json.h"
#include "openssl/evp.h"
#include "file_page.h"
#include "gpio.h"

char globalToken[0x20];

void fill_random(uint8_t *buf, size_t len)
{
    int allCount = len / 4;
    int lessCount = len % 4;
    uint8_t *p = buf;
    uint32_t r;
    uint8_t *rr = (uint8_t *)&r;
    srand((unsigned int)time(0));
    while (allCount-- > 0)
    {
        r = rand() % UINT32_MAX;
        *(p++) = rr[0];
        *(p++) = rr[1];
        *(p++) = rr[2];
        *(p++) = rr[3];
    }
    while (lessCount-- > 0)
    {
        r = rand() % UINT32_MAX;
        *(p++) = rr[0];
    }
}

int sm3_hash(const unsigned char *message, size_t len, unsigned char *hash, unsigned int *hash_len)
{
    EVP_MD_CTX *md_ctx;
    const EVP_MD *md;

    md = EVP_sm3();
    md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, md, NULL);
    EVP_DigestUpdate(md_ctx, message, len);
    EVP_DigestFinal_ex(md_ctx, hash, hash_len);
    EVP_MD_CTX_free(md_ctx);
    return 0;
}

void test_sm3()
{
    const unsigned char sample1[] = {'a', 'b', 'c', 0};
    unsigned int sample1_len = strlen((char *)sample1);

    unsigned char hash_value[64];
    unsigned int i, hash_len;

    sm3_hash(sample1, sample1_len, hash_value, &hash_len);
    LOG("raw data: %s\n", sample1);
    LOG("hash length: %d bytes.\n", hash_len);
    LOG("hash value:\n");
    for (i = 0; i < hash_len; i++)
    {
        LOG("0x%x  ", hash_value[i]);
    }
    LOG("\n");
}

void json_test()
{
    //创建字符串型的json对象
    json_object *j_code = json_object_new_string("200");
    json_object *j_msg = json_object_new_string("ok");
    //创建一个数组对象
    json_object *j_array = json_object_new_array();
    //给元素添加到数组末尾
    json_object *j_data = json_object_new_object();
    json_object_object_add(j_data, "name", json_object_new_string("file1"));
    json_object_object_add(j_data, "create_name", json_object_new_string("20200709"));
    json_object_object_add(j_data, "size", json_object_new_string("123"));

    json_object_array_add(j_array, j_data);

    // json_object_object_add(j_array, "", j_name);

    //将上面创建的对象加入到json对象j_cfg中
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(j_cfg, "code", j_code);
    json_object_object_add(j_cfg, "message", j_msg);
    json_object_object_add(j_cfg, "data", j_array);
    //打印j_cfg
    LOG("j_cfg:%s<br>", json_object_to_json_string(j_cfg));
}
#define bufLen 1024
#define ROOT "/mnt"
int post_para(int cmd, int length)
{
    int nowReadLen = 0;

    LOG("<pre>");
    LOG("cmd:%d\tlength:%d<br>", cmd, length);

    if (length < bufLen)
        nowReadLen = length;
    else
        nowReadLen = bufLen;
    char postBuf[nowReadLen];

    FCGI_fread(postBuf, sizeof(char), nowReadLen, stdin);

    postBuf[nowReadLen] = '\0';
    LOG("%s", postBuf);

    switch (cmd)
    {
    case INVALID:
        ret_json("500", "无效的cmd");
        LOG("CMD INVALID<br>");
        break;
    case LOGIN:
    {
        char *dataBuf;
        char *bufReadP;
        char userName[64];
        char passwd[64];
        int boundaryLen = 0;
        bufReadP = postBuf;
        //parse username
        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", userName);
        if (strcmp(userName, "username\""))
        {
            ret_json("500", "不符合规定的username");
            break;
        }
        bufReadP += strlen(userName) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", userName);
        LOG("username:%s<br>", userName);

        //parse passwd
        bufReadP += boundaryLen + strlen(userName) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", passwd);
        if (strcmp(passwd, "passwd\""))
        {
            ret_json("500", "不符合规定的passwd");
            break;
        }
        bufReadP += strlen(passwd) + 7 + strlen("\r\n");

        sscanf(bufReadP, "%s\r\n%*s", passwd);
        LOG("passwd:%s<br>", passwd);
        if (strcmp(userName, "admin") || strcmp(passwd, "123456"))
        {
            ret_json("500", "错误的用户名或密码");
            break;
        }

        uint8_t randBuf[0x10];
        fill_random(randBuf, 0x10);
        byteToHex(globalToken, randBuf, 0x10);
        LOG("randDataBuf:%s<br>\n", globalToken);

        //return
        json_object *j_cfg = json_object_new_object();
        json_object *j_code = json_object_new_string("200");
        json_object_object_add(j_cfg, "code", j_code);
        json_object *j_msg = json_object_new_string("ok");
        json_object_object_add(j_cfg, "mssage", j_msg);
        json_object *j_data = json_object_new_string(globalToken);
        json_object_object_add(j_cfg, "data", j_data);

        FCGI_printf("Status:200 OK\r\n");
        FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                    "\r\n"
                    "%s",
                    json_object_to_json_string(j_cfg));
        break;
    }
    case FILE_LIST:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s<br>", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s<br>", token);
        LOG("globalToken:%s<br>", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_list_display(rootPath);
        break;
    }
    case FILE_DOWNLOAD:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s<br>", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s<br>", token);
        LOG("globalToken:%s<br>", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_download(rootPath);
        break;
    }
    case FILE_UPLOAD:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[64];
        int boundaryLen = 0;
        bufReadP = postBuf;
        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);

        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s<br>", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);

        // LOG("token:%s<br>", token);
        // LOG("globalToken:%s<br>", globalToken);
        // // verify token
        // if (strcmp(token, globalToken))
        // {
        //     ret_json("500", "token无效");
        //     break;
        // }
        // //check mount
        // if (dev_check())
        // {
        //     ret_json("500", "设备未挂载");
        //     break;
        // }

        //get filename
        bufReadP = postBuf;
        dataBuf = strstr(postBuf, "filename=");
        int filePos = strlen(postBuf) - strlen(dataBuf);

        char fileName[64];
        // LOG("dataBuf:%s\n", dataBuf);

        sscanf(dataBuf, "filename=\"%s\"", fileName);
        fileName[strlen(fileName) - 1] = '\0';
        // LOG("fileName:%s\n", fileName);

        bufReadP += filePos + strlen(fileName) + strlen("filename=") + 2 + strlen("\r\n");
        // LOG("bufReadP:%s\n", bufReadP);

        dataBuf = strstr(bufReadP, "\r\n\r\n");
        // LOG("dataBuf:%s\n", dataBuf);
        bufReadP += strlen(bufReadP) - strlen(dataBuf) + strlen("\r\n\r\n");
        // LOG("bufReadP:%s\n", bufReadP);

        char rootPath[128] = ROOT;
        strcat(rootPath, path);
        printf("rootPath:%s  %s\n", rootPath, path);

        //check upper dir exist
        char *retDirName = strrchr(rootPath, '/');
        printf("dest dir:%s\n", retDirName);
        char upperDir[strlen(rootPath) - strlen(retDirName)];
        strncpy(upperDir, rootPath, strlen(rootPath) - strlen(retDirName));
        upperDir[strlen(rootPath) - strlen(retDirName)] = '\0';
        printf("upperDir dir:%s\n", upperDir);
        printf("path:%s\n", rootPath);
        struct stat statBuf;
        if (access(upperDir, F_OK))
        {
            ret_json("500", "上级目录不存在");
            break;
        }

        //check dir exist

        printf("目录存在，判断是否为文件夹\n");
        //is it dir
        stat(rootPath, &statBuf);
        if (!S_ISDIR(statBuf.st_mode)) //判断是否是目录
        {
            ret_json("500", "上级目录不存在,有同名文件");
            break;
        }

        strcat(rootPath, fileName);
        LOG("filePath:%s\n", rootPath);

        nowReadLen -= strlen(postBuf) - strlen(bufReadP);
        LOG("nowReadLen:%d\n", nowReadLen);
        //create file
        FILE *fp = fopen(rootPath, "wb+");
        if (NULL == fp)
        {
            LOG("file open err\n");
            break;
        }
        length -= nowReadLen;
        while (nowReadLen > 0)
        {
            fputc(*bufReadP, fp);
            bufReadP++;
            nowReadLen--;
        }

        while (length > 0)
        {
            if (length < bufLen)
                nowReadLen = length;
            else
                nowReadLen = bufLen;
            char postBuf[nowReadLen];
            // LOG("%d\t%d\n", length, nowReadLen);
            FCGI_fread(postBuf, sizeof(char), nowReadLen, stdin);
            length -= nowReadLen;
            postBuf[nowReadLen] = '\0';

            char *bufP = postBuf;
            while (nowReadLen > 0)
            {
                if (!strncmp(bufP, "\r\n--------", 10))
                {
                    // LOG("-----\n");
                    // LOG("nowReadLen:%d\n", nowReadLen);
                    length = 0;
                    break;
                }

                fputc(*bufP, fp);
                bufP++;
                nowReadLen--;
            }

            //return
            ret_json("200", "ok");
        }
        fclose(fp);
        break;
    }
    case FILE_CREATE_DIR:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s<br>", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s<br>", token);
        LOG("globalToken:%s<br>", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_create_dir(rootPath);
        break;
    }
    case FILE_DELETE:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("path:%s<br>", path);
        //get token
        char token[32];
        bufReadP += boundaryLen + strlen(path) + strlen("\r\n");

        sscanf(bufReadP, " name=\"%s\"", token);

        bufReadP += strlen(token) + 7 + strlen("\n");

        sscanf(bufReadP, "%s\r\n%*s", token);
        LOG("token:%s<br>", token);
        LOG("globalToken:%s<br>", globalToken);
        //verify token
        if (strcmp(token, globalToken))
        {
            ret_json("500", "token无效");
            break;
        }
        //check mount
        if (dev_check())
        {
            ret_json("500", "设备未挂载");
            break;
        }
        char rootPath[] = ROOT;
        strcat(rootPath, path);
        file_delete(rootPath);
        break;
    }
    case DISK_TEST:
    {
        //get path
        char *dataBuf;
        char *bufReadP;
        char path[32];
        int boundaryLen = 0;
        bufReadP = postBuf;

        dataBuf = strstr(postBuf, "name=");
        boundaryLen = strlen(postBuf) - strlen(dataBuf);
        LOG("boundaryLen:%d<br>", boundaryLen);
        bufReadP += boundaryLen;
        sscanf(dataBuf, "name=\"%s%*s", path);
        if (strcmp(path, "path\""))
        {
            ret_json("500", "不符合规定的path");
            break;
        }
        bufReadP += strlen(path) + 7 + strlen("\r\n");

        dataBuf = bufReadP;
        sscanf(dataBuf, "%s\r\n%*s", path);
        LOG("states:%d<br>", atoi(path));

        gpio_export(1);
        gpio_direction(1, 1);
        gpio_write(1, atoi(path));
        LOG("out:%d\n", gpio_read(1));
        break;
    }
    default:
        break;
    }

    LOG("</pre>");
    return 0;
}

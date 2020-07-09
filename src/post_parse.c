#include "post_parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "get_parse.h"
#include "fcgi_stdio.h"
#include "utils.h"
#include "json-c/json.h"
#include "openssl/evp.h"
#include "file_page.h"

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

int post_para(int cmd, int length)
{
    LOG("<pre>");
    LOG("cmd:%d\tlength:%d<br>", cmd, length);
    char postBuf[length];
    FCGI_fread(postBuf, 1, length, stdin);
    LOG("%s<br>", postBuf);
    switch (cmd)
    {
    case INVALID:
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
            LOG("ERR userName<br>");
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
            LOG("ERR passwd<br>");
            break;
        }
        bufReadP += strlen(passwd) + 7 + strlen("\r\n");

        sscanf(bufReadP, "%s\r\n%*s", passwd);
        LOG("passwd:%s<br>", passwd);
        if (strcmp(userName, "admin") || strcmp(passwd, "123456"))
        {
            FCGI_printf("Status:201\r\n");
            FCGI_printf("Content-type: text/html;charset=utf-8\r\n"
                        "\r\n"
                        "invalid username or password");
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
        //get token
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
            FCGI_printf("Status:200 OK\r\n");
            FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                        "\r\n"
                        "Err path");
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
            json_object *j_cfg = json_object_new_object();
            json_object *j_code = json_object_new_string("500");
            json_object_object_add(j_cfg, "code", j_code);
            json_object *j_msg = json_object_new_string("token无效");
            json_object_object_add(j_cfg, "mssage", j_msg);

            FCGI_printf("Status:200 OK\r\n");
            FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                        "\r\n"
                        "%s",
                        json_object_to_json_string(j_cfg));
            break;
        }
        //check mount
        if (dev_check())
        {
            json_object *j_cfg = json_object_new_object();
            json_object *j_code = json_object_new_string("501");
            json_object_object_add(j_cfg, "code", j_code);
            json_object *j_msg = json_object_new_string("设备未挂载");
            json_object_object_add(j_cfg, "mssage", j_msg);

            FCGI_printf("Status:200 OK\r\n");
            FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                        "\r\n"
                        "%s",
                        json_object_to_json_string(j_cfg));
            break;
        }
        char rootPath[] = "/mnt/";
        strcat(rootPath, path);
        file_list_display(rootPath);
        break;
    }
    case FILE_DOWNLOAD:
    {
        break;
    }
    case FILE_UPLOAD:
    {
        break;
    }
    case FILE_CREATE_DIR:
    {
        break;
    }
    case FILE_DELETE:
    {
        break;
    }
    default:
        break;
    }
    LOG("</pre>");
    return 0;
}

/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 14:35:13
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-12-30 11:25:27
 * @Description  : 程序入口main(),fcgi初始化
 * @FilePath     : /fcgi-openwrt-mt7620/src/main.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "fcgi_stdio.h"
#include "get_parse.h"
#include "utils.h"
#include "file_page.h"
#include "post_parse.h"

int app_fcgi_init()
{
    int postLength = 0;

    while (FCGI_Accept() >= 0)
    {
        LOG("Status:203 DEBUG\r\n");
        LOG("Content-type: text/html;charset=utf-8\r\n"
            "\r\n");

        //GET PARSE
        if (strncmp(getenv("CONTENT_TYPE"), "application/json", 16))
        {
            LOG("CONTENT_TYPE:%s\n", getenv("CONTENT_TYPE"));
            ret_json("500", "CONTENT_TYPE错误");
            continue;
        }

        if (NULL == getenv("QUERY_STRING"))
        {
            ret_json("500", "missing query params");
            continue;
        }
        int ret = get_para(getenv("QUERY_STRING")); //解析接口

        LOG("get_para()ret = %d\n", ret);
        //POST PARSE
        postLength = atoi((char *)getenv("CONTENT_LENGTH"));
        LOG("CONTENT_LENGTH:%d\n", postLength);

        if (!strcmp(getenv("REQUEST_METHOD"), "POST") && postLength > 0)
        {
            LOG("\nPOST_METHOD:\n");
            post_para(ret, postLength); //接口名称解析处理
        }
        else
        {
            ret_json("500", "POST_BODY_ERROR");
        }
    }

    return 0;
}

int main(void)
{
    /*nginx搭建测试*/
    // int count = 0;

    // while (FCGI_Accept() >= 0)
    // {
    //     printf("Content-type: text/html\r\n"
    //            "\r\n"
    //            "<title>Hello World</title>"
    //            "<h1>Hello World from FastCGI!</h1>"
    //            "Request number is: %d\n",
    //            ++count);
    // }
    /*nginx搭建测试*/

    app_fcgi_init();
    //多线程模型
    // pthread_t pthread_id;
    // pthread_create(&pthread_id, NULL, app_fcgi_init, NULL);
    // while (1)
    // {
    // }
    return 0;
}

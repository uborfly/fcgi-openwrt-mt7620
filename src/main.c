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
        if (NULL == getenv("QUERY_STRING"))
        {
            ret_json("500", "missing query params");
            continue;
        }
        int ret = get_para(getenv("QUERY_STRING"));

        LOG("ret = %d <br>", ret);
        //POST PARSE
        postLength = atoi((char *)getenv("CONTENT_LENGTH"));
        LOG("<br />CONTENT_LENGTH:%d<br />", postLength);

        if (!strcmp(getenv("REQUEST_METHOD"), "POST") && postLength > 0)
        {
            LOG("<br />POST_METHOD:<br />");
            post_para(ret, postLength);
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
    pthread_t pthread_id;
    pthread_create(&pthread_id, NULL, app_fcgi_init, NULL);
    while (1)
    {
    }
    return 0;
}

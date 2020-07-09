#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
            FCGI_printf("Status:201 ERR\r\n");
            FCGI_printf("Content-type: text/html;charset=utf-8\r\n"
                        "\r\n"
                        "missing query params");
            continue;
        }
        int ret = get_para(getenv("QUERY_STRING"));

        LOG("ret = %d <br>", ret);
        //POST PARSE
        // postLength = tonumber((uint8_t *)getenv("CONTENT_LENGTH"), strlen(getenv("CONTENT_LENGTH")));
        postLength = atoi((char *)getenv("CONTENT_LENGTH"));
        LOG("<br />CONTENT_LENGTH:%d<br />", postLength);

        if (!strcmp(getenv("REQUEST_METHOD"), "POST") && postLength > 0)
        {
            LOG("<br />POST_METHOD:<br />");
            post_para(ret, postLength);
        }
        else
        {
            FCGI_printf("Status:201 ERR\r\n");
            FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                        "\r\n"
                        "POST_BODY_ERROR");
        }
        // file_list_init();
    }

    return 0;
}

int main(void)
{
    app_fcgi_init();
}

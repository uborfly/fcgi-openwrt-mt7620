/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-08-06 15:16:28
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-09-23 15:32:35
 * @FilePath     : /fcgi-openwrt-mt7620/inc/multipart_parser.h
 * @Description  : post中form-data解析头文件
 */
#ifndef _MULTIPART_PARSER_H_
#define _MULTIPART_PARSER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>
#include <ctype.h>

    typedef struct multipart_parser multipart_parser;
    typedef struct multipart_parser_settings multipart_parser_settings;
    typedef struct multipart_parser_state multipart_parser_state;

    typedef int (*multipart_data_cb)(multipart_parser *, const char *at, size_t length);
    typedef int (*multipart_notify_cb)(multipart_parser *);

    struct multipart_parser_settings
    {
        multipart_data_cb on_header_field;
        multipart_data_cb on_header_value;
        multipart_data_cb on_part_data;

        multipart_notify_cb on_part_data_begin;
        multipart_notify_cb on_headers_complete;
        multipart_notify_cb on_part_data_end;
        multipart_notify_cb on_body_end;
    };

    multipart_parser *multipart_parser_init(const char *boundary, const multipart_parser_settings *settings);

    void multipart_parser_free(multipart_parser *p);

    size_t multipart_parser_execute(multipart_parser *p, const char *buf, size_t len);

    void multipart_parser_set_data(multipart_parser *p, void *data);
    void *multipart_parser_get_data(multipart_parser *p);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

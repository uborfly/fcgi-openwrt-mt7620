#include "utils.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "json-c/json.h"

#define BYTES_PER_LINE 16

void ret_json(char *code, char *message)
{
    json_object *j_cfg = json_object_new_object();
    json_object *j_code = json_object_new_string(code);
    json_object_object_add(j_cfg, "code", j_code);
    json_object *j_msg = json_object_new_string(message);
    json_object_object_add(j_cfg, "message", j_msg);

    FCGI_printf("Status:200 OK\r\n");
    FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                "\r\n"
                "%s",
                json_object_to_json_string(j_cfg));
}

void log_buffer_hex(const void *buffer, uint16_t buff_len)
{
    if (buff_len == 0)
        return;
    char temp_buffer[BYTES_PER_LINE + 3]; //for not-byte-accessible memory
    char hex_buffer[3 * BYTES_PER_LINE + 1];
    const char *ptr_line;
    int bytes_cur_line;

    do
    {
        if (buff_len > BYTES_PER_LINE)
        {
            bytes_cur_line = BYTES_PER_LINE;
        }
        else
        {
            bytes_cur_line = buff_len;
        }

        ptr_line = buffer;

        int i;
        for (i = 0; i < bytes_cur_line; i++)
        {
            sprintf(hex_buffer + 3 * i, "%02x ", ptr_line[i]);
        }
        LOG("%s\n", hex_buffer);
        buffer += bytes_cur_line;
        buff_len -= bytes_cur_line;
    } while (buff_len);
}

uint32_t tonumber(uint8_t *data, uint8_t data_len)
{
    uint32_t num = 0;
    int i = 0;
    for (i = 0; i < data_len; i++)
    {
        num += (data[i] - '0') * pow(10, data_len - i - 1);
    }
    return num;
}

char *tostring(long num)
{
    static char str[20];

    int len = 1;

    long i = num;
    while (i >= 10)
    {
        i = i / 10;
        // printf("//%ld--", i);
        len++;
    }
    // printf("%d", len);
    memset(str, '0', len);
    str[len] = '\0';
    for (; len > 0; len--)
    {
        str[len - 1] = num % 10 + '0';
        num = num / 10;
    }

    // printf("end:%s\n", str);
    return str;
}

static unsigned char bToH(uint8_t x)
{
    if (x >= 0 && x <= 9)
    {
        return x + '0';
    }
    else if (x >= 'A' && x <= 'F')
    {
        return x + 'A' - 10;
    }
    else if (x >= 'a' && x <= 'f')
    {
        return x + 'a' - 10;
    }
}

//16进制转字符串
unsigned char byteToHex(char *out, uint8_t *src, uint8_t len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        // printf("%02x<br>", src[i]);
        sprintf(out + i * 2, "%02x", src[i]); //格式化输出到buffer,每个unsigned char 转换为字符后占两个位置，%x小写输出，%X大写输出
        // printf("to string:%s\n", out);
    }
    return 0;
}

static unsigned char hToB(unsigned char x)
{
    if (x >= '0' && x <= '9')
    {
        return x - '0';
    }
    else if (x >= 'A' && x <= 'F')
    {
        return x - 'A' + 10;
    }
    else if (x >= 'a' && x <= 'f')
    {
        return x - 'a' + 10;
    }
    return 0;
}

//字符转16进制
unsigned char hexToByte(unsigned char *s)
{
    return (hToB(s[0]) << 4) | (hToB(s[1]));
}

uint8_t checkIsHex(unsigned char x)
{
    if (x >= '0' && x <= '9')
    {
        return 1;
    }
    else if (x >= 'A' && x <= 'F')
    {
        return 1;
    }
    else if (x >= 'a' && x <= 'f')
    {
        return 1;
    }
    return 0;
}

#define BURSIZE 2048

int hex2dec(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else
    {
        return -1;
    }
}

char dec2hex(short int c)
{
    if (0 <= c && c <= 9)
    {
        return c + '0';
    }
    else if (10 <= c && c <= 15)
    {
        return c + 'A' - 10;
    }
    else
    {
        return -1;
    }
}

//编码一个url
void urlencode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i)
    {
        char c = url[i];
        if (('0' <= c && c <= '9') ||
            ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            c == '/' || c == '.')
        {
            res[res_len++] = c;
        }
        else
        {
            int j = (short int)c;
            if (j < 0)
                j += 256;
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            res[res_len++] = '%';
            res[res_len++] = dec2hex(i1);
            res[res_len++] = dec2hex(i0);
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}

// 解码url
void urldecode(char url[])
{
    int i = 0;
    int len = strlen(url);
    int res_len = 0;
    char res[BURSIZE];
    for (i = 0; i < len; ++i)
    {
        char c = url[i];
        if (c != '%')
        {
            res[res_len++] = c;
        }
        else
        {
            char c1 = url[++i];
            char c0 = url[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            res[res_len++] = num;
        }
    }
    res[res_len] = '\0';
    strcpy(url, res);
}

int unicode_to_utf8(unsigned long unic, unsigned char *pOutput, int outSize)
{
    assert(pOutput != NULL);
    assert(outSize >= 3);

    if (unic <= 0x0000007F)
    {
        // * U-00000000 - U-0000007F:  0xxxxxxx
        *pOutput = (unic & 0x7F);
        return 1;
    }
    else if (unic >= 0x00000080 && unic <= 0x000007FF)
    {
        // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
        *(pOutput + 1) = (unic & 0x3F) | 0x80;
        *pOutput = ((unic >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else if (unic >= 0x00000800 && unic <= 0x0000FFFF)
    {
        // * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
        *(pOutput + 2) = (unic & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
        *pOutput = ((unic >> 12) & 0x0F) | 0xE0;
        return 3;
    }
    else if (unic >= 0x00010000 && unic <= 0x001FFFFF)
    {
        // * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 3) = (unic & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
        *pOutput = ((unic >> 18) & 0x07) | 0xF0;
        return 4;
    }
    else if (unic >= 0x00200000 && unic <= 0x03FFFFFF)
    {
        // * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 4) = (unic & 0x3F) | 0x80;
        *(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
        *pOutput = ((unic >> 24) & 0x03) | 0xF8;
        return 5;
    }
    else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF)
    {
        // * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        *(pOutput + 5) = (unic & 0x3F) | 0x80;
        *(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
        *(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
        *(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
        *(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
        *pOutput = ((unic >> 30) & 0x01) | 0xFC;
        return 6;
    }

    return 0;
}

void strdel(char a[], char c)
{
    int i, j;
    for (i = 0, j = 0; *(a + i) != '\0'; i++)
    {
        if (*(a + i) == c)
            continue;
        else
        {
            *(a + j) = *(a + i);
            j++;
        }
    }
    *(a + j) = '\0';
}

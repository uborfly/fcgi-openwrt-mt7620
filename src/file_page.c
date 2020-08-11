#include "file_page.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "utils.h"
#include "libudev.h"
#include "json-c/json.h"

int gRecvFileCnt = 0;
int gRecvFileRemain = 0;
FILE *g_fp = NULL;

static void get_file_path(const char *path, const char *filename, char *filepath)
{
    strcpy(filepath, path);
    if (filepath[strlen(path) - 1] != '/')
        strcat(filepath, "/");
    strcat(filepath, filename);
}

static void ret_file_path(const char *path, const char *fileName, struct json_object *val)
{
    char filePath[255];
    struct stat statBuf;

    strcpy(filePath, path);

    if (filePath[strlen(filePath) - 1] != '/')
        strcat(filePath, "/");
    strcat(filePath, fileName);
    // FCGI_printf("%s::%c<br>", filePath, filePath[strlen(filePath) - 1]);
    stat(filePath, &statBuf);

    json_object *j_data = json_object_new_object();

    json_object_object_add(j_data, "name", json_object_new_string(fileName));
    json_object_object_add(j_data, "create_time", json_object_new_string(ctime(&statBuf.st_ctime)));
    if (S_ISDIR(statBuf.st_mode)) //判断是否是目录
    {
        json_object_object_add(j_data, "size", json_object_new_string("dir"));
    }
    else if (S_ISREG(statBuf.st_mode))
    {
        json_object_object_add(j_data, "size", json_object_new_string(tostring(statBuf.st_size)));
    }

    json_object_array_add(val, j_data);
}

int file_list_display(char *path)
{
    // LOG("<br />show_list<br />");
    DIR *dir;
    struct dirent *dirInfo;

    if ((dir = opendir(path)) == NULL)
    {
        ret_json("500", "挂载点不存在");
        return 1;
    }
    json_object *j_code = json_object_new_string("200");
    json_object *j_msg = json_object_new_string("ok");

    json_object *j_array = json_object_new_array();

    while ((dirInfo = readdir(dir)) != NULL)
    {
        if (!strcmp(dirInfo->d_name, ".") || !strcmp(dirInfo->d_name, ".."))
            continue;
        // LOG("path:%s\nd_name:%s\n", path, dirInfo->d_name);
        ret_file_path(path, dirInfo->d_name, j_array);
    }
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(j_cfg, "code", j_code);
    json_object_object_add(j_cfg, "message", j_msg);
    json_object_object_add(j_cfg, "data", j_array);
    closedir(dir);
    FCGI_printf("Status:200 OK\r\n");
    FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                "\r\n"
                "%s",
                json_object_to_json_string(j_cfg));
}

int dev_check()
{
    return access("/proc/scsi/usb-storage", F_OK);
}

void file_download(char *path)
{
    struct stat statBuf;
    stat(path, &statBuf);
    if (S_ISDIR(statBuf.st_mode)) //判断是否是目录
    {
        ret_json("500", "该路径为目录");
        return;
    }
    //check file is exist
    FILE *fp = fopen(path, "rb+");
    if (NULL == fp)
    {
        ret_json("500", "文件读取失败");
        return;
    }

    //return
    FCGI_printf("Status:200 OK\r\n");
    FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                "\r\n");
    //write to stream
    // long i = 0;
    while (!feof(fp))
    {
        fputc(fgetc(fp), stdout);
        // i++;
    }
    fclose(fp);
}

int file_upload_init(char *path, int cnt, int remain)
{
    struct stat statBuf;
    //is it dir
    stat(path, &statBuf);
    if (S_ISDIR(statBuf.st_mode)) //判断是否是目录
    {
        ret_json("500", "文件不存在,有同名文件夹");
        return 1;
    }
    gRecvFileCnt = cnt;
    gRecvFileRemain = remain;
    LOG("\path:%s\ncnt:%d\nremain:%d\n", path, cnt, remain);
    //create file
    g_fp = fopen(path, "wb+");
    if (NULL == g_fp)
    {
        ret_json("500", "file open err");
        return 1;
    }
    return 0;
}

int file_upload_data(int fileCnt, int dataLength, char *end, char *data, char *check)
{
    if (fileCnt > gRecvFileCnt)
    {
        ret_json("500", "传输错误");
        return 1;
    }
    if (!strcmp(end, "true") && strcmp(check, "null"))
    {
        //校验
        {
            fwrite(data, dataLength, 1, g_fp);
            fclose(g_fp);
            return 0;
        }
    }
    if (strlen(data) != dataLength)
    {
        ret_json("500", "数据缺失");
        return 1;
    }
    fwrite(data, dataLength, 1, g_fp);
    return 0;
}

void file_create_dir(char *path)
{
    //check upper dir exist
    char *retDirName = strrchr(path, '/');
    printf("dest dir:%s\n", retDirName);
    char upperDir[strlen(path) - strlen(retDirName)];
    strncpy(upperDir, path, strlen(path) - strlen(retDirName));
    upperDir[strlen(path) - strlen(retDirName)] = '\0';
    printf("upperDir dir:%s\n", upperDir);
    printf("path:%s\n", path);
    struct stat statBuf;
    if (access(upperDir, F_OK))
    {
        ret_json("500", "上级目录不存在");
        return;
    }

    //check dir exist
    if (!access(path, F_OK))
    {
        printf("文件存在，判断是否为文件夹\n");

        //is it dir
        stat(path, &statBuf);
        if (S_ISDIR(statBuf.st_mode)) //判断是否是目录
        {
            ret_json("500", "目录已存在");
            return;
        }
        else
        {
            ret_json("500", "同名文件已存在");
            return;
        }
    }
    //create dir
    int ret = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (-1 == ret)
    {
        ret_json("500", "创建目录失败");
        return;
    }
    //return
    ret_json("200", "ok");
}

void file_delete(char *path)
{
    FCGI_printf("Status:200 OK\r\n");
    FCGI_printf("Content-type: application/json;charset=utf-8\r\n"
                "\r\n");
    //check upper dir exist
    char *retDirName = strrchr(path, '/');
    printf("dest dir:%s\n", retDirName);
    char upperDir[strlen(path) - strlen(retDirName)];
    strncpy(upperDir, path, strlen(path) - strlen(retDirName));
    upperDir[strlen(path) - strlen(retDirName)] = '\0';
    printf("upperDir dir:%s\n", upperDir);
    printf("path:%s\n", path);
    struct stat statBuf;
    if (access(upperDir, F_OK))
    {
        ret_json("500", "上级目录不存在");
        return;
    }

    //check file exist
    if (!access(path, F_OK))
    {
        printf("文件存在，判断是否为文件夹\n");

        //is it dir
        DIR *dir;
        struct dirent *dirinfo;
        struct stat statbuf;
        char filepath[256] = {0};
        stat(path, &statBuf);
        if (S_ISDIR(statBuf.st_mode)) //判断是否是目录
        {
            if ((dir = opendir(path)) == NULL)
            {
                ret_json("500", "文件夹读取失败");
                return;
            }
            while (1)
            {
                dirinfo = readdir(dir);
                if (dirinfo <= 0)
                {
                    break;
                }
                if ((strcmp(".", dirinfo->d_name) == 0) || (strcmp("..", dirinfo->d_name) == 0))
                {
                    continue;
                }
                /*判断是否有目录和文件*/
                if ((dirinfo->d_type == DT_DIR) || (dirinfo->d_type == DT_REG))
                {
                    ret_json("500", "目录内存在子文件");
                    return;
                }
            }
            // while ((dirinfo = readdir(dir)) != NULL)
            // {
            //     get_file_path(path, dirinfo->d_name, filepath);
            //     if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) //判断是否是特殊目录
            //         continue;
            //     DeleteFile(filepath);
            //     rmdir(filepath);
            // }
            closedir(dir);
            rmdir(path);
            ret_json("200", "ok");
            return;
        }
        else
        {
            remove(path);
            ret_json("200", "ok");
            return;
        }
    }
    else
    {
        ret_json("500", "文件或目录不存在");
    }
    //return
}

int disk_format(char *devName, char *devType)
{

    return 0;
}

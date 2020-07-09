#include "file_page.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "utils.h"
#include "libudev.h"
#include "json-c/json.h"

static void Getfilepath(const char *path, const char *fileName, struct json_object *val)
{
    char filePath[255];
    struct stat statbuf;

    strcpy(filePath, path);

    if (filePath[strlen(filePath) - 1] != '/')
        strcat(filePath, "/");
    strcat(filePath, fileName);
    // FCGI_printf("%s::%c<br>", filePath, filePath[strlen(filePath) - 1]);
    stat(filePath, &statbuf);

    json_object *j_data = json_object_new_object();

    json_object_object_add(j_data, "name", json_object_new_string(fileName));
    json_object_object_add(j_data, "create_name", json_object_new_string(ctime(&statbuf.st_ctime)));
    if (S_ISDIR(statbuf.st_mode)) //判断是否是目录
    {
        // FCGI_printf("<a href=\"http://192.168.1.1:8082%s\">%s</a><br>", filePath, fileName);

        json_object_object_add(j_data, "size", json_object_new_string("dir"));
    }
    else if (S_ISREG(statbuf.st_mode))
    {
        // FCGI_printf("<br /><a href=\"http://192.168.1.1:8082%s\">%s</a><br />", filePath, fileName);
        json_object_object_add(j_data, "size", json_object_new_string(tostring(statbuf.st_size)));
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
        LOG("<p>mount point not existed</p>");
        return 1;
    }
    json_object *j_code = json_object_new_string("200");
    json_object *j_msg = json_object_new_string("ok");

    json_object *j_array = json_object_new_array();

    while ((dirInfo = readdir(dir)) != NULL)
    {
        Getfilepath(path, dirInfo->d_name, j_array);
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

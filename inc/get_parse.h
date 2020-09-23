/*
 * @Author       : Kexiang Zhang
 * @Date         : 2020-09-23 15:27:17
 * @LastEditors  : Kexiang Zhang
 * @LastEditTime : 2020-09-23 15:32:54
 * @FilePath     : /fcgi-openwrt-mt7620/inc/get_parse.h
 * @Description  : HTTP GET接口解析头文件
 */
#ifndef __GET_PARSE_H__
#define __GET_PARSE_H__

//HTTP接口定义
typedef enum CMD_PARAM
{
    INVALID,         //无效接口
    LOGIN,           //登录
    FILE_LIST,       //文件列表获取
    FILE_DOWNLOAD,   //文件下载
    FILE_UPLOAD,     //文件上传
    FILE_TRANSPORT,  //文件上传时数据传输
    FILE_CREATE_DIR, //创建文件夹
    FILE_DELETE,     //删除文件
    SAMBA_CONFIG,    //samba配置
    DISK_TEST,       //磁盘断电
    DEV_LIST,        //磁盘挂载列表获取
    DISK_FORMAT      //磁盘格式化
};

int get_para(char *str);

#endif

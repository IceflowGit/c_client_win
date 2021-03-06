﻿/* 
*本文件为client的程序头文件
*程序员：李高杰 严骋龙
*完成日期：2012.11.1
*
*修改记录：
*根据老大的讲解和同事的建议和意见进行了代码整理和就该（2012.11.1）
*修改了显示格式混乱问题（2012.11.12）
*
*版本号：v1.0.1
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<assert.h>
#include<stdarg.h>
#include<string.h>
#include <locale.h>
/*条件编译，linux和windows系统可执行程序*/
#ifdef _MINGW_
#include<winsock2.h>
#include <windows.h>
#include<ws2tcpip.h>
#else
#include<time.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#endif
/*client.c程序中用到的宏定义*/
#define QR_PACK_LEN    sizeof(QR_PACK)//定义请求包长度
#define QR_ID          10   //定义请求包协议ID号
#define MAX_LEN        4097	// 返回包最大是4096
#define TERMINAL_PRINT wprintf
#define QR_BODY_LEN    64  //定义请求包体长度

#define MAX_EMAIL 32
#define MAX_EXTENSION 32
#define MAX_PRIVATION 32
#define MAX_COMPANY 32
#define MAX_FULL 32
#define MAX_ABBREVIATION 32
#define MAX_MYNAME 32

/*返回信息的结构样式*/
typedef struct Infor_Struct//员工信息结构体；
{
    char myname[MAX_MYNAME];
    char abbreviation[MAX_ABBREVIATION];
    char full[MAX_FULL];
    char company[MAX_COMPANY];
    char privation[MAX_PRIVATION];
    char extension[MAX_EXTENSION];
    char emall[MAX_EMAIL];
}INFOR;

/*请求包结构*/
typedef struct {//定义请求包；
    int package_len;
    int package_id;
    char  body[QR_BODY_LEN];
}QR_PACK;
/*请求包头结构*/
typedef struct {
    int package_len;
    int package_id;
}QR_HEAD;
/*返回包头结构*/
typedef struct {
    int package_len;
	int infor_num;
    int package_id;
}QA_HEAD;
/*函数声明*/
int main_loop();
int tcp_init();
int read_input(char*, int);
int read_stdin(char*, int);
int build_package(char*, int);
int write_ser(char*, int);
int read_ser(char*, int);
int unpackage(char*, int);
int Socket_Create(int, int, int);
int Socket_Write(int, char*, int);
int Socket_Read(int, char*, int);
int tcp_close();
char* UTF8toACP(char* szUtf8,char* szAcp,int nAcpLen);
int terminal_print(char* msg, ...);

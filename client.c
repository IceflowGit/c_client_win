﻿/*
 *本文件为client的源程序文件
 *程序员：李高杰 严骋龙
 *完成日期：2012.10.31
 *
 *修改记录：
 *根据老大的讲解和同事的建议和意见进行了代码整理和修改（2012.10.31）
 *根据老大现场的讲解修改而来UTF8toACP函数，修改了打印问题（2012.11.9）
 *程序修改完毕，调整了所有的打印函数（2012.11.11）
 *
 *版本号：v1.0.1
 *
 */
#include"client.h"
#define MAXSTR 4096
/*
 *全局的变量：
 *socket：client的套接字描述符
 *ip    ：所要连接的server的ip地址
 *port  ：所要连接的server的端口号
 */
int sockfd;//连接套接字描述符；
char ip[256] = {0};
int port;

/*
 *功能：封装函数，解决跨平台打印问题
 *参数：无固定参数
 *返回值：打印的字符串长度
 */
int terminal_print(char* msg, ...)
{	
	int ret = -1;
	va_list argp;
	char sTmp[MAXSTR] = {0};
	char sAcp[MAXSTR] = {0};

	va_start(argp, msg);
	vsnprintf(sTmp, MAXSTR, msg, argp);
	va_end(argp);
#ifdef _MINGW_
	ret = printf("%s", UTF8toACP(sTmp, sAcp, MAXSTR));
#else
	ret = printf("%s",sTmp );
#endif
	return ret;
}
/*
 *功能：编码转换函数
 *参数：1.char* szUtf8:需要转换的字符串
 2.int nUtf8Len：szUtf8的长度
 3.char* szAcp：输出参数，转换后的字符串
 4.int nAcpLen：szAcp的长度
 *返回值：char*--转换后的字符串
 */
#ifdef _MINGW_
char*  UTF8toACP(char* szUtf8, char* szAcp, int nAcpLen)
{
	wchar_t szTmp[MAXSTR] = {0};

	if (szAcp == NULL)
		return NULL;

	if( szUtf8 == NULL || MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1, szTmp, 0) >= MAXSTR ){
		szAcp[0]= 0;
		return szAcp ;
	}

	MultiByteToWideChar(CP_UTF8, 0, szUtf8, -1 ,szTmp, MAXSTR);
	WideCharToMultiByte(CP_ACP, 0,szTmp, -1, szAcp,nAcpLen, NULL, FALSE);

	szAcp[MAXSTR] = 0;

	return szAcp;
}
#endif
/*
 *功能：关闭tcp的连接
 *参数：无参
 *返回值：0 -- 正确执行，<0 -- 执行异常
 */
int tcp_close()
{
	int ret = -1;

#ifdef _MINGW_

	ret = shutdown(sockfd, SD_BOTH);
	if(ret < 0){
		terminal_print("shutdown error\n");
		return -1;
	}

	ret = closesocket(sockfd);
	if(ret < 0){
		terminal_print("close fd error\n");
		return -2;
	}
#else
	ret = shutdown(sockfd, SHUT_RDWR );
	if(ret < 0){
		terminal_print("shutdown error\n");
		return -1;
	}
	ret = close(sockfd);
	if(ret < 0){
		terminal_print("close fd error\n");
		return -2;
	}
#endif
	return 0;
}

/*
 *功能：读取socket函数
 *参数：1.int nsock：描述符
 *	   2.char*pBuf：输出参数，存放读取的数据段的指针
 *      3.int nsize：pBuf的长度
 *返回值：0--结束/关闭，<0--读取错误， >0--读取到的实际字节数
 */
int Socket_Read(int nSock,char* pBuf,int nSize)
{
	int ret;
	ret = recv(nSock,pBuf,nSize,0);
	return ret;
}
/*
 *功能：socket写函数
 *参数：1.int nSock：描述符
 *	   2.char*pBuf：输入参数，存放要写入的数据
 *	   3.int nSize：pBuf的长度
 *返回值：0--结束/关闭，<0--写入错误，>0--实际写入的字节数
 */
int Socket_Write(int nSock,char* pBuf,int nSize)
{
	int ret;
	ret = send(nSock,pBuf,nSize,0);
	return ret;
}

/*
 *功能：创建socket
 *参数：1.int af：地址族协议类型
 *      2.int type：socket类型
 *	   3.int protocol：指定的更加精确地协议
 *返回值：>=0--生成的套接字，-1--生成失败
 */
int Socket_Create(int af,int type,int protocol)
{
	int sock=0;

#ifdef _MINGW_
	WSADATA wsaData ={0};
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) return -1;

	sock=socket(af,type,protocol);
	if (sock == INVALID_SOCKET) return -1;
#else
	sock=socket(af,type,protocol);
	if (sock == -1) return -1;
#endif

	return sock;
}

/*
 *功能：解析QA包
 *参数：1.char*buf:需要解析的包
 *      2.int  len:包长度
 *返回值：0--成功，<0--失败
 */
int unpackage(char* buf, int len)//解析查询结果数据包；
{
	QA_HEAD* head = (QA_HEAD*)buf;
	INFOR* p = (INFOR*)(buf + sizeof(QA_HEAD));

	if(head->package_id != 11){
		terminal_print("package_id error\n");
		return -1;
	}

	if(head->infor_num == 0){
		terminal_print("你要查询的信息不存在！\n");
		return 0;
	}

	if(len != (int)(sizeof(QA_HEAD) +  head->infor_num * sizeof(INFOR)) || len != head->package_len){
		terminal_print("包长度错误\n");
		return -2;
	}

	int i;
	for(i = 0 ; i < head->infor_num ; i++){
		terminal_print("你要查询的信息如下:\n姓名：%s\n简拼：%s\n全拼：%s\n公司电话：%s\n私人电话：%s\n分机号：%s\nEmail：%s\n",
				p->myname,
				p->abbreviation,
				p->full,
				p->company,
				p->privation,
				p->extension,
				p->emall
				);
		p += sizeof(INFOR);
	};
	return 0;    
}

/*
 *功能：读取socket里的内容
 *参数：1.char*buf：输出参数，存放读取到的内容
 *      2.int buf_len：buf的字节数
 *返回值：读取到的实际的字节数
 */
int read_ser(char* buf, int buf_len)//读数据的函数；
{
	int ret	= -1;
	int len	= 0;
	QA_HEAD*head = (QA_HEAD*) buf;

	do{
		ret = Socket_Read(sockfd, buf+len, buf_len - len - 1);
		if( ret < 0 ) {
			terminal_print("socket读错误\n");
			return -1;
		}

		len += ret;

		if ( len == sizeof(buf) - 1) 
		{
			terminal_print("socket返回包太长\n");
			return -2;
		}
	}while( ret != 0 );
	if(len == 0){
		terminal_print("server end\n");
		return -3;
	}
	if(len < (int)sizeof(QA_HEAD)){
		terminal_print("socket返回包太短\n");
		return -4;
	}

	if(len != head->package_len) {
		terminal_print("返回包长度错误\n");
		return -5;
	}

	return len;
}

/*
 *功能：socket写数据功能函数
 *参数：1.char*buf：所要写入的数据
 *      2.int buf_len：数据的长度
 *返回值：>0--写入的字节数，<0--调用出错
 */
int write_ser(char* buf, int buf_len)//写数据的函数；
{   
	int len = 0;

	if(buf == NULL){
		terminal_print("write_ser buf is NULL\n");
		return -1;		
	}
	if(buf_len != QR_BODY_LEN + sizeof(QR_HEAD)){
		terminal_print("write_ser buf_len error\n");
		return -2;
	}

	while(1) {
		int ret = -1;
		ret = Socket_Write(sockfd, buf + len, buf_len - len);//把标准输入的数据写入到套接字里面；
		if(ret < 0) {
			terminal_print("write_ser write error\n");
			return -3;
		}

		if(ret == buf_len) {
			break;
		}

		len += ret;
		if(len == buf_len) {
			break;
		}
	}
	return len ;
}

/*
 *功能：封装包函数
 *参数：1.char*buf：输入输出参数，待填充完整数据的包
 *      2.int buf_len：buf的长度
 *返回值：0--封包成功，<0--失败
 */
int	build_package(char* buf, int buf_len)
{
	if(buf == NULL){
		terminal_print("build_package buf is NULL\n");
		return -1;	
	}

	if(buf_len != QR_BODY_LEN + sizeof(QR_HEAD)) {
		terminal_print("build_pack buf_len error\n");
		return -2;
	}

	QR_PACK* pack = (QR_PACK*)buf;

	pack->package_len = QR_PACK_LEN;//定长包；
	pack->package_id = QR_ID;//协议类型；	

	return 0;
}

/*
 *功能：读取终端要输入的信息
 *参数：1.char*data：输出参数，保存终端数据
 *      2.int len：data的长度
 *返回值：>0--成功，<0--失败,0--收到退出命令
 */
int read_stdin(char*data, int len)
{
	int ret = -1;

	terminal_print("查找支持：中文名、简拼、全拼、公司手机号、私人号码、分机号、邮箱！\n退出系统请输入：quit\n");
	terminal_print("input message:");
	fflush(stdout);
	fflush(stdin);

	ret = read(0, data, len-1);//从标准输入中读取数据；
	if(ret < 0){   
		terminal_print("read_stdin read error\n");
		return ret;
	}

	if(strncmp(data, "quit", sizeof("quit")-1)==0){
		terminal_print("quit 退出命令\n");
		return 0;
	}
	fflush(stdout);
	// memcpy(data, "lgj", sizeof("lgj"));
	// sleep(1);
	return ret;
}

/*
 *功能：读取终端数据逻辑函数
 *参数：1.char*buf：输入参数，用于保存接受的数据
 *      2.int buf_size：buf的长度
 *返回值：>0--实际读取到的长度，<0--失败，0--收到退出命令
 */
int read_input(char* buf, int buf_size)
{
	int ret = -1;

	ret = read_stdin(buf, buf_size);
	if(ret < 0){
		return -1;
	}

	return ret;
}

/*
 *功能：初始化tcp，建立socket连接
 *参数：无
 *返回值：0--成功，<0--失败
 */
int tcp_init()
{
	int ret = -1;

	sockfd = Socket_Create(AF_INET, SOCK_STREAM, 0);
	if (sockfd < -1){
		terminal_print("socket error\n");
		return -1;
	}	
	//IPv4地址族结构；
	struct sockaddr_in seraddr;
	memset (&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(port);
	seraddr.sin_addr.s_addr=inet_addr(ip);


	ret = connect(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr));//连接；
	if(ret < 0){
		terminal_print("connect error\n");
		return -2;
	}
	terminal_print("与服务器连接成功！\n");

	return 0;
}

/*
 *功能：程序的主循环
 *参数：wu
 *返回值：0--正常退出，<0--遇到错误
 */
int main_loop()
{
	char w_buf[sizeof(QR_PACK)] = {0};
	char r_buf[MAX_LEN]	= {0};

	while(1){
		int len = 0 ;
		int ret = -1 ;

		ret = tcp_init();
		if(ret < 0){
			tcp_close();
			return -1;
		}

		ret = read_input(w_buf + sizeof(QR_HEAD), QR_BODY_LEN);
		if(ret < 0){
			return -2;	
		}
		if(ret == 0){
			return 0;
		}

		ret = build_package(w_buf, sizeof(w_buf));
		if(ret < 0){
			return -3;
		}

		ret = write_ser(w_buf, sizeof(w_buf));
		if(ret < 0){
			return -4;
		}

		len = read_ser(r_buf, sizeof(r_buf));
		if(len < 0){
			return -5;
		}

		ret = unpackage(r_buf, len);	
		if(ret < 0){
			return -6;	
		}			

		ret = tcp_close();
		if(ret < 0){
			return -7;
		}

		memset(w_buf, 0, sizeof(w_buf));
		memset(r_buf, 0, sizeof(r_buf));
	}	

	return 0;		
}

/*
 *功能：程序入口函数
 *参数：1.int argc：参数个数
 *      2.char*argv[]:接收到的参数
 *返回值：0--正常退出，<0--异常退出
 */
int main(int argc, char* argv[])
{

#ifdef _MINGW_
	setlocale(LC_ALL,"chs");
#else
	setlocale(LC_ALL,"");
#endif

	int ret = -1;

	if(argc<3){
		printf("Usage:%s option\n",argv[0]);
		printf("for example:%s ip port \n",argv[0]);
		return -1;
	}
	if(strlen(argv[1]) > sizeof(ip)){
		terminal_print("IP length error\n");
		return -2;
	}	

	strcpy(ip, argv[1]);
	port = atoi(argv[2]);

	ret = main_loop();
	if(ret < 0){
		return -3;
	}
	if(ret == 0){
		return 0;
	}

	return 0;
}

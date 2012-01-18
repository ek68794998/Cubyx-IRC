#pragma once

#pragma comment(lib, "wsock32.lib")

#include <stdio.h>
#include <winsock.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string>
#include <time.h>
#include <ctype.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;
using std::vector;

#pragma once

#pragma warning(push)
#pragma warning(disable: 4005)
	#define WIN32_LEAN_AND_MEAN
	#define _CRT_SECURE_NO_WARNINGS

	#define BI_SOCKBUFFERSIZE 1024
	#define BI_SOCKCHUNKSIZE 1024
#pragma warning(pop)

typedef unsigned char byte;

enum {
	BI_ERROR_LINETOOLONG = 1000000
};

class TcpConnectionClass {
protected:
	friend class TcpConnectionListenerClass;

public:
	sockaddr_in Address;
	bool IsBlocking;
	
	bool UpdateBlocking();

	SOCKET Socket;
	int Error;
	
	TcpConnectionClass();
	~TcpConnectionClass();

	bool Open();
	bool SetBlocking(bool _IsBlocking);
	bool SetAddress(const char* HostName, int Port);
	bool SetAddress(in_addr LongIp, int Port);
	void SetPort(int Port);
	int Connect();
	bool Send(const char* Data, int DataLen);
	int Receive(char* Data, int DataLen, int Flags = 0);
	bool Close();
	void ClearError();
	void SetError(int _Error);
	bool HasErrorOccurred();
	bool UpdateError();
};

class AsciiTcpConnectionClass : public TcpConnectionClass {
protected:
	friend class AsciiTcpConnectionListenerClass;

public:
	std::string ReceiveBuffer;
	bool ShiftLinefeed;

	AsciiTcpConnectionClass();
	bool SendLine(const char* Format, ...);
	int ReceiveLine(std::string& Line);
};

class TcpConnectionListenerClass : public TcpConnectionClass {
public:
	bool Bind();
	bool Listen();
	virtual int Accept(TcpConnectionClass*& AcceptedConnection);
};

class AsciiTcpConnectionListenerClass : public TcpConnectionListenerClass {
public:
	virtual int Accept(AsciiTcpConnectionClass*& AcceptedConnection);
};

class ConnectionManager {
public:
	static unsigned int ConnectionCount;

	static void Add(TcpConnectionClass& Connection);
	static void Remove(TcpConnectionClass& Connection);
};

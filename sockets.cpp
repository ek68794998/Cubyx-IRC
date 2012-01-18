// socket.cpp

#include "sockets.h"

#pragma comment(lib, "Ws2_32.lib")

class TcpConnectionClass;

TcpConnectionClass::TcpConnectionClass() {
	Socket = NULL;
	memset(&Address, 0, sizeof(Address));
	Address.sin_family = AF_INET;
	IsBlocking = true;
	Error = ERROR_SUCCESS;

	ConnectionManager::Add(*this);
}

TcpConnectionClass::~TcpConnectionClass() {
	Close();
	ConnectionManager::Remove(*this);
}

bool TcpConnectionClass::UpdateBlocking() {
	u_long Argument = !IsBlocking;
	if (ioctlsocket(Socket, FIONBIO, &Argument) == SOCKET_ERROR) { return UpdateError(); }

	return true;
}

bool TcpConnectionClass::Open() {
	// Close socket if still opened and clear the error.
	Close();
	ClearError();

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == SOCKET_ERROR) { return UpdateError(); }
	
	return UpdateBlocking();
}
bool TcpConnectionClass::SetBlocking(bool _IsBlocking) {
	IsBlocking = _IsBlocking;
	if (Socket) {
		return UpdateBlocking();
	}
	return true;
}
bool TcpConnectionClass::SetAddress(const char* HostName, int Port) {
	hostent* HostData;
	HostData = gethostbyname(HostName);
	if (!HostData) { return UpdateError(); }
	
	Address.sin_port = htons(Port);
	Address.sin_addr = *(in_addr*)HostData->h_addr;

	return true;
}
bool TcpConnectionClass::SetAddress(in_addr LongIp, int Port) {		
	Address.sin_port = htons(Port);
	Address.sin_addr = LongIp;

	return true;
}
void TcpConnectionClass::SetPort(int Port) {
	Address.sin_port = htons(Port);
}
int TcpConnectionClass::Connect() {
	if (connect(Socket,(sockaddr*)&Address,sizeof(sockaddr)) == SOCKET_ERROR) {
		int Err = WSAGetLastError();
		if (Err == WSAEALREADY) { return 400; }
		else if (Err == WSAEINVAL) { return 401; }
		else if (Err == WSAEWOULDBLOCK) { return 402; }
		else if (Err == WSAEISCONN) { return 1; }
		else { UpdateError(); return -1; }
	}

	return 1;
}
bool TcpConnectionClass::Send(const char* Data, int DataLen) {
	if (send(Socket,Data,DataLen, 0) != DataLen) return UpdateError();
	return true;
}
int TcpConnectionClass::Receive(char* Data, int DataLen, int Flags) {
	int ReceiveResult = recv(Socket,Data,DataLen,Flags);
	if (ReceiveResult == 0) {
		return -1;
	}
	if (ReceiveResult == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return 0;
		} else {
			UpdateError();
			return -1;
		}
	}
	return ReceiveResult;
}
bool TcpConnectionClass::Close() {
	if (Socket) {
		if (closesocket(Socket) == SOCKET_ERROR) { return UpdateError(); }
		Socket = NULL;
	}
	return true;
}
void TcpConnectionClass::ClearError() {
	Error = ERROR_SUCCESS;
}
void TcpConnectionClass::SetError(int _Error) {
	Error = _Error;
	Close();
}
bool TcpConnectionClass::HasErrorOccurred() {
	return Error != ERROR_SUCCESS;
}
bool TcpConnectionClass::UpdateError() {
	if (!HasErrorOccurred()) {
		SetError(WSAGetLastError());
	}
	return false;
}

AsciiTcpConnectionClass::AsciiTcpConnectionClass() {
	ReceiveBuffer = "";
	ShiftLinefeed = false;
}
bool AsciiTcpConnectionClass::SendLine(const char* Format, ...) {
	char FormattedString[BI_SOCKBUFFERSIZE];

	va_list Arguments;
	va_start(Arguments, Format);
	int FormattedLength = vsnprintf(FormattedString, sizeof(FormattedString) - 3, Format, Arguments);
	va_end(Arguments);
	
	if (FormattedLength == -1)
	{
		SetError(BI_ERROR_LINETOOLONG);
		return false;
	}

	strcpy(FormattedString + FormattedLength, "\r\n");
	return Send(FormattedString, FormattedLength + 2);
}
int AsciiTcpConnectionClass::ReceiveLine(string& Line) {
	while (true) {
		if (ReceiveBuffer.length() > 0) {
			if (ShiftLinefeed) {
				if (ReceiveBuffer[0] == '\n') { ReceiveBuffer = ReceiveBuffer.substr(1); }
				ShiftLinefeed = false;
			}

			// Find newline
			string::size_type NewlinePos = ReceiveBuffer.find_first_of("\r\n");
			if (NewlinePos != string::npos) {
				// Copy to buffer
				Line = ReceiveBuffer.substr(0,NewlinePos);

				// Shift line off buffer
				if (ReceiveBuffer[NewlinePos] == '\r') { ShiftLinefeed = true; }
				ReceiveBuffer = ReceiveBuffer.substr(NewlinePos + 1);

				return 1;
			}
		}
		
		// Read chunk
		char Chunk[BI_SOCKCHUNKSIZE];
		int ChunkSize = Receive(Chunk,BI_SOCKCHUNKSIZE);
		if (ChunkSize == 0) { return 0; } // No data
		if (ChunkSize == -1) { return -1; } // Error

		ReceiveBuffer.append(Chunk, ChunkSize);
	}
}

bool TcpConnectionListenerClass::Bind() {
	if (bind(Socket, (sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR) { return UpdateError(); }

	return true;
}
bool TcpConnectionListenerClass::Listen() {
	if (listen(Socket, SOMAXCONN) == SOCKET_ERROR) { return UpdateError(); }

	return true;
}
int TcpConnectionListenerClass::Accept(TcpConnectionClass*& AcceptedConnection) {
	SOCKET RemoteSocket = accept(Socket, NULL, 0);
	if (RemoteSocket == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return 0;
		} else {
			UpdateError();
			return -1;
		}
	}

	AcceptedConnection = new TcpConnectionClass();
	AcceptedConnection->Socket = RemoteSocket;
	return 1;
}

int AsciiTcpConnectionListenerClass::Accept(AsciiTcpConnectionClass*& AcceptedConnection) {
	SOCKET RemoteSocket = accept(Socket, NULL, 0);
	if (RemoteSocket == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return 0;
		}
		else
		{
			UpdateError();
			return -1;
		}
	}

	AcceptedConnection = new AsciiTcpConnectionClass();
	AcceptedConnection->Socket = RemoteSocket;
	return 1;
}

unsigned int ConnectionManager::ConnectionCount = 0;

void ConnectionManager::Add(TcpConnectionClass& Connection) {
	if (ConnectionCount++ == 0) {
		WSADATA WsaData;
		WSAStartup(MAKEWORD(2, 0), &WsaData);
	}
}

void ConnectionManager::Remove(TcpConnectionClass& Connection) {
	if (--ConnectionCount == 0) {
		WSACleanup();
	}
}

#include "irc.h"

IRC::IRC() {
}

IRC::~IRC() {
}

void IRC::Init() {
	InitConnection();
}

void IRC::Connect() {
	UpdateState(CONNECTING);
}

void IRC::Disconnect(wxString& QuitMsg, bool UseMsg) {
	if (UseMsg) {
		if (IRCState == CONNECTED || IRCState == INSESSION) {
			if (!QuitMsg.Cmp("")) {
				QuitMsg = wxString::Format("%s IRC Client v%s",xAPPNAME,xAPPVER);
			}
			Throw("QUIT :%s",QuitMsg.c_str());
		}
	}

	Sock.Close();

	InitConnection();
	UpdateState(DISCONNECTED);
}

void IRC::InitConnection() {
	Sock.SetBlocking(false);
	Sock.SetAddress(ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_Address.c_str(),ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_Port);
	Sock.Open();
}

void IRC::TryConnect() {
	int Success = Sock.Connect();
	if (Success == -1) {
		UpdateState(DISCONNECTED);
	} else if (Success == 1) {
		UpdateState(CONNECTED);
	} else {
		// Some dumb error... <.<
	}
}

void IRC::UpdateState(State S) {
	IRCState = S;
	if (S == DISCONNECTED) {
		ServerManager->GetServerByID(m_info.m_ServerID)->ProcessEvent(SERVER_DISCONNECT);
	} else if (S == CONNECTING) {
		TryConnect();
	} else if (S == CONNECTED) {
		ServerManager->GetServerByID(m_info.m_ServerID)->ProcessEvent(SERVER_CONNECT);

		if (ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_Password.Len() > 0) {
			Throw("PASS %s",ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_Password.c_str());
		}
		Throw("NICK %s",ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_Nick.c_str());

		TCHAR szHostName[MAX_PATH];
		DWORD cbHostName = sizeof(szHostName);
		GetComputerName(szHostName,&cbHostName);

		Throw("USER %s %s %s :%s",ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_UserID.c_str(),szHostName,"server",ServerManager->GetServerByID(m_info.m_ServerID)->m_Data.m_FullName.c_str());

		// Set Window Title for Server
	} else if (S == INSESSION) {
	}
}

bool IRC::Think() {
	if (IRCState == DISABLED) {
		// Nothing!
	} else if (IRCState == DISCONNECTED) {
		// Reconnect?
	} else if (IRCState == CONNECTING) {
		TryConnect();
	} else if (IRCState == CONNECTED || IRCState == INSESSION) {
		std::string buf;
		int Success = Sock.ReceiveLine(buf);
		if (Success == -1) { Disconnect(); }
		else if (Success != 0) { HandleMessage(buf); }
	}
	return false;
}

void IRC::HandleMessage(const std::string& msg) {
	ServerManager->GetServerByID(m_info.m_ServerID)->ProcessDataFromRaw(wxString(msg.c_str()));
}

void IRC::Throw(const char *in, ...) {
	if (IRCState != CONNECTED && IRCState != INSESSION) { return; }
	char b[510];
	va_list args;
	va_start(args,in);
	vsnprintf(b,sizeof(b),in,args);
	va_end(args);
	char m[512];
	int len = sprintf(m,"%s\r\n",b);
	if (!Sock.Send(m,len)) { Disconnect(wxString(""),false); }
}

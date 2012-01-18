#include "mod_server.h"

cServerManager* ServerManager;

void cServerManager::StartServerManager() {
	ServerManager = new cServerManager();
}
int cServerManager::StartNewServer(const char *Addr, int Port) {
	cServer *nServer = new cServer(wxString(Addr),Port);

	INI* Config = wxGetApp().xManager->Settings.m_Profile;
	nServer->m_Data.m_Nick = Config->GetString("Config","Nick","CUBYX");
	nServer->m_Data.m_UserID = Config->GetString("Config","UserID","CUBYX");
	nServer->m_Data.m_FullName = Config->GetString("Config","FullName","CUBYX");
	nServer->m_Data.m_Password = wxEmptyString;

	nServer->m_ServerID = int(m_Servers.size());
	m_Servers.push_back(nServer);
	nServer->m_Session = new IRC();

	IRCInfo info;
	info.m_ServerID = nServer->m_ServerID;
	info.bIdentServer = true;
	info.iIdentServerPort = 113;
	info.sIdentServerType = "UNIX";
	nServer->m_Session->SetInfo(info);

	nServer->Init();
	return nServer->m_ServerID - 1;
}
cServer* cServerManager::GetServerByID(int ID) {
	return m_Servers[ID];
}

cServer::cServer(wxString& Addr, int Port) {
	m_Data = cServerData();
	m_Data.m_NetworkName = Addr;
	m_Data.m_Address = Addr;
	m_Data.m_Port = Port;
}
cChannel* cServer::AddChannel(const char *Name) {
	cChannel *nChan = new cChannel(m_ServerID,Name);
	nChan->Init();
	return nChan;
}
cChannel* cServer::GetChannel(wxString& Name) {
	for (unsigned int i = 0; i < m_Channels.size(); i++) {
		if (!m_Channels[i]) { continue; }
		wxString& Criterion = wxString(Name);
		wxString& ToCheck = wxString(m_Channels[i]->m_ChannelName);
		Criterion.Replace("#","");
		ToCheck.Replace("#","");
		if (!ToCheck.CmpNoCase(Criterion)) {
			return m_Channels[i];
		}
	}
	return NULL;
}
void cServer::Init() {
	m_StatusWindowID = GUI->AddNewFrame(FRAME_SERVER,m_ServerID,wxString("~Status"));
}
void cServer::Output(const char* Str) {
	((cServerWindow *)(wxMDIChildFrame *)(wxWindow::FindWindowById(m_StatusWindowID)))->m_TextDisplay->AppendFormattedText(wxString(Str));
}
void cServer::OutputDebug(const char* Str) {
#ifdef _DEBUG
	Output(Str);
#endif
}
void cServer::ProcessDataFromRaw(wxString& Str) {
	wxString &Nick = Str.AfterFirst(':').BeforeFirst('!');
	wxString &Msg = Str.AfterFirst(' ');
	wxString &Cmd = Msg.BeforeFirst(' ');
	if (Cmd.IsNumber()) {
		wxString NMsg = wxString(Msg.AfterFirst(' '));
		if (!NMsg.BeforeFirst(' ').CmpNoCase(m_Data.m_Nick.c_str())) { NMsg = NMsg.AfterFirst(' '); }
		ProcessEventNumeric(atoi(Cmd.c_str()),NMsg);
	} else if (!Str.BeforeFirst(' ').CmpNoCase("ping")) {
		if (!Msg.AfterFirst(':').CompareTo(m_Data.m_Address.c_str(),wxString::ignoreCase)) {
			Raw("PONG %s",Msg.AfterFirst(' ').c_str());
		}
	} else if (!Cmd.CmpNoCase("join")) {
		if (!Nick.CmpNoCase(m_Data.m_Nick.c_str())) {
			cChannel *Chan = AddChannel(Msg.AfterFirst(':').c_str());
			if (Chan) { Chan->Join(); }
		}
	} else if (!Cmd.CmpNoCase("part")) {
		if (!Nick.CmpNoCase(m_Data.m_Nick.c_str())) {
			cChannel *Chan = GetChannel(Msg.AfterFirst(' '));
			if (Chan) { Chan->CloseWindow(); }
		}
	} else if (!Cmd.CmpNoCase("nick")) {
		if (!Nick.CmpNoCase(m_Data.m_Nick.c_str())) {
			wxString& NewNick = Msg.AfterFirst(' ');
			NewNick.Replace(":","",false);
			m_Data.m_Nick = wxString(NewNick);
		}
	} else if (!Cmd.CmpNoCase("privmsg")) {
		cChannel *Chan = GetChannel(Msg.AfterFirst(' ').BeforeFirst(' '));
		if (Chan) {
			Chan->ProcessMessage(Nick,Msg.AfterFirst(' ').AfterFirst(' '));
		}
	} else {
		Output(Str);
	}
}
void cServer::ProcessEvent(SERVEREVENT Event) {
	// Process an event sent by the server
}
void cServer::ProcessEventNumeric(int N, wxString& Str) {
	if (N <= 0) { return; }
	if (Str.Left(1) == ":") { Str = Str.AfterFirst(':'); }
	if (N == RPL_WELCOME) { /* NOOP */ }
	else if (N == ERR_ERRONEUSNICKNAME) { Output(wxString::Format("%c NICK: Invalid nickname.",0x95)); }
	else if (N == RPL_MYINFO) { m_Data.m_NetworkName = wxString(Str.BeforeFirst(' ')); UpdateTitle(); }
	else if (N == RPL_MOTD) { Output(wxString::Format("%c MOTD: %s",0x95,Str.c_str())); }
	else if (N == RPL_ENDOFMOTD || N == ERR_NOMOTD) { m_Session->UpdateState(INSESSION); }
	else { OutputDebug(wxString::Format("%c Raw %d: %s",0x95,N,Str.c_str())); }
}
bool cServer::ProcessInput(wxString& Input) {
	// Input Processing
	if (Input.Len() <= 0) { return false; }
	if (Input.Left(1) == wxGetApp().xManager->Settings.CommandPre) {
		wxString &Msg = Input.AfterFirst(' ');
		wxString &Cmd = Input.BeforeFirst(' ').AfterFirst(wxGetApp().xManager->Settings.CommandPre.GetChar(0));
		if (!Cmd.CmpNoCase("connect")) {
			wxString Network = Msg.BeforeFirst(' ');
			if (m_Session->IRCState == CONNECTED || m_Session->IRCState == INSESSION) { m_Session->Disconnect(); }
			m_Data.m_Address = wxString(Network);
			m_Session->Init();
			m_Session->Connect();
			return true;
		} else if (!Cmd.CmpNoCase("join")) {
			wxString Chan = Msg.BeforeFirst(' ');
			if (Chan.Left(1) != "#") { Chan.Prepend("#"); }
			Raw("JOIN %s",Chan.c_str());
			return true;
		} else if (!Cmd.CmpNoCase("part")) {
			wxString Chan = Msg.BeforeFirst(' ');
			wxString Reason = Msg.AfterFirst(' ');
			if (Chan.Left(1) != "#") { Chan.Prepend("#"); }
			if (Reason.Len() <= 0 || Reason == wxEmptyString) { Raw("PART %s",Chan.c_str()); }
			else { Raw("PART %s :%s",Chan.c_str(),Reason.c_str()); }
			return true;
		} else if (!Cmd.CmpNoCase("nick")) {
			wxString Nick = Msg.BeforeFirst(' ');
			if (Nick.Left(1) != ":") { Nick.Prepend(":"); }
			Raw("NICK %s",Nick.c_str());
			return true;
		} else if (!Cmd.CmpNoCase("quit")) {
			if (Msg.Len() <= 0 || Msg == wxEmptyString) { Raw("QUIT"); }
			else { Raw("QUIT :%s",Msg.c_str()); }
			return true;
		} else {
			Output(wxString::Format("[ERROR] Unknown command \"%s\".",Cmd.Upper().c_str()));
		}
	} else {
		Output(wxString::Format("[ERROR] You cannot send text to the server. If you wish to use a command, prefix it with %s.",wxGetApp().xManager->Settings.CommandPre));
	}
	return false;
}
void cServer::Raw(const char *in, ...) {
	char b[512];
	va_list args;
	va_start(args,in);
	vsnprintf(b,sizeof(b),in,args);
	va_end(args);
	m_Session->Throw((const char *)b);
}
bool cServer::RemoveChannel(int j) {
	if (m_Channels[j]) {
		delete m_Channels[j];
		m_Channels.erase(m_Channels.begin() + j);
		return true;
	}
	return false;
}
void cServer::Terminate(const char *msg) {
	if (!m_Session) { return; }
	m_Session->Disconnect(wxString(msg));
}
void cServer::UpdateTitle() {
	((cServerWindow *)(wxMDIChildFrame *)(wxWindow::FindWindowById(m_StatusWindowID)))->SetTitle(wxString::Format("Server Window - %s",m_Data.m_NetworkName));
}

BEGIN_EVENT_TABLE(cServerWindow, wxFrame)
	EVT_SET_FOCUS(cServerWindow::OnFocus)
	EVT_SIZE(cServerWindow::OnResize)
	EVT_CLOSE(cServerWindow::OnClose)
END_EVENT_TABLE()
cServerWindow::cServerWindow(wxMDIParentFrame *Parent, wxWindowID ID, const wxString& title, const wxPoint& pos, const wxSize& size) : wxMDIChildFrame(Parent,ID,title,pos,size,wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT) {
}
void cServerWindow::OnFocus(wxFocusEvent& e) {
	if (wxGetApp().IsMainLoopRunning()) {
		if (!m_TextBox) { return; }
		m_TextBox->Navigate();
	}
}
void cServerWindow::OnResize(wxSizeEvent& e) {
	wxSize Size = GetClientSize();
	int X = 0;
	int Y = 0;
	int W = Size.GetWidth();
	int H = Size.GetHeight();

	m_TextDisplay->SetPosition(wxPoint(X,Y));
	m_TextDisplay->SetSize(wxSize(W,H-22)); // -30

	m_TextBox->SetPosition(wxPoint(X,Y+H-22));
	m_TextBox->SetSize(wxSize(W,22));
}
void cServerWindow::OnClose(wxCloseEvent& e) {
	int Quit = wxMessageDialog(NULL,"Are you sure you want to close this server window?","Close Server",wxYES_NO | wxCENTER).ShowModal();
	if (Quit == wxID_YES) {
		std::vector<cChannel*> Channels = ServerManager->GetServerByID(m_ServerID)->m_Channels;
		for (unsigned int i = 0; i < Channels.size(); i++) {
			if (!Channels[i]) { continue; }
			Channels[i]->CloseWindow(false);
		}
		ServerManager->GetServerByID(m_ServerID)->Terminate("");
		Destroy();
	}
}

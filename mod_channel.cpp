#include "mod_channel.h"

cChannel::cChannel(int Server, const char* Name) : m_Initialized(false) {
	m_ServerID = Server;
	m_ChannelName = wxString(Name);
	if (m_ChannelName.Left(1) != "#") { m_ChannelName = m_ChannelName.Prepend("#"); }
}
void cChannel::CloseWindow(bool Part) {
	if (Part == true) { ((cChannelWindow *)(wxMDIChildFrame *)(wxWindow::FindWindowById(m_ChannelWindowID)))->Close(); }
	else { ((cChannelWindow *)(wxMDIChildFrame *)(wxWindow::FindWindowById(m_ChannelWindowID)))->Destroy(); }
}
void cChannel::Init() {
	if (m_Initialized) { return; }
	m_ChannelWindowID = GUI->AddNewFrame(FRAME_CHANNEL,m_ServerID,wxString(m_ChannelName));
	ServerManager->GetServerByID(m_ServerID)->m_Channels.push_back(this);
	m_Initialized = true;
}
void cChannel::Join() {
	Init();
}
void cChannel::Output(wxString& Str) {
	((cChannelWindow *)(wxMDIChildFrame *)(wxWindow::FindWindowById(m_ChannelWindowID)))->m_TextDisplay->AppendFormattedText(Str);
}
void cChannel::Part() {
	ServerManager->GetServerByID(m_ServerID)->Raw("PART %s",m_ChannelName.c_str());
}
bool cChannel::ProcessInput(wxString& Input) {
	if (Input.Left(1) != wxGetApp().xManager->Settings.CommandPre) {
		ServerManager->GetServerByID(m_ServerID)->Raw("PRIVMSG %s :%s",m_ChannelName.c_str(),Input.c_str());
		ProcessMessage(ServerManager->GetServerByID(m_ServerID)->m_Data.m_Nick,Input.Prepend(":"));
		return true;
	}
	return false;
}
void cChannel::ProcessMessage(wxString& Nick, wxString& Msg) {
	wxString& NMsg = wxString(Msg);
	if (NMsg.Left(1) == ":") { NMsg = NMsg.AfterFirst(':'); }
	Output(wxString::Format("%s: %s",Nick.c_str(),NMsg.c_str()));
}

BEGIN_EVENT_TABLE(cChannelWindow, wxFrame)
	EVT_SET_FOCUS(cChannelWindow::OnFocus)
	EVT_SIZE(cChannelWindow::OnResize)
	EVT_CLOSE(cChannelWindow::OnClose)
	EVT_KEY_DOWN(cChannelWindow::OnKey)
END_EVENT_TABLE()
cChannelWindow::cChannelWindow(wxMDIParentFrame *Parent, wxWindowID ID, const wxString& title, const wxPoint& pos, const wxSize& size) : wxMDIChildFrame(Parent,ID,title,pos,size,wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT) {
}
void cChannelWindow::OnClose(wxCloseEvent& e) {
	std::vector<cChannel*> Channels = ServerManager->GetServerByID(m_ServerID)->m_Channels;
	for (unsigned int i = 0; i < Channels.size(); i++) {
		if (!Channels[i]) { continue; }
		if (!Channels[i]->m_ChannelName.CmpNoCase(m_ChannelName)) {
			Channels[i]->Part();
			ServerManager->GetServerByID(m_ServerID)->RemoveChannel(i);
			break;
		}
	}
	Destroy();
}
void cChannelWindow::OnFocus(wxFocusEvent& e) {
	if (wxGetApp().IsMainLoopRunning()) {
		if (!m_TextBox) { return; }
		m_TextBox->Navigate();
	}
}
void cChannelWindow::OnKey(wxKeyEvent& e) {
	if (wxGetApp().IsMainLoopRunning()) {
	}
}
void cChannelWindow::OnResize(wxSizeEvent& e) {
	wxSize Size = GetClientSize();
	int X = 0;
	int Y = 0;
	int W = Size.GetWidth();
	int H = Size.GetHeight();

	m_TextDisplay->SetPosition(wxPoint(X,Y));
	m_TextDisplay->SetSize(wxSize(W,H-22-30));

	m_TextBox->SetPosition(wxPoint(X,Y+H-22));
	m_TextBox->SetSize(wxSize(W,22));
}

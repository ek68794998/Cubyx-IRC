#pragma once

class cServerManager;
class cServer;

enum SERVEREVENT {
	SERVER_CONNECT,
	SERVER_DISCONNECT
};

extern cServerManager* ServerManager;

#include "main.h"
#include "gui.h"
#include "irc.h"
#include "mod_channel.h"

struct cServerData {
	cServerData() { };

	wxString m_Address;
	wxString m_NetworkName;
	int m_Port;
	wxString m_Nick;
	wxString m_UserID;
	wxString m_FullName;
	wxString m_Password;
};

class cServerManager {
public:
	cServerManager() {
		m_NextWindowID = 0;
	};

	static void StartServerManager();

	int StartNewServer(const char *Addr, int Port);
	cServer* GetServerByID(int ID);

	std::vector<cServer*> m_Servers;
	int m_NextWindowID;
};

class cServer {
public:
	cServer(wxString& Addr, int Port);

	cChannel* AddChannel(const char *Name);
	cChannel* GetChannel(wxString& Name);
	void Init();
	void Output(const char* Str);
	void OutputDebug(const char* Str);
	void ProcessDataFromRaw(wxString& Input);
	void ProcessEvent(SERVEREVENT Event);
	void ProcessEventNumeric(int N, wxString& Str);
	bool ProcessInput(wxString& Input);
	void Raw(const char *in, ...);
	bool RemoveChannel(int j);
	void Terminate(const char *msg);
	void UpdateTitle();

	IRC* m_Session;
	int m_ServerID;
	cServerData m_Data;
	std::vector<cChannel*> m_Channels;
	int m_StatusWindowID;
};

class cServerWindow : public wxMDIChildFrame {
public:
	cServerWindow(wxMDIParentFrame *Parent, wxWindowID ID, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	
	void OnFocus(wxFocusEvent& e);
	void OnResize(wxSizeEvent& e);
	void OnClose(wxCloseEvent& e);
	
	int m_ServerID;
	cWindowInputCtrl* m_TextBox;
	cWindowOutputCtrl* m_TextDisplay;

private:
	DECLARE_EVENT_TABLE();
};

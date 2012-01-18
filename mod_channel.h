#pragma once

class cChannel;

#include "main.h"
#include "mod_server.h"

class cChannel {
public:
	cChannel(int Server, const char* Name);

	void CloseWindow(bool Part = true);
	void Init();
	void Join();
	void Output(wxString& Str);
	void Part();
	bool ProcessInput(wxString& Input);
	void ProcessMessage(wxString& Nick, wxString& Msg);

	bool m_Initialized;
	int m_ServerID;
	wxString m_ChannelName;
	int m_ChannelWindowID;
};

class cChannelWindow : public wxMDIChildFrame {
public:
	cChannelWindow(wxMDIParentFrame *Parent, wxWindowID ID, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	
	void OnClose(wxCloseEvent& e);
	void OnFocus(wxFocusEvent& e);
	void OnKey(wxKeyEvent& e);
	void OnResize(wxSizeEvent& e);
	
	int m_ServerID;
	wxString m_ChannelName;
	cWindowInputCtrl* m_TextBox;
	cWindowOutputCtrl* m_TextDisplay;

private:
	DECLARE_EVENT_TABLE();
};

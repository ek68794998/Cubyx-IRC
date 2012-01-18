#pragma once

class cGUI;
class cWindowInputCtrl;
class cWindowOutputCtrl;
class cColorDialog;

enum FRAMETYPE {
	FRAME_SERVER,
	FRAME_CHANNEL,
	FRAME_QUERY
};

enum ChannelWindowStyle {
	STYLE_NONE = 0,
	STYLE_BOLD = 1,
	STYLE_UNDERLINE = 2,
	STYLE_ITALIC = 4
};

enum ChannelWindowColor {
	COLOR_WHITE = 0,
	COLOR_BLACK,
	COLOR_NAVY,
	COLOR_GREEN,
	COLOR_RED,
	COLOR_MAROON,
	COLOR_PURPLE,
	COLOR_ORANGE,
	COLOR_YELLOW,
	COLOR_LIME,
	COLOR_TEAL,
	COLOR_CYAN,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_LGREY,
	COLOR_DGREY
};

extern cGUI* GUI;
extern char* ChannelWindowColorNames[16];

#include "main.h"

#define HtmlEntities(String) String.Replace("&","&amp;"); String.Replace("<","&lt;"); String.Replace(">","&lt;");
#define GetWxColorFromInt(Col) wxColor((Col >> 16)&0xFF,(Col >> 8)&0xFF,(Col)&0xFF)
#define GetWxColorStringFromInt(Col) wxString::Format("%02X%02X%02X",(Col >> 16)&0xFF,(Col >> 8)&0xFF,(Col)&0xFF)
#define GetIntColorFromEnum(Col) wxGetApp().xManager->Settings.Colors[Col]

class cGUI {
public:
	cGUI();

	int AddNewFrame(FRAMETYPE Type, int ServerID, wxString& Target);

	wxFont* m_Font;

	wxMDIParentFrame *m_TopFrame;
	wxMDIClientWindow *m_FrameContainer;
};

class cWindowInputCtrl : public wxTextCtrl {
public:
	cWindowInputCtrl(wxWindow *Parent, wxWindowID ID, const wxString& value, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxRE_MULTILINE, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr);

	void OnEnter(wxCommandEvent& e);
	void OnKey(wxKeyEvent& e);

	wxString m_Target;
	int m_ServerID;
	int m_WindowID;
	FRAMETYPE m_WindowType;

private:
	DECLARE_EVENT_TABLE();
};

class cWindowOutputCtrl : public wxHtmlWindow {
public:
	//cWindowOutputCtrl(wxWindow *Parent, wxWindowID ID, const wxString& value, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxRE_MULTILINE, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr);
	cWindowOutputCtrl(wxWindow *Parent, wxWindowID ID, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHW_DEFAULT_STYLE, const wxString& name = wxTextCtrlNameStr);

	void AppendFormattedText(const wxString &Str, int style = 0, ChannelWindowColor color = COLOR_BLACK);
};

class cGenericButton : public wxButton {
public:
	cGenericButton(wxWindow* parent, wxWindowID id, const wxString& label = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = "button");

	void OnClick(wxCommandEvent& e);

	cColorDialog* m_ColorDialog;

private:
	DECLARE_EVENT_TABLE();
};

class cColorDialog : public wxDialog {
public:
	cColorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
	
	void Answer();
	void Ask();
	
	int m_ColorCode;
	int m_WindowID;
	FRAMETYPE m_WindowType;
};

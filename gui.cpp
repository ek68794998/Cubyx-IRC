#include "gui.h"

cGUI* GUI;
char* ChannelWindowColorNames[16] = {
	"White",
	"Black",
	"Navy",
	"Green",
	"Red",
	"Maroon",
	"Purple",
	"Orange",
	"Yellow",
	"Lime",
	"Teal",
	"Cyan",
	"Blue",
	"Magenta",
	"Light Grey",
	"Dark Grey"
};

cGUI::cGUI() {
	m_TopFrame = new wxMDIParentFrame(NULL,wxID_ANY,wxGetApp().xManager->m_Title,wxPoint(0,0),wxSize(800,500),wxDEFAULT_FRAME_STYLE | wxMAXIMIZE);
	m_TopFrame->Show();
	m_TopFrame->Maximize();
	m_FrameContainer = new wxMDIClientWindow(m_TopFrame);
}
int cGUI::AddNewFrame(FRAMETYPE Type, int ServerID, wxString& Target) {
	if (!m_TopFrame) { return -1; }
	int ID = ServerManager->m_NextWindowID + CONST_WINDOWIDBASE;
	ServerManager->m_NextWindowID++;
	const char *WinPrefix = "";
	wxSize Size = wxDefaultSize;

	m_Font = new wxFont(wxGetApp().xManager->Settings.FontSize,wxFONTFAMILY_ROMAN,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL);
	m_Font->SetFaceName(wxGetApp().xManager->Settings.FontName);

	wxMDIChildFrame *MDIFrame = NULL;

	if (Type == FRAME_SERVER) {
		WinPrefix = "Server Window";
		Size = wxGetApp().xManager->Settings.DefServWinSize;
		cServerWindow *Frame = new cServerWindow(m_TopFrame,ID,wxString(WinPrefix),m_TopFrame->GetClientAreaOrigin(),Size);
		MDIFrame = (wxMDIChildFrame *)Frame;

		Frame->m_ServerID = ServerID;

		Frame->m_TextBox = new cWindowInputCtrl(Frame,wxID_ANY,_(""),wxPoint(0,0),wxSize(0,0),wxTE_RICH);
		Frame->m_TextBox->SetDefaultStyle(wxTextAttr(wxColor(0,0,0),wxColor(255,255,255),*m_Font));
		Frame->m_TextBox->m_Target = wxString(Target);
		Frame->m_TextBox->m_ServerID = ServerID;
		Frame->m_TextBox->m_WindowID = Frame->GetId();
		Frame->m_TextBox->m_WindowType = Type;

		Frame->m_TextDisplay = new cWindowOutputCtrl(Frame,wxID_ANY,wxPoint(0,0),wxSize(0,0));
		/*Frame->m_TextDisplay = new cWindowOutputCtrl(Frame,wxID_ANY,_(""),wxPoint(0,0),wxSize(0,0),wxRE_READONLY | wxRE_MULTILINE);
		Frame->m_TextDisplay->BeginSuppressUndo();
		Frame->m_TextDisplay->Freeze();
		Frame->m_TextDisplay->SetDefaultStyle(wxTextAttr(wxColor(0,0,0),wxColor(255,255,255),*m_Font));
		Frame->m_TextDisplay->SetStyle(0,Frame->m_TextDisplay->GetLastPosition(),Frame->m_TextDisplay->GetDefaultStyleEx());
		Frame->m_TextDisplay->Thaw();*/
	} else if (Type == FRAME_CHANNEL) {
		WinPrefix = "Channel Window";
		Size = wxGetApp().xManager->Settings.DefChanWinSize;

		wxWindowList& Children = m_TopFrame->GetChildren();
		int X = m_TopFrame->GetClientAreaOrigin().x + 20;
		int Y = m_TopFrame->GetClientAreaOrigin().y + 20;
		for (wxWindowList::iterator i = Children.begin(); i != Children.end(); ++i) {
			wxWindow* obj = *i;
			if (obj->GetPosition().x == X && obj->GetPosition().y == Y) {
				X += 20;
				Y += 20;
				i = Children.begin();
				if (X + Size.GetWidth() > m_TopFrame->GetClientSize().GetWidth() ||
					Y + Size.GetHeight() > m_TopFrame->GetClientSize().GetHeight()) {
					X = m_TopFrame->GetClientAreaOrigin().x + 20;
					Y = m_TopFrame->GetClientAreaOrigin().y + 20;
					break;
				}
			}
		}

		cChannelWindow *Frame = new cChannelWindow(m_TopFrame,ID,wxString::Format("%s - %s",WinPrefix,Target.c_str()),m_TopFrame->GetClientAreaOrigin()+wxPoint(X,Y),Size);
		MDIFrame = (wxMDIChildFrame *)Frame;

		Frame->m_ServerID = ServerID;
		Frame->m_ChannelName = wxString(Target);

		Frame->m_TextBox = new cWindowInputCtrl(Frame,wxID_ANY,_(""),wxPoint(0,0),wxSize(0,0),wxTE_RICH);
		Frame->m_TextBox->SetDefaultStyle(wxTextAttr(wxColor(0,0,0),wxColor(255,255,255),*m_Font));
		Frame->m_TextBox->m_Target = wxString(Target);
		Frame->m_TextBox->m_ServerID = ServerID;
		Frame->m_TextBox->m_WindowID = Frame->GetId();
		Frame->m_TextBox->m_WindowType = Type;

		Frame->m_TextDisplay = new cWindowOutputCtrl(Frame,wxID_ANY,wxPoint(0,0),wxSize(0,0));
	} else if (Type == FRAME_QUERY) {
		WinPrefix = "Query Window";
		Size = wxGetApp().xManager->Settings.DefQuerWinSize;
		wxMDIChildFrame *Frame = new wxMDIChildFrame(m_TopFrame,ID,wxString::Format("%s - %s",WinPrefix,"Stuff"),m_TopFrame->GetClientAreaOrigin(),Size);
	}

	MDIFrame->SetMinSize(MDIFrame->GetBestSize());
	MDIFrame->SendSizeEvent();
	MDIFrame->Show();
	return ID;
}

BEGIN_EVENT_TABLE(cWindowInputCtrl, wxTextCtrl)
	EVT_TEXT_ENTER(wxID_ANY,cWindowInputCtrl::OnEnter)
	EVT_KEY_DOWN(cWindowInputCtrl::OnKey)
END_EVENT_TABLE()
cWindowInputCtrl::cWindowInputCtrl(wxWindow *Parent, wxWindowID ID, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxTextCtrl(Parent,ID,value,pos,size,style,validator,name) {
}
void cWindowInputCtrl::OnEnter(wxCommandEvent& e) {
	if (GetValue().Len() <= 0) { return; }
	if (m_Target.Left(1) == "#") {
		cChannel* Chan = ServerManager->GetServerByID(m_ServerID)->GetChannel(m_Target);
		if (Chan && Chan->ProcessInput(GetValue())) {
			Clear();
			return;
		}
	}
	if (ServerManager->GetServerByID(m_ServerID)->ProcessInput(GetValue())) {
		Clear();
	}
}
void cWindowInputCtrl::OnKey(wxKeyEvent& e) {
	if (e.GetModifiers() == wxMOD_CONTROL) {
		if (e.GetKeyCode() == 0x4B) { // K key
			cColorDialog *Dialog = new cColorDialog(wxGetApp().GetTopWindow(),wxID_ANY,wxString("Pick a Color"),wxDefaultPosition,wxSize(300,300));
			Dialog->m_WindowID = m_WindowID;
			Dialog->m_WindowType = m_WindowType;
			Dialog->Ask();
			return;
		}
	}
	e.Skip();
}

//cWindowOutputCtrl::cWindowOutputCtrl(wxWindow *Parent, wxWindowID ID, const wxString& value, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxRichTextCtrl(Parent,ID,value,pos,size,style,validator,name) {
cWindowOutputCtrl::cWindowOutputCtrl(wxWindow *Parent, wxWindowID ID, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxHtmlWindow(Parent,ID,pos,size,style,name) {
	EnableScrolling(false,true);
	int FontSizes[7] = {6,8,10,12,14,16,18};
	SetFonts(GUI->m_Font->GetFaceName(),GUI->m_Font->GetFaceName(),FontSizes);
	AppendToPage("<font style='background-color: #333333'>ABC</font>");
}
void cWindowOutputCtrl::AppendFormattedText(const wxString &Str, int style, ChannelWindowColor color) {
	Freeze();
	wxString& String = wxString(Str);
	HtmlEntities(String);
	bool Underline = false;
	bool Bold = false;
	int BgColor = (int)COLOR_WHITE;
	int Color = (int)color;

	wxString& ps = wxString("");
	for (unsigned int i = 0; i < String.Len(); i++) {
		wxString& s = String.Mid(i,1);
		if (!s.Cmp(wxString::Format("%c",2))) {
			if (Bold) {
				Bold = false;
			} else {
				Bold = true;
			}
		} else if (!s.Cmp(wxString::Format("%c",3))) {
			wxString& Mid11 = String.Mid(i+1,1);
			wxString& Mid12 = String.Mid(i+1,2);
			wxString& Mid21 = String.Mid(i+2,1);
			wxString& Mid22 = String.Mid(i+2,2);
			wxString& Mid31 = String.Mid(i+3,1);
			wxString& Mid32 = String.Mid(i+3,2);
			wxString& Mid41 = String.Mid(i+4,1);
			wxString& Mid42 = String.Mid(i+4,2);
			if (Mid12.IsNumber() && atoi(Mid12) > 0 && atoi(Mid12) <= 15) {
				Color = atoi(Mid12);
				i += 2;
				if (Mid31 == ",") {
					if (Mid42.IsNumber() && atoi(Mid42) > 0 && atoi(Mid42) <= 15) {
						BgColor = atoi(Mid42);
						i += 3;
					} else if (Mid41.IsNumber() && (!Mid41.Cmp("0") || (atoi(Mid41) > 0 && atoi(Mid41) <= 9))) {
						BgColor = atoi(Mid41);
						i += 2;
					}
				}
			} else if (Mid11.IsNumber() && (!Mid11.Cmp("0") || (atoi(Mid11) > 0 && atoi(Mid11) <= 9))) {
				Color = atoi(Mid11);
				i += 1;
				if (Mid21 == ",") {
					if (Mid32.IsNumber() && atoi(Mid32) > 0 && atoi(Mid32) <= 15) {
						BgColor = atoi(Mid32);
						i += 3;
					} else if (Mid31.IsNumber() && (!Mid31.Cmp("0") || (atoi(Mid31) > 0 && atoi(Mid31) <= 9))) {
						BgColor = atoi(Mid31);
						i += 2;
					}
				}
			} else {
				Color = (int)color;
				BgColor = (int)COLOR_WHITE;
			}
			ps.Append(wxString::Format("</font><font color='#%s' bgcolor='#%s'>",GetWxColorStringFromInt(GetIntColorFromEnum(Color)).c_str(),GetWxColorStringFromInt(GetIntColorFromEnum(BgColor)).c_str()));
		} else if (!s.Cmp(wxString::Format("%c",15))) {
			Underline = false;
			Bold = false;
			int StyleToApply = style;
			if (StyleToApply >= STYLE_UNDERLINE) {
				StyleToApply -= STYLE_UNDERLINE;
				Underline = true;
			}
			if (StyleToApply >= STYLE_BOLD) {
				StyleToApply -= STYLE_BOLD;
				Bold = true;
			}
			Color = (int)color;
		} else if (!s.Cmp(wxString::Format("%c",31))) {
			if (Underline) {
				Underline = false;
			} else {
				Underline = true;
			}
		} else {
			if (Bold) { s.Prepend("<b>"); s.Append("</b>"); }
			if (Underline) { s.Prepend("<u>"); s.Append("</u>"); }
			ps.Append(s);
		}
	}
	AppendToPage(wxString::Format("<br /><font color='#%s' bgcolor='#%s'>%s</font>",GetWxColorStringFromInt(GetIntColorFromEnum((int)color)).c_str(),GetWxColorStringFromInt(GetIntColorFromEnum((int)COLOR_WHITE)).c_str(),ps.c_str()));
	Scroll(-1,INT_MAX);
	Thaw();
}

BEGIN_EVENT_TABLE(cGenericButton, wxButton)
	EVT_BUTTON(wxID_ANY,cGenericButton::OnClick)
END_EVENT_TABLE()
cGenericButton::cGenericButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxButton(parent,id,label,pos,size,style,validator,name) {
}
void cGenericButton::OnClick(wxCommandEvent& e) {
	m_ColorDialog->EndModal(GetId());
}

cColorDialog::cColorDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent,id,title,pos,size,style) {
	m_ColorCode = 0;
	for (int i = 0; i < 16; i++) {
		wxString& Label = (i < 10 ? wxString::Format("%s (&%d)",ChannelWindowColorNames[i],i) : wxString::Format("&%s (%d)",ChannelWindowColorNames[i],i));
		cGenericButton* Button = new cGenericButton(this,int(CONST_COLORBUTTONIDBASE)+i,Label,wxPoint((i <= 7 ? 10 : 150),10+(i % 8)*30),wxSize(130,30));
		Button->m_ColorDialog = this;
		Button->SetBackgroundColour(GetWxColorFromInt(wxGetApp().xManager->Settings.Colors[i]));
		Button->SetFont(wxFont(10,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD));
	}
	SetSize(GetBestSize()+wxSize(10,10));
}
void cColorDialog::Answer() {
	wxString& Str = wxString::Format("%c%s%d",3,((m_ColorCode - int(CONST_COLORBUTTONIDBASE)) >= 10 ? "" : "0"),(m_ColorCode - int(CONST_COLORBUTTONIDBASE)));
	wxMDIChildFrame* Frame = (wxMDIChildFrame *)(wxWindow::FindWindowById(m_WindowID));
	if (!Frame) { return; }
	if (m_WindowType == FRAME_SERVER) {
		((cServerWindow *)Frame)->m_TextBox->AppendText(Str);
	} else if (m_WindowType == FRAME_CHANNEL) {
		((cChannelWindow *)Frame)->m_TextBox->AppendText(Str);
	}
}
void cColorDialog::Ask() {
	m_ColorCode = ShowModal();
	Answer();
}

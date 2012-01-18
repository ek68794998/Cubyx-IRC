#include "app.h"

xApplicationManager::xApplicationManager() : m_Initialized(false) {
}

bool xApplicationManager::OnInitialization() {
	m_Title = wxString("Cubix IRC Client");

	Settings = xConfiguration();
	Settings.m_Profile = new INI("cubyx.ini");
	Settings.CommandPre = Settings.m_Profile->GetString("Config","CommandPrefix","/");
	for (unsigned int i = 0; i < _countof(Settings.Colors); i++) {
		char key[10];
		sprintf(key,"Color%d",i);
		wxString Value = Settings.m_Profile->GetString("Colors",key,"0,0,0");
		int R = atoi(Value.BeforeFirst(','));
		int G = atoi(Value.AfterFirst(',').BeforeLast(','));
		int B = atoi(Value.AfterLast(','));
		Settings.Colors[i] = (R * 0x10000) + (G * 0x100) + (B);
	}
	Settings.DefServWinSize = wxSize(Settings.m_Profile->GetInt("Windows","DefaultServWidth",700),Settings.m_Profile->GetInt("Windows","DefaultServHeight",450));
	Settings.DefChanWinSize = wxSize(Settings.m_Profile->GetInt("Windows","DefaultChanWidth",700),Settings.m_Profile->GetInt("Windows","DefaultChanHeight",450));
	Settings.DefQuerWinSize = wxSize(Settings.m_Profile->GetInt("Windows","DefaultQueryWidth",700),Settings.m_Profile->GetInt("Windows","DefaultQueryHeight",450));
	Settings.FontName = Settings.m_Profile->GetString("Format","FontName","Courier New");
	Settings.FontSize = Settings.m_Profile->GetInt("Format","FontSize",10);

	cServerManager::StartServerManager();
	GUI = new cGUI();

	ServerManager->StartNewServer(Settings.m_Profile->GetString("Config","Server","").c_str(),Settings.m_Profile->GetInt("Config","Port",6667));
	return true;
}
int xApplicationManager::OnExitApplication() {
	for (unsigned int i = 0; i < ServerManager->m_Servers.size(); i++) {
		if (!ServerManager->m_Servers[i]) { continue; }
		if (!ServerManager->m_Servers[i]->m_Session) { continue; }
		ServerManager->m_Servers[i]->Terminate("");
	}
	return 0;
}
void xApplicationManager::Think() {
	for (unsigned int i = 0; i < ServerManager->m_Servers.size(); i++) {
		if (!ServerManager->m_Servers[i]) { continue; }
		if (!ServerManager->m_Servers[i]->m_Session) { continue; }
		ServerManager->m_Servers[i]->m_Session->Think();
	}
}

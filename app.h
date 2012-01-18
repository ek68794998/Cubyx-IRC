#pragma once

class xApplicationManager;

enum CONSTANTS {
	CONST_WINDOWIDBASE = 13000,
	CONST_COLORBUTTONIDBASE = 14000,
	CONST_OTHERBUTTONIDBASE = 14020
};

#define xAPPNAME "Cubyx"
#define xAPPVER "1.0"

#include "main.h"
#include "profile.h"
#include "gui.h"
#include "mod_server.h"
#include "mod_channel.h"

struct xConfiguration {
	xConfiguration() {};

	INI* m_Profile;

	wxString CommandPre;

	unsigned long Colors[16];

	wxSize DefServWinSize;
	wxSize DefChanWinSize;
	wxSize DefQuerWinSize;

	wxString FontName;
	int FontSize;
};

class xApplicationManager {
public:
	xApplicationManager();

	bool OnInitialization();
	int OnExitApplication();
	void Think();

	bool m_Initialized;
	wxString m_Title;
	xConfiguration Settings;
};


#pragma once

#ifndef _WIN32_IE
	#define _WIN32_IE 0x0600
#endif
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string>
#include <time.h>
#include <ctype.h>
#include <list>
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;
using std::vector;

#include "wx/wx.h"
#include "wx/app.h"
#include <wx/clrpicker.h>
#include <wx/frame.h>
#include <wx/html/htmlwin.h>
#include <wx/html/winpars.h>
#include <wx/list.h>
#include <wx/mdi.h>
#include <wx/richtext/richtextctrl.h>

#define wxStrFormat wxString::Format

class wxAppHandler;

DECLARE_APP(wxAppHandler);

#include "app.h"

class wxAppHandler : public wxApp {
public:
	virtual bool OnInit();
	virtual int OnExit();
	void Think(wxIdleEvent& e);

	xApplicationManager* xManager;

	DECLARE_EVENT_TABLE();
};

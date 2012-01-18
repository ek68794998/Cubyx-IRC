#include "main.h"

IMPLEMENT_APP(wxAppHandler)

BEGIN_EVENT_TABLE(wxAppHandler, wxApp)
	EVT_IDLE(wxAppHandler::Think)
END_EVENT_TABLE()
bool wxAppHandler::OnInit() {
	xManager = new xApplicationManager();

	return xManager->OnInitialization();
}
int wxAppHandler::OnExit() {
	return xManager->OnExitApplication();
}
void wxAppHandler::Think(wxIdleEvent& e) {
	xManager->Think();
}

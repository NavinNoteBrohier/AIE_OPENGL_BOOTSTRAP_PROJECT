#include "PROJECTApp.h"

int main() {
	
	auto app = new PROJECTApp();
	app->run("AIE", app->GetWindowWidth(), app->GetWindowHeight(), false);
	delete app;

	return 0;
}
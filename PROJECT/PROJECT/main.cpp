#include "PROJECTApp.h"

int main() {
	
	auto app = new PROJECTApp();
	app->run("AIE", 1280, 720, false);
	delete app;

	return 0;
}
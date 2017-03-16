#include "PROJECTApp.h"

int main() {
	
	auto app = new PROJECTApp();
	app->run("AIE", 1920, 1080, false);
	delete app;

	return 0;
}
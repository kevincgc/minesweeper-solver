#include "game_application.h"

int main(int argc, char* argv[]) {
	auto app = game_application::create();
	
	const int status = app->run(argc, argv);
	return status;
}
#include <stdio.h>
#include "LedController.h"
int main(int argc, char** argv)
{
	CLedController* led = CLedController::instance();
	led->init();
	led->closeAll();
	return 0;
}


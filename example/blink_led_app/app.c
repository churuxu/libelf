#include "../api.h"


static void* dev_;
static int state_;

static int add1(int a, int b){return a+b;}

int add(int a, int b){return a+b;}


int app(const runtime_api* api){	
	api->println("[APP] app start");
	dev_ = api->device_get("LED0");
	if(!dev_){
		api->println("[APP] device LED0 not found");
		return 1;
	}
	while(1){
		state_ = !state_;
		api->println("[APP] tick");		
		api->gpio_set(dev_, state_);
		api->delay(500);
	}
	return 0;
}






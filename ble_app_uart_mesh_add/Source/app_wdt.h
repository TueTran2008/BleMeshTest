#ifndef  _APP_WDT_H_
#define   _APP_WDT_H_


#define WDT_PIN   NRF_GPIO_PIN_MAP(0, 20)



void app_wdt_init();

void app_wdt_feed();
#endif
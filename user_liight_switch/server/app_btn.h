 
#ifndef __INITSYSTEM_H__
#define __INITSYSTEM_H__
#include "simple_hal.h"

#define BUTTON_DEBOUND_WAIT				20
#define BUTTON_LONGPRESS				3000
#define BUTTON_SHORTHPRESS				500

#define NUMBER_BUTTON					3
#define	NUMBER_EVENT					3

typedef void(*Button_Event)(void);
typedef enum
{
	BUTTON_PRESSED_EVENT			= 	0,
	BUTTON_LONGPRESSED_EVENT		=	1,
	BUTTON_SHORTPRESSED_EVENT		=	2
}button_event;
typedef enum
{
	BUTTON_READ,
	BUTTON_WAIT_DEBOUND,
	BUTTON_WAIT_RELEASE_AND_CHECK_LONG_PRESS,
	BUTTON_WAIT_RELEASE
}Button_State;

typedef enum
{
	BUTTON_ACTIVE_HIGH,
	BUTTON_ACTIVE_LOW
}Button_active_t;
typedef struct
{
	//GPIO_TypeDef *gpio_port;
	uint16_t gpio_pin;
	uint8_t current_status;
	uint8_t last_status;
	uint32_t time_debounce;
	uint32_t t_long_press;
	Button_State button_state;
	uint8_t button_active;
	uint8_t button_id;
	int16_t t_accel_call;
	uint32_t t_accel_press;
}Button;
void Button_AddEvent(uint8_t button_id,button_event event,void (*func)());
void Button_ResetEvent(uint8_t button_id);
//void button_init(Button *btn,GPIO_TypeDef *port,uint16_t pin,uint8_t button_active,uint8_t button_id);
void button_check(Button *button);
uint32_t user_button_init(hal_button_handler_cb_t cb);
#endif
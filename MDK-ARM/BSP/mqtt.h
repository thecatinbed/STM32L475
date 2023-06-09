#ifndef __MQTT_H__
#define __MQTT_H__
#include "main.h"
#include "usart.h"
void MQTT_Connect(void);
void Connect(const char* send_string,const char* success_string,const char* fail_string);
void MQTT_Connect_new(void);
#endif

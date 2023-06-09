#include "mqtt.h"
#include <stdio.h>
#include <string.h>

const char c1[]="AT\r\n";
const char c2[]="AT+RST\r\n";
const char c3[]="AT+GMR\r\n";
const char c4[]="AT+CWMODE=1\r\n";
const char c5[]="AT+CWJAP=\"HONORM\",\"2576041061\"\r\n";
const char c6[]="AT+MQTTUSERCFG=0,1,\"stm32\",\"stm32\",\"1667749\",0,0,\"\"\r\n";
const char c7[]="AT+MQTTCONNCFG=0,60,1,\"theme\\stm32_conn_lost\",\"stm32 connection lost\",0,0\r\n";
const char c8[]="AT+MQTTCONN=0,\"192.168.43.171\",1883,0\r\n";
const char c9[]="AT+MQTTSUB=0,\"theme/stm32status\",1\r\n";
const char c10[]="AT+MQTTSUB=0,\"theme/weather\",1\r\n";
const char c11[]="AT+MQTTSUB=0,\"theme/door\",1\r\n";
const char c12[]="AT+MQTTSUB=0,\"theme/updatetime\",1\r\n";
const char c13[]="AT+MQTTSUB=0,\"theme/dbresult\",1\r\n";
int count=0;
extern uint8_t R2x_String[];    //接收字符串数组
extern uint8_t R2x_Flag;         //接收字符串标志

uint8_t ok1[2]="OK";
/**
  * @brief  函数功能简介
  * @param  参数1，功能，范围
  * @param  参数2，功能，范围
  * @param  参数3，功能，范围
  * @retval  返回值
  */
void Connect(const char* send_string,const char* success_string,const char* fail_string){
	int temp_count=0;
	char TxData[40];
	HAL_UART_Transmit(&huart2, (uint8_t *)send_string, strlen(send_string),0xFFFF); 
	while(1){
		temp_count++;
		if(R2x_Flag && temp_count<=50000000)
		{
			if(R2x_Flag){
				R2x_Flag=0;
				if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					 //sprintf(TxData,"main.t11.txt=\"%s\"",success_string);
					 printf("main.t11.txt=\"%s\"",success_string);
					 printf("\xff\xff\xff");
					 break;
				 }
				 else{
					 sprintf(TxData,"main.t11.txt=\"%s\"",fail_string);
					 printf("%s",TxData);
					 printf("\xff\xff\xff");
					 //HAL_UART_Transmit(&huart2, (uint8_t *)send_string, strlen(send_string),0xFFFF);
				 }
			}
		}else{
			if(temp_count>50000000){
				temp_count = 0;
				HAL_UART_Transmit(&huart2, (uint8_t *)send_string, strlen(send_string),0xFFFF);
			}
		}
	}
	HAL_Delay(500);
	//memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
}

void MQTT_Connect_new(void){
	Connect(c2,"RST+OK\r\n","RST+Fail\r\n");
	Connect(c4,"MODE+OK\r\n","MODE+Fail\r\n");
	Connect(c5,"WIFI+OK\r\n","WIFI+Fail\r\n");
	Connect(c6,"USERCONFIG+OK\r\n","USERCONFIG+Fail\r\n");
	Connect(c7,"CONNCONFIG+OK\r\n","CONNCONFIG+Fail\r\n");
	Connect(c8,"CONNECTION+OK\r\n","CONNECTION+Fail\r\n");
	Connect(c9,"SUBSCRIB+OK\r\n","SUBSCRIB+Fail\r\n");
	Connect(c10,"SUBSCRIB WEATHER+OK\r\n","SUBSCRIB WEATHER+Fail\r\n");
	Connect(c11,"SUBSCRIB DOOR+OK\r\n","SUBSCRIB DOOR+Fail\r\n");
	Connect(c12,"SUBSCRIB UPDATETIME+OK\r\n","SUBSCRIB UPDATETIME+Fail\r\n");
	Connect(c13,"SUBSCRIB DBRES+OK\r\n","SUBSCRIB DBRES+Fail\r\n");
}

void MQTT_Connect(void)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF); //发送AT+RST
		while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					 printf("WIFI GOT IP+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
					 printf("WIFI Fail\r\n");
					 R2x_Flag=0;
					 HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);   //超时重发
				  }
		  }
		//memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
	
	HAL_Delay(1000);
	HAL_UART_Transmit(&huart2, (uint8_t *)c4, strlen(c4),0xFFFF); //AT+CWMODE=1
		while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					// printf("%s",(char *)R2x_String);
					 printf("AT+CWMODE+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
//					 HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);
//					 memset(R2x_String, 0, strlen(R2x_String));  //清空字符串数组
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c4, strlen(c4),0xFFFF);   //超时重发
				  }
		  }
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
		  
	HAL_Delay(1000);		
	HAL_UART_Transmit(&huart2, (uint8_t *)c5, strlen(c5),0xFFFF);  //AT+CWJAP
		while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					// printf("%s",(char *)R2x_String);
					 printf("AT+CWJAP+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
//					 HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);
//					 memset(R2x_String, 0, strlen(R2x_String));  //清空字符串数组
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c5, strlen(c5),0xFFFF);   //超时重发
				  }
		  }
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
		  
  HAL_Delay(1000);
	HAL_UART_Transmit(&huart2, (uint8_t *)c6, strlen(c6),0xFFFF);   //AT+MQTTUSERCFG
				while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					// printf("%s",(char *)R2x_String);
					 printf("AT+MQTTUSERCFG+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
//					 HAL_UART_Transmit(&huart1, (uint8_t *)c2, strlen(c2),0xFFFF);
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c6, strlen(c6),0xFFFF);   //超时重发
				  }
		  }
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
		  
		HAL_Delay(1000);
		HAL_UART_Transmit(&huart2, (uint8_t *)c8, strlen(c8),0xFFFF);   //AT+MQTTCONN
		while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					 //printf("%s",(char *)R2x_String);
					 printf("AT+MQTTCONN+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
//					 HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);
//					 memset(R2x_String, 0, strlen(R2x_String));  //清空字符串数组
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c8, strlen(c8),0xFFFF);   //超时重发
				  }
		  }
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
	
	HAL_Delay(1000);
	HAL_UART_Transmit(&huart2, (uint8_t *)c9, strlen(c9),0xFFFF);  //AT+MQTTSUB
		while(1)
  		{
			count++;
			if(R2x_Flag==1 && count<=50000000)
			{
				 if(strstr((char*)R2x_String,"OK") !=NULL){  //判断wifi模块是否连接成功
					// printf("%s",(char *)R2x_String);
					 printf("AT+MQTTSUB+OK\r\n");  //在串口二显示
					 R2x_Flag=0;
					 break;
				  }
				 else
				 {
//					 HAL_UART_Transmit(&huart2, (uint8_t *)c2, strlen(c2),0xFFFF);
//					 memset(R2x_String, 0, strlen(R2x_String));  //清空字符串数组
				 }
				R2x_Flag=0;
			  }
			 else if(count>=50000000)
				 {
					  count=0;
					  HAL_UART_Transmit(&huart2, (uint8_t *)c9, strlen(c9),0xFFFF);   //超时重发
				  }
		  }
		memset(R2x_String, 0, strlen((char*)R2x_String));  //清空字符串数组
}

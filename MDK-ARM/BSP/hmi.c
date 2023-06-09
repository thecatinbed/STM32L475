/********************************
hmi部分
********************************/
#include "usart.h"
#include <stdio.h>
#include <string.h>

char Rec_buff[200],temp[200];
int	y,m,d,high,low;
char name[20], date[20] ,text_day[20]; 
int card_flag=55,finger_flag=55;   //添加或删除的标志（flage=1：添加，flag=0：删除）

extern uint8_t R2x_String[];    //接收字符串数绿
extern uint8_t R2x_Flag;
char trans[10];
//extern uint8_t R1x_buff;

/**
  * @brief  设置串口屏内容、接收判断串口屏发送的数据
  * @retval  null
  */
void him_set(void)
{
  if(R2x_Flag )	//如果收到字符串
	{
		   strcpy(Rec_buff,(char*)R2x_String);
     	  // printf("%s",Rec_buff);
		   //memset(R2x_String, 0, sizeof(R2x_String));  //清空字符串数组
		 if(strstr(Rec_buff,"theme/weather") !=NULL )   //判断串口一的接收内容中是否含有mqtt订阅的信息
		 {
			 sscanf((char*)Rec_buff,"%*[^{]{%[^}]}",temp);
			 
			 //printf("%s",temp);
			 memset(Rec_buff, 0, sizeof(Rec_buff));  //清空字符串数组
			 sscanf((char*)temp,"\'name\': \'%[^\']\', \'date\': \'%[^\']\', \'text_day\': \'%[^\']\', \'high\': \'%d\', \'low\': \'%d\'",
								name,date,text_day,&high,&low);
			 sscanf((char*)date,"%d-%d-%d",&y,&m,&d);
			 memset(temp, 0, sizeof(temp));  //清空字符串数组
     		 //printf("%s\r\n%s\r\n%s\r\n%d\r\n%d\r\n%d\r\n%d\r\n%d\r\n",name,date,text_day,high,low,y,m,d);
			 
			 
			//判断天气标志赋对应图片 
			int day = 0;
			HAL_UART_Transmit(&huart3,(uint8_t*)text_day,strlen((char*)text_day),0xFFFF);
			HAL_UART_Transmit(&huart3,(uint8_t*)"\r\n",1,0xFFFF);
			HAL_UART_Transmit(&huart3,(uint8_t*)"\xd2\xf5",strlen("\xd2\xf5"),0xFFFF);
			if(strstr(text_day,"\xc7\xe7") != NULL){
				//strcpy(text_day,"\xc7\xe7");
				day=1;
			}
			if(strstr(text_day,"\xd2\xf5") != NULL){
				//strcpy(text_day,"\xd2\xf5");
				day=2;
			}
			if(strstr(text_day,"\xd0\xa1\xd3\xea") != NULL){
				//strcpy(text_day,"\xd0\xa1\xd3\xea");
				day=3;
			}
			if(strstr(text_day,"\xd6\xd0\xd3\xea") != NULL){
				//strcpy(text_day,"\xd6\xd0\xd3\xea");
				day=4;
			}
			if(strstr(text_day,"\xb4\xf3\xd3\xea") != NULL){
				//strcpy(text_day,"\xb4\xf3\xd3\xea");
				day=5;
			}
			if(strstr(text_day,"\xb6\xe0\xd4\xc6") != NULL){
				//strcpy(text_day,"\xb6\xe0\xd4\xc6");
				day=6;
			}
//			if(strstr(name,"\\xd6\\xd8\\xc7\\xec")!=NULL){
//				//strcpy(name,"\xd6\xd8\xc7\xec");
//			}		 
					 
			switch (day)
			{
			case 1: printf("main.p0.pic=1");printf("\xff\xff\xff");break; //晴
			case 2: printf("main.p0.pic=4"); printf("\xff\xff\xff");break;//阴	
			case 3: printf("main.p0.pic=3");printf("\xff\xff\xff");break; //小雨
			case 4: printf("main.p0.pic=2");printf("\xff\xff\xff");break;//中雨		
			case 5: printf("main.p0.pic=0");printf("\xff\xff\xff");break; //大雨		
			case 6: printf("main.p0.pic=5");printf("\xff\xff\xff");break;//多云		
			}
					//为主屏幕提供当日天气数据
			printf("main.t8.txt=\"%s\"",name);printf("\xff\xff\xff");
			printf("main.t3.txt=\"%s\"",text_day);printf("\xff\xff\xff");
			printf("main.n6.val=%d",high);printf("\xff\xff\xff");
			printf("main.n7.val=%d",low);printf("\xff\xff\xff");
			printf("main.n0.val=%d",y);printf("\xff\xff\xff");
			printf("main.n1.val=%d",m);printf("\xff\xff\xff");
			printf("main.n2.val=%d",d);printf("\xff\xff\xff");
			R2x_Flag=0;
			}
	}
}
void LCD_ShowText(char* text){
	printf("main.t11.txt=\"%s\"",text);
	printf("\xff\xff\xff");
}

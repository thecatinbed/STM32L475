/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "mqtt.h"
#include "hmi.h"
#include "string.h"
#include "voice.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define pubstring "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n"
#define esp32topic "theme/esp32_on"
#define serverRecTopic "theme/access_control"
#define esp32on "on"
#define esp32off "off"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern uint8_t R2x_String[];    //鎺ユ敹瀛楃涓叉暟缁?
extern uint8_t R2x_Flag;         //鎺ユ敹瀹屾垚鏍囧織
extern uint8_t R2x_buff;
extern uint8_t R3x_String[];            
extern uint8_t R3x_Flag;         
extern uint8_t R3x_buff;
char Tx_Buf[100],BUFFER[100];
char check_string[200];
int main_status = 0,temp_status;
uint8_t door_status = 0;
uint8_t people_status = 0;
uint8_t closing_flag = 0;
RTC_TimeTypeDef GetTime;	//获取时间结构体
RTC_DateTypeDef GetDate;  //获取日期结构体
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * 鍑芥暟鍔熻兘: 閲嶅畾鍚慶搴撳嚱鏁皃rintf鍒癉EBUG_USARTx
  * 杈撳叆鍙傛暟: 鏃?
  * 杩斿洖鍊硷細鏃?
  * 璇存槑锛氭棤
  */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 50);
  return ch;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4)){	//下降沿
		HAL_Delay(20);
		if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4)){
			if(main_status==0){
				if(!door_status&&!closing_flag){
					LCD_ShowText("检测到有人");
					sprintf(Tx_Buf,pubstring,esp32topic,esp32on);
					HAL_UART_Transmit(&huart2,(uint8_t*)Tx_Buf,strlen(Tx_Buf),0xffff);
				}
			}
		}
	}else{																		//上升沿
		HAL_Delay(20);
		if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4)){
			if(main_status==0){
				if(!closing_flag){
					LCD_ShowText("关门中");
					sprintf(Tx_Buf,pubstring,esp32topic,esp32off);
					HAL_UART_Transmit(&huart2,(uint8_t*)Tx_Buf,strlen(Tx_Buf),0xffff);
					closing_flag = 1;
					__HAL_TIM_SetCounter(&htim7,0);
					HAL_TIM_Base_Start_IT(&htim7);
				}
			}
		}
	}
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance==TIM7){
		__HAL_TIM_SetCompare(&htim15,TIM_CHANNEL_2,5);
		HAL_TIM_Base_Stop_IT(&htim7);
		closing_flag = 0;
		door_status = 0;
		LCD_ShowText(" ");
	}
	if(htim->Instance==TIM16){
		if(main_status==0){
			HAL_RTC_GetTime(&hrtc,&GetTime,RTC_FORMAT_BIN);
			printf("main.t9.txt=\"%02d:%02d:%02d\"\xff\xff\xff",GetTime.Hours, GetTime.Minutes, GetTime.Seconds);
			HAL_RTC_GetDate(&hrtc,&GetDate,RTC_FORMAT_BIN);
		}
	}
}
void updatetime(){
	RTC_TimeTypeDef nTime;
	RTC_DateTypeDef nDate;
	int year,month,date,hours,minutes,seconds;
	char Tx_Data[60];
	unsigned int count = 0;
	sprintf(Tx_Data,pubstring,"theme/updateReq","update");
	HAL_UART_Transmit(&huart2,(uint8_t*)Tx_Data,strlen(Tx_Data),0xFFFF);
	while(1&&count<=50000000){
		count++;
		if(R2x_Flag){
			if(strstr((char*)R2x_String,"theme/updatetime")!=NULL){
				//HAL_UART_Transmit(&huart3,R2x_String,strlen((char*)R2x_String),0xFFFF);
				break;
			}
		}
	}
	HAL_Delay(1000);
	sscanf((char*)R2x_String,"%*[^{]{20%d-%d-%d %d:%d:%d}",&year,&month,&date,&hours,&minutes,&seconds);
	nDate.Year = year;
	nDate.Month = month;
	nDate.Date = date;
	nTime.Hours = hours;
	nTime.Minutes = minutes;
	nTime.Seconds = seconds+1;
	sprintf(Tx_Buf,"%d,%d,%d,%d,%d,%d\r\n",year,month,date,hours,minutes,seconds);
	//HAL_UART_Transmit(&huart3,(uint8_t*)Tx_Buf,strlen(Tx_Buf),0xffff);
	HAL_RTC_SetDate(&hrtc,&nDate,RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc,&nTime,RTC_FORMAT_BIN);
	if(count<=50000000){
		LCD_ShowText("获取时间成功");
	}else{
		LCD_ShowText("获取时间失败");
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM6_Init();
  MX_DAC1_Init();
  MX_TIM15_Init();
  MX_TIM7_Init();
  MX_RTC_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart2, (uint8_t *)&R2x_buff, 1);
	HAL_UART_Receive_IT(&huart3, (uint8_t *)&R3x_buff, 1);
	HAL_TIM_Base_Start(&htim6);
	HAL_TIM_PWM_Start(&htim15,TIM_CHANNEL_2);
	__HAL_TIM_SetCompare(&htim15,TIM_CHANNEL_2,5);
	//HAL_Delay(100);
	printf("page main\xff\xff\xff");  //转跳到天气主界面
	MQTT_Connect_new();
	LCD_ShowText(" ");
	updatetime();
	HAL_TIM_Base_Start_IT(&htim16);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(main_status==0){
			him_set();
			if(R2x_Flag){
				R2x_Flag = 0;
				if(strstr((char*)R2x_String,"theme/door")!=NULL){
					if(!door_status){
						door_status = 1;
						LCD_ShowText("欢迎回家");
						__HAL_TIM_SetCompare(&htim15,TIM_CHANNEL_2,25);
						HAL_DAC_Stop_DMA(&hdac1,DAC_CHANNEL_1);
						HAL_DAC_Start_DMA(&hdac1,DAC1_CHANNEL_1,(uint32_t*)voice_data,sizeof(voice_data),DAC_ALIGN_8B_R);
					}
				}
				if(strstr((char*)R2x_String,"theme/updatetime")!=NULL){
					
				}
				if(strstr((char*)R2x_String,"theme/stm32status")!=NULL){
					//strcpy(check_string,(char*)R2x_String);
					//HAL_UART_Transmit(&huart3,(uint8_t*)check_string,strlen(check_string),0xFFFF);
					sscanf((char*)R2x_String,"%*[^{]{%*[^:]:%d}",&main_status);
					if(main_status==0){
						//HAL_UART_Transmit(&huart3,(uint8_t*)"status:0\r\n",10,50);
					}else if(main_status==1){
						printf("page page1\xff\xff\xff");  //转跳到添加卡号界面
						printf("page1.t0.txt=\" \"\xff\xff\xff");
						HAL_UART_Transmit(&huart3,(uint8_t*)"status:1\r\n",10,50);//协处理器跳转到卡号管理状态
						
					}else if(main_status==2){
						printf("page page0\xff\xff\xff");  //转跳到添加指纹界面
						printf("page0.t0.txt=\" \"\xff\xff\xff");
						HAL_UART_Transmit(&huart3,(uint8_t*)"status:2\r\n",10,50);//协处理器跳转到指纹管理状态

					}
				}
			}
			if(R3x_Flag){
				R3x_Flag = 0;
				if(strstr((char*)R3x_String,"CardId")!=NULL){
					sscanf((char*)R3x_String,"%*[^:]:%s",BUFFER);
					sprintf(Tx_Buf,"RequestCard:%s",BUFFER);
					sprintf(BUFFER,pubstring,serverRecTopic,Tx_Buf);
					//HAL_UART_Transmit(&huart3,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
					HAL_UART_Transmit(&huart2,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
				}
				if(strstr((char*)R3x_String,"FingerSuccess")!=NULL){
					LCD_ShowText("指纹识别成功");
					sprintf(BUFFER,pubstring,serverRecTopic,"Fingerprint:True");
					HAL_UART_Transmit(&huart2,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
				}
				if(strstr((char*)R3x_String,"FingerFail")!=NULL){
					LCD_ShowText("指纹识别失败");
				}
			}
		}else if(main_status==1){
				if(R2x_Flag){
					R2x_Flag = 0;
					if(strstr((char*)R2x_String,"theme/stm32status")!=NULL){
						sscanf((char*)R2x_String,"%*[^{]{%*[^:]:%d}",&temp_status);
						if(temp_status==0){
							main_status = 0;
							printf("page main\xff\xff\xff");  //转跳到天气主界面
							HAL_UART_Transmit(&huart3,(uint8_t*)"status:0\r\n",10,50);//协处理器跳转到正常运行状态
						}
					}
				}
				if(R3x_Flag){
					R3x_Flag = 0;
					if(strstr((char*)R3x_String,"CardId")!=NULL){//显示卡号
						sscanf((char*)R3x_String,"%*[^:]:%s",BUFFER);
						printf("page1.t0.txt=\"卡号:%s\"\xff\xff\xff",BUFFER);
						//printf("page1.t0.txt=\"正在添加指纹...\"");printf("\xff\xff\xff");  //开始添加
					}
					if(strstr((char*)R3x_String,"AddCard")!=NULL){
						sscanf((char*)R3x_String,"%*[^:]:%s",BUFFER);
						sprintf(Tx_Buf,"AddCard:%s",BUFFER);
						sprintf(BUFFER,pubstring,serverRecTopic,Tx_Buf);
						//HAL_UART_Transmit(&huart3,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
						HAL_UART_Transmit(&huart2,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
					}
					if(strstr((char*)R3x_String,"DeleteCard")!=NULL){
						sscanf((char*)R3x_String,"%*[^:]:%s",BUFFER);
						sprintf(Tx_Buf,"DeleteCard:%s",BUFFER);
						sprintf(BUFFER,pubstring,serverRecTopic,Tx_Buf);
						//HAL_UART_Transmit(&huart3,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
						HAL_UART_Transmit(&huart2,(uint8_t*)BUFFER,strlen(BUFFER),0xffff);
					}
				}
		}else if(main_status==2){
				if(R2x_Flag){
					R2x_Flag = 0;
					if(strstr((char*)R2x_String,"theme/stm32status")!=NULL){
						sscanf((char*)R2x_String,"%*[^{]{%*[^:]:%d}",&temp_status);
						if(temp_status==0){
							main_status = 0;
							printf("page main\xff\xff\xff");  //转跳到天气主界面
							HAL_UART_Transmit(&huart3,(uint8_t*)"status:0\r\n",10,50);//协处理器跳转到正常运行状态
						}
					}
				}
				if(R3x_Flag){
					R3x_Flag = 0;
					if(strstr((char*)R3x_String,"TEXT")!=NULL){
						sscanf((char*)R3x_String,"%*[^:]:%s",BUFFER);
						printf("page0.t0.txt=\"%s\"",BUFFER);printf("\xff\xff\xff");  
					}
					if(strstr((char*)R3x_String,"TEXT:ID")!=NULL){
						printf("page0.t0.txt=\"指纹添加成功\"");printf("\xff\xff\xff");  
					}
				}
		}	
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

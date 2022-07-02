/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   串口中断接收测试
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
 
 
#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_led.h"
int panduan;

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
	char ch;
  /*初始化USART 配置模式为 115200 8-N-1，中断接收*/
  USART_Config();
	LED_GPIO_Config();
	/* 发送一个字符串 */
	Usart_SendString( DEBUG_USARTx,"这是一个串口中断接收回显实验\n");
	printf("欢迎使用野火STM32开发板\n\n\n\n");
	//scanf("%c",&ch);
  while(1)
		
	{	
		LED1_ON;
		ch=getchar();
		printf("接收到字符：%c\n",ch);
		if(ch=='a')
			{
		  LED_RED;
		}
			else
				LED_BLUE;
	}	
}
/*********************************************END OF FILE**********************/

/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   �����жϽ��ղ���
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
 
 
#include "stm32f10x.h"
#include "bsp_usart.h"
#include "bsp_led.h"
int panduan;

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{	
	char ch;
  /*��ʼ��USART ����ģʽΪ 115200 8-N-1���жϽ���*/
  USART_Config();
	LED_GPIO_Config();
	/* ����һ���ַ��� */
	Usart_SendString( DEBUG_USARTx,"����һ�������жϽ��ջ���ʵ��\n");
	printf("��ӭʹ��Ұ��STM32������\n\n\n\n");
	//scanf("%c",&ch);
  while(1)
		
	{	
		LED1_ON;
		ch=getchar();
		printf("���յ��ַ���%c\n",ch);
		if(ch=='a')
			{
		  LED_RED;
		}
			else
				LED_BLUE;
	}	
}
/*********************************************END OF FILE**********************/

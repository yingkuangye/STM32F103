/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   Һ����ʾ����ʾ��������_�������ȣ�
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
#include "./usart/bsp_usart.h"	
#include "./lcd/bsp_ili9341_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./adc/bsp_adc.h"
#include "./fft/stm32_dsp.h"
#include "./fft/table_fft.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <./dac/bsp_dac.h>
static void Delay ( __IO uint32_t nCount );
extern __IO uint16_t ADC_ConvertedValue; //��ȡ�ĵ�ѹֵ
float ADC_ConvertedValueLocal;   //ת����ĵ�ѹֵ


#define NPT 256

float adc_buf[NPT]={0};
long lBufInArray[NPT];
long lBufOutArray[NPT/2];
long lBufMagArray[NPT/2];

int i;
int j=0;
void Printf_Charater(void)   ;
void getPowerMag();
int main(void)
{	
        
	/* USART config */
	USART_Config();  
	ADCx_Init();
	GENERAL_TIM_Init();
	DMA_init();
	DAC_Mode_Init();
	
	while ( 1 )
	{
			
	/*************��ò�����************************/
	while(j<=255)    //��ò�����
		{
			ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ	
			adc_buf[j]=ADC_ConvertedValueLocal;
			//printf("adc[%d]=%f",j,adc_buf[j]);   //���ڴ�ӡ
			j++;
		}
	/***********************************************/
	
		
	/************************************************************
  ****************�Բ�������п��ٸ���Ҷ�任******************************/
		if(j==256)
		{
    for(i=0;i<NPT;i++)
	  lBufInArray[i] = ((signed short)(adc_buf[i]-2048)) << 16;
     cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
		}
		getPowerMag();//��ø���г��������ֵ
   /*****************************************************************/
	/****************************************************************/
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ	
	}
		
}



/******************************************************************
���ٸ���Ҷ�任�������г��������ֵ
lBufOutArray�洢����FFT��ĸ���ϵ��
lBufMagArray�洢�˸���г�������ķ�ֵ
*******************************************************************/
void getPowerMag()
{
    signed short lX,lY;
    float X,Y,Mag;
    unsigned short i;
    for(i=0; i<NPT/2; i++)
    {
        lX  = (lBufOutArray[i] << 16) >> 16;
        lY  = (lBufOutArray[i] >> 16);
			
        X = NPT * ((float)lX) / 32768;
        Y = NPT * ((float)lY) / 32768;
        Mag = sqrt(X * X + Y * Y) / NPT;
        if(i == 0)
            lBufMagArray[i] = (unsigned long)(Mag * 32768);
        else
            lBufMagArray[i] = (unsigned long)(Mag * 65536);
    }
		
		//��ӡ������ 
		/*for(i=0;i<NPT/2;i++)
		{
			printf("��г������[%d]=%ld \r\n",i,lBufMagArray[i]);
		}*/
}

/* ------------------------------------------end of file---------------------------------------- */


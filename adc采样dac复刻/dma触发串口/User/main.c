/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   液晶显示变量示例（整数_浮点数等）
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
extern __IO uint16_t ADC_ConvertedValue; //读取的电压值
float ADC_ConvertedValueLocal;   //转换后的电压值


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
			
	/*************获得采样点************************/
	while(j<=255)    //获得采样点
		{
			ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值	
			adc_buf[j]=ADC_ConvertedValueLocal;
			//printf("adc[%d]=%f",j,adc_buf[j]);   //串口打印
			j++;
		}
	/***********************************************/
	
		
	/************************************************************
  ****************对采样点进行快速傅里叶变换******************************/
		if(j==256)
		{
    for(i=0;i<NPT;i++)
	  lBufInArray[i] = ((signed short)(adc_buf[i]-2048)) << 16;
     cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
		}
		getPowerMag();//获得各个谐波分量幅值
   /*****************************************************************/
	/****************************************************************/
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值	
	}
		
}



/******************************************************************
快速傅里叶变换中求各个谐波分量幅值
lBufOutArray存储的是FFT后的各个系数
lBufMagArray存储了各个谐波分量的幅值
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
		
		//打印到串口 
		/*for(i=0;i<NPT/2;i++)
		{
			printf("各谐波分量[%d]=%ld \r\n",i,lBufMagArray[i]);
		}*/
}

/* ------------------------------------------end of file---------------------------------------- */


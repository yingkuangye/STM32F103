
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
#include <./GeneralTim/bsp_GeneralTim.h>
static void Delay ( __IO uint32_t nCount );
extern __IO uint16_t ADC_ConvertedValue; //读取的电压值
float ADC_ConvertedValueLocal;   //转换后的电压值


#define NPT 1000

float adc_buf[NPT]={0};
long lBufInArray[NPT];
long lBufOutArray[NPT/2];
long lBufMagArray[NPT/2];
 long Largest_Pow;
//long Largest_Pow;
float V_low;
float V_high;
int i = 0;
int j = 0;
int id = 0;
float Vpp = -1;					// 原始信号峰峰值
float f = -2.882;				// 原始信号频率
float DC_offset = 2;		// 原始信号直流偏置
void Printf_Charater(void)   ;
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
	/*	for(i=0;i<NPT/2;i++)
		{
			printf("各谐波分量[%d]=%ld \r\n",i,lBufMagArray[i]);
		}*/
}
void get_Largest_Pow()
{
	id = 1;
	Largest_Pow = lBufMagArray[1];
	for (i = 1; i < NPT/2; i++)
	{
		if (lBufMagArray[i] > Largest_Pow)
		{
			Largest_Pow = lBufMagArray[i];
			id = i;
		}
	}
	f = id * 72000000/GENERAL_TIM_Period / GENERAL_TIM_Prescaler/ NPT;
	//if(f<=200) f=f*1.25*100/93.75;
}
void get_Vpp()
{
	V_high = adc_buf[2];
	V_low = adc_buf[2];
	for (i = 2; i < NPT ; i++)
	{
		if (adc_buf[i] > V_high)
			V_high = adc_buf[i];
		if (adc_buf[i] < V_low)
			V_low = adc_buf[i];
	}
	Vpp = V_high - V_low;
}


#define BEGIN_CMD() TX_8(0XEE)            //帧头
#define END_CMD() TX_32(0XFFFCFFFF)       //帧尾
#define TX_8(P1) SEND_DATA((P1)&0xFF)                    //发送单个字节
#define TX_8N(P,N) SendNU8((uint8 *)P,N)                 //发送N个字节
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)                 //发送16位整数
#define TX_16N(P,N) SendNU16((uint16 *)P,N)              //发送N个16位整数
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)     //发送32位整数
#define SEND_DATA(P) SendChar(P)          //发送一个字节

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long
#define int32    long
static int32 num = 0;                                                                //曲线采样点计数
void  SendChar(uchar t)
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//等待串口发送完毕
}
/*!
*  \brief  串口发送送N个字节
*  \param  个数
*/
void SendNU8(uint8 *pData,uint16 nDataLen)
{
    uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_8(pData[i]);
    }
}
void SetTextFloat(uint16 screen_id,uint16 control_id,float value,uint8 precision,uint8 show_zeros)
{
	uint8 i = 0;

	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x07);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(0x02);
	TX_8((precision&0x0f)|(show_zeros?0x80:0x00));

	for (i=0;i<4;++i)
	{
	 //需要区分大小端
#if(0)
		TX_8(((uint8 *)&value)[i]);
#else
		TX_8(((uint8 *)&value)[3-i]);
#endif
	}
	END_CMD();
}
/*!
*  \brief     曲线控件-添加数据
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道号
*  \param  pData 曲线数据
*  \param  nDataLen 数据个数
*/
void GraphChannelDataAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint8 *pData,uint16 nDataLen)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x32);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_16(nDataLen);
    TX_8N(pData,nDataLen);
    END_CMD();
}

int main(void)
{	     
	/* USART config */
	USART_Config();  
	ADCx_Init();
	DMA_init();
	GENERAL_TIM_Init();
	
	while ( 1 )
	{
		j = 0;
		/*************获得采样点************************/
		while(j < NPT)    
		{
				ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值
				adc_buf[j]=ADC_ConvertedValueLocal;
				printf("adc_buf[%d] = %f\n", j, adc_buf[j]);
				j++;				
		}

	/***********************************************/
	

		get_Vpp();
		// printf("%f\n", Vpp);
	/****************************************************************/
		// 串口屏输出，注意此处不是原始波形
		// 输出数据
		//SetTextFloat(0, 9, Vpp,5, 1);
      	//SetTextFloat(0, 10, f,5, 1);
       	//SetTextFloat(0, 11, DC_offset,5, 1);
		// 输出波形
		uint8 wave[256];
		for (i = 0; i < 256; i++)
		{
			wave[i] = (uint8)adc_buf[2*i]*100;
			//printf("adc_buf[%d] = %f, wave[%d] = %d\n", i*2,adc_buf[i*2], i, wave[i]);
		} 
		//GraphChannelDataAdd(0,7,0, &wave[0],256); 
	}
}

/* ------------------------------------------end of file---------------------------------------- */


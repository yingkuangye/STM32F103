
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
extern __IO uint16_t ADC_ConvertedValue; //��ȡ�ĵ�ѹֵ
float ADC_ConvertedValueLocal;   //ת����ĵ�ѹֵ


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
float Vpp = -1;					// ԭʼ�źŷ��ֵ
float f = -2.882;				// ԭʼ�ź�Ƶ��
float DC_offset = 2;		// ԭʼ�ź�ֱ��ƫ��
void Printf_Charater(void)   ;
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
	/*	for(i=0;i<NPT/2;i++)
		{
			printf("��г������[%d]=%ld \r\n",i,lBufMagArray[i]);
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


#define BEGIN_CMD() TX_8(0XEE)            //֡ͷ
#define END_CMD() TX_32(0XFFFCFFFF)       //֡β
#define TX_8(P1) SEND_DATA((P1)&0xFF)                    //���͵����ֽ�
#define TX_8N(P,N) SendNU8((uint8 *)P,N)                 //����N���ֽ�
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)                 //����16λ����
#define TX_16N(P,N) SendNU16((uint16 *)P,N)              //����N��16λ����
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)     //����32λ����
#define SEND_DATA(P) SendChar(P)          //����һ���ֽ�

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long
#define int16    short int
#define int32    long
#define int32    long
static int32 num = 0;                                                                //���߲��������
void  SendChar(uchar t)
{
    USART_SendData(USART1,t);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    while((USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET));//�ȴ����ڷ������
}
/*!
*  \brief  ���ڷ�����N���ֽ�
*  \param  ����
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
	 //��Ҫ���ִ�С��
#if(0)
		TX_8(((uint8 *)&value)[i]);
#else
		TX_8(((uint8 *)&value)[3-i]);
#endif
	}
	END_CMD();
}
/*!
*  \brief     ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  pData ��������
*  \param  nDataLen ���ݸ���
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
		/*************��ò�����************************/
		while(j < NPT)    
		{
				ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ
				adc_buf[j]=ADC_ConvertedValueLocal;
				printf("adc_buf[%d] = %f\n", j, adc_buf[j]);
				j++;				
		}

	/***********************************************/
	

		get_Vpp();
		// printf("%f\n", Vpp);
	/****************************************************************/
		// �����������ע��˴�����ԭʼ����
		// �������
		//SetTextFloat(0, 9, Vpp,5, 1);
      	//SetTextFloat(0, 10, f,5, 1);
       	//SetTextFloat(0, 11, DC_offset,5, 1);
		// �������
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


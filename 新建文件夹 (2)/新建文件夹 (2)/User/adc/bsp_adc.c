#include "bsp_adc.h"

__IO uint16_t ADC_ConvertedValue;

/**
  * @brief  ADC GPIO ��ʼ��
  * @param  ��
  * @retval ��
  */
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// �� ADC IO�˿�ʱ��
	ADC_GPIO_APBxClock_FUN ( ADC_GPIO_CLK, ENABLE );
	
	// ���� ADC IO ����ģʽ
	GPIO_InitStructure.GPIO_Pin = ADC_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	// ��ʼ�� ADC IO
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);				
}

/**
  * @brief  ����ADC����ģʽ
  * @param  ��
  * @retval ��
  */
static void ADCx_Mode_Config(void)
{
	
	ADC_InitTypeDef ADC_InitStructure;
	
	// ��ADCʱ��
	ADC_APBxClock_FUN ( ADC_CLK, ENABLE );
	
	
	// ADC ģʽ����
	// ֻʹ��һ��ADC�����ڵ�ģʽ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	
	// ��ֹɨ��ģʽ����ͨ����Ҫ����ͨ������Ҫ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE ; 

	// ����ת��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;

	// timer3����
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;   

	// ת������Ҷ���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	
	// ת��ͨ��1��
	ADC_InitStructure.ADC_NbrOfChannel = 1;	
		
	// ��ʼ��ADC
	ADC_Init(ADCx, &ADC_InitStructure);
	
	// ����ADCʱ��ΪPCLK2��8��Ƶ����9MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	
	// ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ55.5��ʱ������
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL, 1, ADC_SampleTime_55Cycles5);
	
	// ʹ��ADC DMA ����
	ADC_DMACmd(ADCx, ENABLE);
	
	// ����ADC ������ʼת��
	ADC_Cmd(ADCx, ENABLE);
	
	// ��ʼ��ADC У׼�Ĵ���  
	ADC_ResetCalibration(ADCx);
	// �ȴ�У׼�Ĵ�����ʼ�����
	while(ADC_GetResetCalibrationStatus(ADCx));
	
	// ADC��ʼУ׼
	ADC_StartCalibration(ADCx);
	// �ȴ�У׼���
	while(ADC_GetCalibrationStatus(ADCx));
	//ʹ���ⲿ����
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);	
}


static void DMA_ADCConfig()
{
	DMA_InitTypeDef DMA_InitStructure;
	// ��DMAʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	// ��λDMA������
	DMA_DeInit(ADC_DMA_CHANNEL);
	
	// ���� DMA ��ʼ���ṹ��
	// �����ַΪ��ADC ���ݼĴ�����ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr = ( uint32_t ) ( & ( ADCx->DR ) );
	
	// �洢����ַ��ʵ���Ͼ���һ���ڲ�SRAM�ı���
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValue;
	
	// ����Դ��������
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	
	// ��������СΪ1���������Ĵ�СӦ�õ��ڴ洢���Ĵ�С
	DMA_InitStructure.DMA_BufferSize = 1;
	
	// ����Ĵ���ֻ��һ������ַ���õ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	// �洢����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; 
	
	// �������ݴ�СΪ���֣��������ֽ�
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	
	// �洢�����ݴ�СҲΪ���֣����������ݴ�С��ͬ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	
	// ѭ������ģʽ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular ;	

	// DMA ����ͨ�����ȼ�Ϊ�ߣ���ʹ��һ��DMAͨ��ʱ�����ȼ����ò�Ӱ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	
	// ��ֹ�洢�����洢��ģʽ����Ϊ�Ǵ����赽�洢��
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	
	// ��ʼ��DMA
	DMA_Init(ADC_DMA_CHANNEL, &DMA_InitStructure);
	
	// ʹ�� DMA ͨ��
	DMA_Cmd(ADC_DMA_CHANNEL , ENABLE);
	DMA_ClearFlag(DMA1_FLAG_TC1);   //����ж�
	DMA_ITConfig(ADC_DMA_CHANNEL, DMA_IT_TC, ENABLE);
}
static void DMA_NVIC_Config()   //������DMA�жϳ�ʼ����ע�������DMA��ʼ����ʹ��DMA�ж�
{
	NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn  ;  //�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
}
/**
  * @brief  ADC��ʼ��
  * @param  ��
  * @retval ��
  */
void ADCx_Init(void)
{
	ADCx_GPIO_Config();
	ADCx_Mode_Config();
}

void DMA_init()
{
	DMA_ADCConfig(); //��ADC�йص�DMA����
	//DMA_NVIC_Config() ;//dma�жϳ�ʼ�������û���жϿ���ɾ��
}
/*********************************************END OF FILE**********************/

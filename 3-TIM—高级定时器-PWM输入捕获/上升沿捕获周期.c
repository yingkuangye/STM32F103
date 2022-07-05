//野火开发板例程只给了测量高电平脉宽
//这里给出将例程改为测量脉冲频率的方法
//我们只用修改中断服务函数 GENERAL_TIM_INT_FUN 即可
// 只用改这个函数的第二个参数 GENERAL_TIM_OCxPolarityConfig_FUN(GENERAL_TIM_CATCH, TIM_ICPolarity_Rising);
//如果向将本例程改为均为下降沿触发，只用将配置边沿的函数的参数均改为下降沿即可
void GENERAL_TIM_INT_FUN(void)
{
	// TIM_IT_Update为溢出的中断标志，这一步是在处理周期大于我们计数周期
	//而溢出的情况
	if ( TIM_GetITStatus ( GENERAL_TIM_CATCH, TIM_IT_Update) != RESET )               
	{	
		TIM_ICUserValueStructure.Capture_Period ++;	 //这个参数记录经历了几个周期数	
		TIM_ClearITPendingBit ( GENERAL_TIM_CATCH, TIM_FLAG_Update ); //清除中断标志位（溢出）		
	}

	// 下面开始在一个周期内进行处理
	//判断有无定时器中断
	if ( TIM_GetITStatus (GENERAL_TIM_CATCH, GENERAL_TIM_IT_CCx ) != RESET)
	{
	   //第一次捕获
		if ( TIM_ICUserValueStructure.Capture_StartFlag == 0 )
		{
			//计数器清零
			TIM_SetCounter ( GENERAL_TIM_CATCH, 0 );
			//周期数记录变量清零
			TIM_ICUserValueStructure.Capture_Period = 0;
            // 	捕获寄存器清零（捕获计时器的功能是在到达设定的脉冲沿时将计数器的值记录下来）
			TIM_ICUserValueStructure.Capture_CcrValue = 0;
			//配置下一次触发为上升沿触发
			/**************移植时唯二需要改的*********************/
            GENERAL_TIM_OCxPolarityConfig_FUN(GENERAL_TIM_CATCH, TIM_ICPolarity_Rising);
		    //开始计时标志
			TIM_ICUserValueStructure.Capture_StartFlag = 1;			
		}
		// 记录第二个脉冲，这里还是上升沿触发
		else  
		{
			//记录捕获寄存器的值，这个值就是我们捕获到方波周期的值
			TIM_ICUserValueStructure.Capture_CcrValue = 
			GENERAL_TIM_GetCapturex_FUN (GENERAL_TIM_CATCH);
			//配置下一次触发为上升沿触发
			/**************移植时唯二需要改的*********************/
            GENERAL_TIM_OCxPolarityConfig_FUN(GENERAL_TIM_CATCH, TIM_ICPolarity_Rising);
            //开始捕获标志置0
			TIM_ICUserValueStructure.Capture_StartFlag = 0;
           //捕获完成标志置1	
			TIM_ICUserValueStructure.Capture_FinishFlag = 1;		
		}
         //清除定时器中断
		TIM_ClearITPendingBit (GENERAL_TIM_CATCH,GENERAL_TIM_IT_CCx);	    
	}		
}





// 这个是我们自己定义的一个很重要存储周期的结构体
typedef struct
{   
	uint8_t   Capture_FinishFlag;   // 捕获结束标志位
	uint8_t   Capture_StartFlag;    // 捕获开始标志位
	uint16_t  Capture_CcrValue;     // 捕获寄存器的值记录
	uint16_t  Capture_Period;       //自动重装载值更新次数标志（多余的周期数）
}TIM_ICUserValueTypeDef;
//捕获结束标志和开始标志顾名思义
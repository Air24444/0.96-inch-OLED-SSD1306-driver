#include "stm32f10x.h"
#include "oled.h"
#include "My_timer.h"
#include "OLED_Font.h"
#include "delay.h"
uint16_t s=0;
uint16_t min=59;
uint16_t h=23;
int main(void)
{
    OLED_Init();
    Timer_Init();

    uint8_t start_x = 0;  // 起始横坐标
	uint8_t start_y = 15;
	OLED_DrawBitmap(128-32, 1, OLED_dian,32,16);    
	OLED_DrawBitmap(1, 1,OLED_du[0], 8, 16);      
	OLED_DrawBitmap(8, 1,OLED_du[1], 8, 16);     
	OLED_DrawBitmap(16, 1,OLED_du[2], 8, 16);     
	//OLED_InvertArea(0,0,128,16);
    while (1)
    {
        // 1. 清空整个帧缓冲区
        OLED_ClearArea(0, 16, 128, 64-16);

        // 2. 绘制小时（两位数）
        OLED_DrawBitmap(start_x, start_y, OLED_Num[h / 10], 16, 32);      // 小时的十位
        OLED_DrawBitmap(start_x + 16, start_y, OLED_Num[h % 10], 16, 32);  // 小时的个位

        // 3. 绘制冒号
        OLED_DrawBitmap(start_x + 32, start_y, OLED_Num[10], 16, 32);      // 冒号

        // 4. 绘制分钟（两位数）
        OLED_DrawBitmap(start_x + 48, start_y, OLED_Num[min / 10], 16, 32); // 分钟的十位
        OLED_DrawBitmap(start_x + 64, start_y, OLED_Num[min % 10], 16, 32); // 分钟的个位
		
		// 5. 绘制冒号
        OLED_DrawBitmap(start_x + 80, start_y, OLED_Num[10], 16, 32);      // 冒号
		
		// 6. 绘制秒（两位数）
        OLED_DrawBitmap(start_x + 96, start_y, OLED_Num[s / 10], 16, 32); // 分钟的十位
        OLED_DrawBitmap(start_x + 96+16, start_y, OLED_Num[s % 10], 16, 32); // 分钟的个位
		
		// 9.取反
		//OLED_InvertArea(0,16,128,64-16);
        // 8. 刷新到屏幕
        OLED_Refresh();

        // 9. 等1秒，避免重复刷新
        Delay_ms(1000);
    }
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		s++;
		if(s==60)
		{
			s=0;
			min++;
			if(min==60)
			{
				min =0;
				h++;
				if(h==24)
				{
					h=0;
				}
		    }
		}
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

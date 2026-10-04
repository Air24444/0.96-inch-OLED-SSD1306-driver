#include "stm32f10x.h"
#include "delay.h"

/*引脚配置*/
#define OLED_W_SCL(x) GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x) GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

uint8_t framebuffer[1024];

/*引脚初始化*/
void OLED_I2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
 * @brief  I2C开始
 * @param  无
 * @retval 无
 */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
 * @brief  I2C停止
 * @param  无
 * @retval 无
 */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
 * @brief  I2C发送一个字节
 * @param  Byte 要发送的一个字节
 * @retval 无
 */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(Byte & (0x80 >> i)));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1); // 额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
 * @brief  OLED写命令
 * @param  Command 要写入的命令
 * @retval 无
 */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78); // 从机地址
	OLED_I2C_SendByte(0x00); // 写命令
	OLED_I2C_SendByte(Command);
	OLED_I2C_Stop();
}

/**
 * @brief  OLED写数据
 * @param  Data 要写入的数据
 * @retval 无
 */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78); // 从机地址
	OLED_I2C_SendByte(0x40); // 写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);				 // 设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4)); // 设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));		 // 设置X位置低4位
}

/**
 * @brief  OLED清屏
 * @param  无
 * @retval 无
 */
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for (i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
 * @brief  在帧缓冲区中画一个点
 * @param  x: 横坐标 (0 ~ 127)
 * @param  y: 纵坐标 (0 ~ 63)
 * @param  color: 1=点亮, 0=熄灭
 */
void OLED_DrawPixel(int16_t x, int16_t y, uint8_t color)
{
    // 边界检查，防止越界
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;

    uint16_t byte_index = (y / 8) * 128 + x; // 计算这个像素点属于第几个字节
    uint8_t  bit_mask   = 1 << (y % 8);      // 计算这个像素点在这个字节的第几位

    if (color)
        framebuffer[byte_index] |= bit_mask;  // 点亮
    else
        framebuffer[byte_index] &= ~bit_mask; // 熄灭
}

/**
 * @brief  在屏幕任意位置显示一个自定义图像
 * @param  x: 图像左上角横坐标
 * @param  y: 图像左上角纵坐标
 * @param  w: 图像宽度（像素）
 * @param  h: 图像高度（像素）
 * @param  bitmap: 图像点阵数据（规则：横向取模，一个字节存8个垂直像素，高位在上）
 */
void OLED_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h)
{
    int16_t byte_col, pixel_row;
    // 按图像的列循环
    for (byte_col = 0; byte_col < w; byte_col++) {
        // 按图像的像素行循环
        for (pixel_row = 0; pixel_row < h; pixel_row++) {
            // 1. 计算当前像素在图像数据数组中的位置
            //    规则是“横向取模，字节高位在上”
            uint16_t data_index = (pixel_row / 8) * w + byte_col;
            uint8_t  data_byte  = bitmap[data_index];
            uint8_t  bit_mask   = 1 << (pixel_row % 8); // 注意：低位在上
			
			 // 2. 判断图像在这个位置的像素是不是应该被点亮
            if (data_byte & bit_mask) {
                // 3. 如果该点亮，就调用画点函数，画到framebuffer上
                //    注意这里的坐标叠加：图像内的坐标 + 起始偏移量 = 最终屏幕坐标
                OLED_DrawPixel(x + byte_col, y + pixel_row, 1);
            }
			
//             2. 判断图像在这个位置的像素是不是应该被点亮
//            if (data_byte & bit_mask) 
//			{
//                OLED_DrawPixel(x + byte_col, y + pixel_row, 1); // 点亮
//            } 
//			else 
//			{
//                OLED_DrawPixel(x + byte_col, y + pixel_row, 0); // 熄灭
//            }
        }
    }
}

/**
  * @brief  清空帧缓冲区中指定区域的所有像素
  * @param  x: 区域左上角横坐标
  * @param  y: 区域左上角纵坐标
  * @param  w: 区域宽度（像素）
  * @param  h: 区域高度（像素）
  */
void OLED_ClearArea(int16_t x, int16_t y, int16_t w, int16_t h)
{
    int16_t i, j;
    for (i = 0; i < w; i++)
    {
        for (j = 0; j < h; j++)
        {
            OLED_DrawPixel(x + i, y + j, 0);  // 把这个像素熄灭
        }
    }
}

/**
  * @brief  在屏幕任意位置连续显示一串汉字
  * @param  x: 起始横坐标 (0 ~ 127)
  * @param  y: 起始纵坐标 (0 ~ 63)
  * @param  font_base: 字库的首地址，例如 (const uint8_t*)OLED_CN16x16
  * @param  indices: 汉字索引数组，存放要显示的汉字在字库中的序号
  * @param  count: 要显示的汉字个数
  * @param  char_w: 每个汉字的宽度（像素）
  * @param  char_h: 每个汉字的高度（像素）
  * @note   此函数会直接修改全局帧缓冲区 framebuffer[1024]，调用后需使用 OLED_Refresh() 更新屏幕
  */
void OLED_ShowString_ByIndex(uint8_t x, uint8_t y,const uint8_t *font_base,const uint8_t *indices,
                             uint8_t count,uint8_t char_w, uint8_t char_h)
{
    uint8_t i;
    uint16_t char_bytes = (char_w * char_h) / 8; // 每个汉字占用的字节数

    for (i = 0; i < count; i++)
    {
        uint8_t idx = indices[i];                        // 获取当前汉字的索引号
        const uint8_t *char_data = font_base + (idx * char_bytes); // 指针移动到当前汉字的数据起始位置
        OLED_DrawBitmap(x + i * char_w, y, char_data, char_w, char_h); // 将这个汉字“贴”到画布上
    }
}

/**
  * @brief  将帧缓冲区指定矩形区域像素反色
  * @param  x: 区域左上角横坐标
  * @param  y: 区域左上角纵坐标
  * @param  w: 区域宽度
  * @param  h: 区域高度
  * @note   修改framebuffer，调用OLED_Refresh()生效
  */
void OLED_InvertArea(int16_t x, int16_t y, int16_t w, int16_t h)
{
    int16_t i, j;
    // 遍历区域内所有像素
    for (i = 0; i < w; i++)
    {
        for (j = 0; j < h; j++)
        {
            int16_t px = x + i;
            int16_t py = y + j;
            // 边界判断，防止越界
            if (px < 0 || px >= 128 || py < 0 || py >= 64)
                continue;

            uint16_t byte_index = (py / 8) * 128 + px;
            uint8_t  bit_mask   = 1 << (py % 8);

            framebuffer[byte_index] ^= bit_mask;  // 异或实现按位取反
        }
    }
}

void OLED_Refresh(void)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++) {
        OLED_SetCursor(i, 0);
        for (j = 0; j < 128; j++) {
            OLED_WriteData(framebuffer[i * 128 + j]);
        }
    }
}

/**
 * @brief  OLED初始化
 * @param  无
 * @retval 无
 */
void OLED_Init(void)
{
	uint32_t i, j;

	for (i = 0; i < 1000; i++) // 上电延时
	{
		for (j = 0; j < 1000; j++);
	}

	OLED_I2C_Init(); // 端口初始化

	OLED_WriteCommand(0xAE); // 关闭显示

	OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);

	OLED_WriteCommand(0xA8); // 设置多路复用率
	OLED_WriteCommand(0x3F);

	OLED_WriteCommand(0xD3); // 设置显示偏移
	OLED_WriteCommand(0x00);

	OLED_WriteCommand(0x40); // 设置显示开始行

	OLED_WriteCommand(0xA1); // 设置左右方向，0xA1正常 0xA0左右反置

	OLED_WriteCommand(0xC8); // 设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
	OLED_WriteCommand(0x12);

	OLED_WriteCommand(0x81); // 设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9); // 设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB); // 设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4); // 设置整个显示打开/关闭

	OLED_WriteCommand(0xA6); // 设置正常/倒转显示

	OLED_WriteCommand(0x8D); // 设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF); // 开启显示
	
	OLED_Clear();
}

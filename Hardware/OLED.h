#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ClearArea(int16_t x, int16_t y, int16_t w, int16_t h);
void OLED_DrawPixel(int16_t x, int16_t y, uint8_t color);
void OLED_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h);
void OLED_ShowString_ByIndex(uint8_t x, uint8_t y,const uint8_t *font_base,const uint8_t *indices,
                             uint8_t count,uint8_t char_w, uint8_t char_h);
void OLED_InvertArea(int16_t x, int16_t y, int16_t w, int16_t h);
void OLED_Refresh(void);
#endif

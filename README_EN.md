# STM32 OLED Driver Library (STM32-OLED-Driver)

## 📖 Project Overview
This is a 0.96-inch OLED (SSD1306) graphics driver library developed for the STM32F103 series based on the Standard Peripheral Library. 

This project does **NOT** rely on any third-party graphics libraries. It features a fully hand-written software I2C timing sequence and a complete framebuffer architecture. It provides a highly abstracted drawing API supporting pixel operations, area clearing, image inversion, custom bitmaps, and indexed Chinese character display. The `main.c` in this repository is just a demo application (a digital clock) to verify the driver's capabilities.

## 🚀 Driver Features (API Capabilities)
*   **Low-level Timing**: GPIO-based software I2C (open-drain output), consuming no hardware I2C peripherals.
*   **Memory Architecture**: Built-in `framebuffer[1024]`. All drawing operations are completed in memory first, then refreshed to the screen in one go, avoiding frequent I2C communications.
*   **Graphics API**:
    *   `OLED_DrawPixel()`: Draw a pixel (with boundary checking).
    *   `OLED_DrawBitmap()`: Draw bitmaps of any size (horizontal byte orientation, MSB first).
    *   `OLED_ClearArea()`: Clear a specified rectangular area for partial refresh.
    *   `OLED_InvertArea()`: Invert pixels in a specified area (implemented via XOR).
    *   `OLED_ShowString_ByIndex()`: Display arbitrary Chinese characters at any position based on font index.
    *   `OLED_Refresh()`: Refresh the entire framebuffer to the screen.
    *   `OLED_Init()`: Standard SSD1306 initialization sequence.

## 🛠️ Hardware & Environment
*   **MCU**: STM32F103C8T6 (72MHz)
*   **Display**: 0.96-inch OLED (SSD1306, 128x64)
*   **Pins**: SCL -> PB8, SDA -> PB9
*   **Environment**: Keil MDK-ARM (V5) + STM32 Standard Library

## 📂 Code Architecture
*   `oled.c` / `oled.h`: Core driver layer. Contains I2C timing, framebuffer, and drawing APIs.
*   `OLED_Font.c` / `OLED_Font.h`: Font layer. Contains 16x32 number arrays and custom icons.
*   `main.c`: Test case. Demonstrates how to use drawing APIs to assemble a digital clock in the main loop.
*   `My_timer.c` / `My_timer.h`: Auxiliary. Provides a 1-second timer interrupt for the demo.

## ⚠️ Code Review & Optimization Directions
As a driver with an industrial prototype, there is room for improvement in extreme performance and engineering robustness:

1.  **Missing I2C ACK Detection**:
    `OLED_I2C_SendByte()` currently ignores the ACK on the 9th clock cycle. If the OLED disconnects, the driver won't know.
    *   *Optimization*: Switch SDA to input mode on the 9th SCL cycle. If NACK is detected, return an error code and abort the transmission.
2.  **Software I2C Timing Too Fast**:
    STM32 GPIO toggles extremely fast. The current code lacks microsecond delays.
    *   *Optimization*: Add `delay_us(1)` while SCL is high to ensure signal stability within the SSD1306's 400kHz limit, or switch to hardware I2C.
3.  **`OLED_DrawBitmap()` Rendering Efficiency**:
    The function calls `OLED_DrawPixel()` for every pixel, causing massive function call overhead and redundant bit operations when drawing full screens.
    *   *Optimization*: Directly operate on the `framebuffer` bytes using bitwise OR (`|=`) to bypass per-pixel function calls.
4.  **Blocking Demo in Main Loop**:
    `Delay_ms(1000)` in `main.c` makes the demo unresponsive.
    *   *Optimization*: Set a `time_flag` in the timer interrupt. Make the main loop "non-blocking" to allow for future key inputs and menu systems.

## 📌 TODO
- [ ] Add hardware I2C + DMA transfer for maximum refresh rate.
- [ ] Improve font library, support Chinese lexicon or custom font sizes.
- [ ] Integrate FreeRTOS to provide thread-safe refresh APIs.

## 👨‍💻 Author
**Air24444 (艾尔帕提)**
*Harbin Normal University - Electronic Information Science and Technology*
*Focus: Embedded Software / Edge AI*

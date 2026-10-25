/*
 * mip-8color-driver.h
 *
 *  Created on: 15.10.2026
 *      Author: Dominik Piórkowski
 */

 #ifndef DRIVERS_MIP_8COLOR_MIP_8COLOR_DRIVER_H_
 #define DRIVERS_MIP_8COLOR_MIP_8COLOR_DRIVER_H_

// // include STM32 HAL and STM32 HAL SPI header files
#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_spi.h"
#include <stdint.h>

// Display resolution
#define SCREEN_WIDTH_IN_PIXELS  	176
#define SCREEN_HEIGHT_IN_PIXELS 	176
#define SCREEN_WIDTH_IN_BYTES       3 * (SCREEN_WIDTH_IN_PIXELS / 8)

/**
 *  Color definitions for JDI 8-color MIP display
*/
// 									  R G B
#define JDI_8COL_BLACK   0x00		//0 0 0
#define JDI_8COL_GREEN   0x02		//0 0 1
#define JDI_8COL_RED	 0x04		//0 1 0
#define JDI_8COL_YELLOW  0x06		//0 1 1
#define JDI_8COL_BLUE    0x08		//1 0 0
#define JDI_8COL_CYAN    0x0A		//1 0 1
#define JDI_8COL_MAGENTA 0x0C		//1 1 0
#define JDI_8COL_WHITE   0x0E		//1 1 1

#define BITS_PER_PIXEL 3u   // 3 bits per pixel for 8 colors


#ifdef __cplusplus
extern "C" {
#endif

/** SPI and GPIO configuration */
typedef struct
{
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
}MIP_8c_Display_Config_t;


/**
 * @brief Initializes the MIP 8-color display driver.
 *
 * @param config Pointer to the display hardware configuration.
 */
void MIP_8c_Display_Init(const MIP_8c_Display_Config_t *config);

/**
    * @brief Draws a horizontal line. This method operates on full bytes, not on pixels also requires to redraw only 1 line of the screen, 
    * hence has very good performance
    *
    * @param x row, in PIXELS
    * @param color The color to use for the horizontal line. The color can be one of the following:
    *  - JDI_8COL_BLACK: Black color (0x00)
    *  - JDI_8COL_GREEN: Green color (0x02)
    *  - JDI_8COL_RED: Red color (0x04)
    *  - JDI_8COL_YELLOW: Yellow color (0x06)
    *  - JDI_8COL_BLUE: Blue color (0x08)
    *  - JDI_8COL_CYAN: Cyan color (0x0A)
    *  - JDI_8COL_MAGENTA: Magenta color (0x0C)
    *  - JDI_8COL_WHITE: White color (0x0E)
    */
void MIP_8c_Display_DrawHorizontalLineColor(uint16_t x, uint8_t color);

/**
    * @brief Refreshes the screen by sending the updated screen buffer to the display.
    *
    * @param line_start The starting line (row) to refresh, in PIXELS.
    * @param line_end The ending line (row) to refresh, in PIXELS.
    */
void MIP_8c_Display_RefreshScreen(uint8_t line_start, uint8_t line_end);

void MIP_8c_Display_ClearScreen();


// @TODO So far the driver is designed to work with Display which is using EXTCOMIN pin to switch VCOM. 
// In future, it should be possible to use the driver with display which is not using EXTCOMIN pin. 
// In that case, the driver should provide function which allows to switch VCOM bit in command byte. 
// The function should be called with period of 1 second, to avoid burn-in effect on display.


#ifdef __cplusplus
}
#endif


#endif /* DRIVERS_MIP_8COLOR_MIP_8COLOR_DRIVER_H_ */
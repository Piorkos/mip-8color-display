/*
 * mip-8color-driver.h
 *
 *  Created on: 15.10.2026
 *      Author: Dominik Piórkowski
 */
#include <stdbool.h>
#include <stdint.h>
#include <sys/_intsup.h>

// #include "main.h"
#include "MIP-8c/mip-8c-display.h"
#include "MIP-8c/font.h"
#include "stm32wbxx_hal_def.h"


//  Three packed bits per pixel; round up in case the display size is not byte-aligned
static uint8_t screen_buffer[(SCREEN_WIDTH_IN_PIXELS * SCREEN_HEIGHT_IN_PIXELS * BITS_PER_PIXEL + 7u) / 8u];

static MIP_8c_Display_Config_t display_config;
static bool vcom_bool_ = false;


void MIP_8c_Display_Init(const MIP_8c_Display_Config_t *config)
{
    display_config = *config;
    HAL_GPIO_WritePin(display_config.cs_port, display_config.cs_pin, GPIO_PIN_RESET);	// this display is low on inactive
    
    // Initialize buffer with white pixels
    for(int i = 0; i < sizeof(screen_buffer); ++i)
    {
        screen_buffer[i] = 0xFF;
    }

    // IMPORTANT! 
    // Workaround: Toggle CS before the first SPI transmission to eliminate an unwanted SCK pulse observed during initial SPI startup.
    // This ensures the display sees a clean CS transition before data transfer.
    // Do not remove until the initial SCK glitch has been properly investigated and resolved.
    HAL_GPIO_WritePin(display_config.cs_port, display_config.cs_pin, GPIO_PIN_SET);	// this display is low on inactive
    HAL_GPIO_WritePin(display_config.cs_port, display_config.cs_pin, GPIO_PIN_RESET);	// this display is low on inactive
}


void MIP_8c_Display_DrawHorizontalLine(uint16_t x, uint8_t color)
{
    for(int i = 0; i < SCREEN_WIDTH_IN_BYTES; i += 3)
    {
        screen_buffer[x*SCREEN_WIDTH_IN_BYTES + i] = (color << 5) | (color << 2) | (color >> 1);
        screen_buffer[x*SCREEN_WIDTH_IN_BYTES + i + 1] = (color << 7) | (color << 4) | (color << 1) | (color >> 2);
        screen_buffer[x*SCREEN_WIDTH_IN_BYTES + i + 2] = (color << 6) | (color << 3) | color;
    }
}

void MIP_8c_Display_DrawVerticalLine(uint16_t x, uint8_t color)
{
    if (x >= SCREEN_WIDTH_IN_PIXELS)
    {
        return;
    }

    for (uint16_t y = 0; y < SCREEN_HEIGHT_IN_PIXELS; ++y)
    {
        MIP_8c_Display_SetPixel(x, y, color);
    }
}

void MIP_8c_Display_SetPixel(uint16_t x, uint16_t y, uint8_t color)
{
    if (x >= SCREEN_WIDTH_IN_PIXELS || y >= SCREEN_HEIGHT_IN_PIXELS)
    {
        return;
    }

    uint32_t pixel_bit_index = (uint32_t)x * BITS_PER_PIXEL;
    uint32_t buffer_index = (uint32_t)y * SCREEN_WIDTH_IN_BYTES + pixel_bit_index / 8u;
    uint8_t bit_offset = (uint8_t)(pixel_bit_index % 8u);
    uint8_t pixel_color = color & ((1u << BITS_PER_PIXEL) - 1u);

    for (uint8_t bit = 0; bit < BITS_PER_PIXEL; ++bit)
    {
        uint32_t destination_bit = bit_offset + bit;
        uint32_t destination_byte = buffer_index + destination_bit / 8u;
        uint8_t mask = (uint8_t)(1u << (7u - destination_bit % 8u));
        uint8_t color_bit = (uint8_t)((pixel_color >> (BITS_PER_PIXEL - 1u - bit)) & 1u);

        if (color_bit != 0)
        {
            screen_buffer[destination_byte] |= mask;
        }
        else
        {
            screen_buffer[destination_byte] &= (uint8_t)~mask;
        }
    }
}

void MIP_8c_Display_DrawShape(uint16_t x, uint16_t y, uint8_t color, const gfx_icons_t *shape)
{
    if (shape == NULL || shape->data == NULL || shape->width == 0 || shape->height == 0)
    {
        return;
    }

    static const uint16_t expanded_nibble_masks[16] = {
        0x000, 0x007, 0x038, 0x03F,
        0x1C0, 0x1C7, 0x1F8, 0x1FF,
        0xE00, 0xE07, 0xE38, 0xE3F,
        0xFC0, 0xFC7, 0xFF8, 0xFFF
    };
    uint32_t bytes_per_row = ((uint32_t)shape->width + 7u) / 8u;
    uint32_t repeated_color = (uint32_t)(color & 0x07u) * 0x249249u;

    for (uint32_t shape_y = 0; shape_y < shape->height; ++shape_y)
    {
        uint32_t screen_y = (uint32_t)y + shape_y;

        if (screen_y >= SCREEN_HEIGHT_IN_PIXELS)
        {
            break;
        }

        for (uint32_t shape_x = 0; shape_x < shape->width; shape_x += 8u)
        {
            uint32_t screen_x = (uint32_t)x + shape_x;

            if (screen_x >= SCREEN_WIDTH_IN_PIXELS)
            {
                break;
            }

            uint32_t source_index = shape_y * bytes_per_row + shape_x / 8u;
            uint8_t source_bits = shape->data[source_index];
            uint32_t valid_pixels = (uint32_t)shape->width - shape_x;
            uint32_t visible_pixels = SCREEN_WIDTH_IN_PIXELS - screen_x;

            if (valid_pixels > 8u)
            {
                valid_pixels = 8u;
            }
            if (valid_pixels > visible_pixels)
            {
                valid_pixels = visible_pixels;
            }

            source_bits &= (uint8_t)(0xFFu << (8u - valid_pixels));

            uint32_t pixel_mask =
                ((uint32_t)expanded_nibble_masks[source_bits >> 4] << 12) |
                expanded_nibble_masks[source_bits & 0x0Fu];
            if (pixel_mask == 0)
            {
                continue;
            }

            uint32_t pixel_bits = repeated_color & pixel_mask;
            uint32_t pixel_bit_index = screen_x * BITS_PER_PIXEL;
            uint32_t buffer_index =
                screen_y * SCREEN_WIDTH_IN_BYTES + pixel_bit_index / 8u;
            uint32_t bit_offset = pixel_bit_index % 8u;
            uint32_t shifted_mask = pixel_mask << (8u - bit_offset);
            uint32_t shifted_bits = pixel_bits << (8u - bit_offset);
            uint32_t buffer_bytes = SCREEN_WIDTH_IN_BYTES - pixel_bit_index / 8u;

            if (buffer_bytes > 4u)
            {
                buffer_bytes = 4u;
            }

            for (uint32_t byte = 0; byte < buffer_bytes; ++byte)
            {
                uint32_t shift = 24u - byte * 8u;
                uint8_t byte_mask = (uint8_t)(shifted_mask >> shift);
                uint8_t byte_bits = (uint8_t)(shifted_bits >> shift);

                screen_buffer[buffer_index + byte] =
                    (screen_buffer[buffer_index + byte] & (uint8_t)~byte_mask) |
                    (byte_bits & byte_mask);
            }
        }
    }
}

void MIP_8c_Display_DrawText(uint16_t row, uint8_t color, const char *text)
{
    if (text == NULL || row >= SCREEN_HEIGHT_IN_PIXELS)
    {
        return;
    }

    uint32_t x_position = 0;
    bool previous_was_letter = false;

    for (size_t text_index = 0; text[text_index] != '\0'; ++text_index)
    {
        char character = text[text_index];
        if (character == ' ')
        {
            x_position += SPACE_IN_PIXELS;
            previous_was_letter = false;
            continue;
        }

        if (character < '!' || character > '}')
        {
            continue;
        }

        if (previous_was_letter)
        {
            x_position += CHARACTERS_SEPARATOR_IN_PIXELS;
        }
        if (x_position >= SCREEN_WIDTH_IN_PIXELS)
        {
            break;
        }

        uint8_t letter_index = (uint8_t)(character - INDEX_OF_FIRST_CHAR);
        uint8_t letter_width = letters_width[letter_index];
        uint8_t letter_height = letters_height[letter_index];

        for (uint8_t glyph_row = 0; glyph_row < letter_height; ++glyph_row)
        {
            uint32_t screen_y = (uint32_t)row + glyph_row;
            if (screen_y >= SCREEN_HEIGHT_IN_PIXELS)
            {
                break;
            }

            for (uint8_t glyph_column = 0; glyph_column < letter_width; ++glyph_column)
            {
                if (x_position + glyph_column >= SCREEN_WIDTH_IN_PIXELS)
                {
                    break;
                }
                
                // Background pixels are not changed. Whatever color is already in the buffer will remain.
                if (letters_bin[letter_index][glyph_row][glyph_column] == 0)
                {
                    continue;
                }

                uint32_t pixel_bit_index = x_position * BITS_PER_PIXEL;
                pixel_bit_index += glyph_column * BITS_PER_PIXEL;
                uint32_t buffer_index = screen_y * SCREEN_WIDTH_IN_BYTES + pixel_bit_index / 8u;
                uint8_t bit_offset = (uint8_t)(pixel_bit_index % 8u);

                for (uint8_t bit = 0; bit < BITS_PER_PIXEL; ++bit)
                {
                    uint32_t destination_bit = bit_offset + bit;
                    uint8_t mask = (uint8_t)(1u << (7u - destination_bit % 8u));
                    uint8_t color_bit = (uint8_t)((color >> (BITS_PER_PIXEL - 1u - bit)) & 1u);
                    uint32_t destination_byte = buffer_index + destination_bit / 8u;

                    if (color_bit != 0)
                    {
                        screen_buffer[destination_byte] |= mask;
                    }
                    else
                    {
                        screen_buffer[destination_byte] &= (uint8_t)~mask;
                    }
                }
            }
        }

        x_position += letter_width;
        previous_was_letter = true;
    }
}




void MIP_8c_Display_RefreshScreen(uint8_t line_start, uint8_t line_end)
{
    //  printf("-- MIP_8c_Display_RefreshScreen \n");


    int length_of_buffer = 1 + (line_end - line_start) * (1 + SCREEN_WIDTH_IN_BYTES + 1) + 1;
    uint8_t buf[length_of_buffer];
    int buf_iterator = 0;

    /*
	 * Buffer structure:
	 * - 6 bit - command + VCOM bit
	 * - for each line:
	 *   	10 bits - line address (little-endian), first 2 bits always 00 for 176px height
	 *   	3 x 176 bits - pixel data for RGB
	 *   	6 bits - end of line trailer (0x00)
	 * - 16 bits - end of transmission trailer (0x00)
	 */

//    Command "byte". Only first 6 bits will be used as command.
//    Last 2 bits are first 2 bits of line address of first line. For 176 px heigh it's always 00.
    if(vcom_bool_)
    {
        buf[0] = 0b11000000;
        vcom_bool_ = false;
    }
    else
    {
        buf[0] = 0b10000000;
        vcom_bool_ = true;
    }
    buf_iterator++;

//    Address and pixel data for each line. Each line consists of:
//    - 10 bits line address (little-endian). The first 2 bits of address
//      will overwrite last 2 bits of previous byte (command for line_start or line trailer for other lines).
//    	In case of display with 176 pixels height, first 2 bits are always 00, so no action is needed.
//    - 3 x 176 bits pixel data. R1-G1-B1, R2-G2-B2, ..., R176-G176-B176.
//    - 6 bits end of line trailer (0x00)

    for (uint8_t i = line_start; i < line_end; i++)
    {
//    	Address
        buf[buf_iterator] = i;    //line address
        buf_iterator++;

//      Pixel data
        for (size_t j = 0; j < SCREEN_WIDTH_IN_BYTES; ++j)
        {
            buf[buf_iterator] = screen_buffer[i * SCREEN_WIDTH_IN_BYTES + j];
            buf_iterator++;
        }

//      End of line trailer
        buf[buf_iterator] = 0b00000000;
        buf_iterator++;
    }

    buf[buf_iterator] = 0b00000000;     //end transmission trailer

    
    
    HAL_GPIO_WritePin(display_config.cs_port, display_config.cs_pin, GPIO_PIN_SET);	// this display is low on inactive
    HAL_StatusTypeDef status = HAL_SPI_Transmit(display_config.hspi, buf, length_of_buffer, HAL_MAX_DELAY);
    if(status != HAL_OK)
    {
        
    }
    HAL_GPIO_WritePin(display_config.cs_port, display_config.cs_pin, GPIO_PIN_RESET);
}

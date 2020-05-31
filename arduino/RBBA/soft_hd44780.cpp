#include <Arduino.h>
#include "soft_hd44780.h"

////////////////////////////////////////////////////////////////////////////////
// defines taken from https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung

#define LCD_DDADR_LINE1         0x00
#define LCD_DDADR_LINE2         0x40
#define LCD_DDADR_LINE3         0x14
#define LCD_DDADR_LINE4         0x54

// some delays
#define LCD_BOOTUP_MS           15
#define LCD_ENABLE_US           20
#define LCD_WRITEDATA_US        46
#define LCD_COMMAND_US          42
 
#define LCD_SOFT_RESET_MS1      5
#define LCD_SOFT_RESET_MS2      1
#define LCD_SOFT_RESET_MS3      1
#define LCD_SET_4BITMODE_MS     5
 
#define LCD_CLEAR_DISPLAY_MS    2
#define LCD_CURSOR_HOME_MS      2

// Clear Display -------------- 0b00000001
#define LCD_CLEAR_DISPLAY       0x01
 
// Cursor Home ---------------- 0b0000001x
#define LCD_CURSOR_HOME         0x02
 
// Set Entry Mode ------------- 0b000001xx
#define LCD_SET_ENTRY           0x04
 
#define LCD_ENTRY_DECREASE      0x00
#define LCD_ENTRY_INCREASE      0x02
#define LCD_ENTRY_NOSHIFT       0x00
#define LCD_ENTRY_SHIFT         0x01
 
// Set Display ---------------- 0b00001xxx
#define LCD_SET_DISPLAY         0x08
 
#define LCD_DISPLAY_OFF         0x00
#define LCD_DISPLAY_ON          0x04
#define LCD_CURSOR_OFF          0x00
#define LCD_CURSOR_ON           0x02
#define LCD_BLINKING_OFF        0x00
#define LCD_BLINKING_ON         0x01
 
// Set Shift ------------------ 0b0001xxxx
#define LCD_SET_SHIFT           0x10
 
#define LCD_CURSOR_MOVE         0x00
#define LCD_DISPLAY_SHIFT       0x08
#define LCD_SHIFT_LEFT          0x00
#define LCD_SHIFT_RIGHT         0x04
 
// Set Function --------------- 0b001xxxxx
#define LCD_SET_FUNCTION        0x20
 
#define LCD_FUNCTION_4BIT       0x00
#define LCD_FUNCTION_8BIT       0x10
#define LCD_FUNCTION_1LINE      0x00
#define LCD_FUNCTION_2LINE      0x08
#define LCD_FUNCTION_5X7        0x00
#define LCD_FUNCTION_5X10       0x04
 
#define LCD_SOFT_RESET          0x30
 
// Set CG RAM Address --------- 0b01xxxxxx  (Character Generator RAM)
#define LCD_SET_CGADR           0x40
 
#define LCD_GC_CHAR0            0
#define LCD_GC_CHAR1            1
#define LCD_GC_CHAR2            2
#define LCD_GC_CHAR3            3
#define LCD_GC_CHAR4            4
#define LCD_GC_CHAR5            5
#define LCD_GC_CHAR6            6
#define LCD_GC_CHAR7            7
 
// Set DD RAM Address --------- 0b1xxxxxxx  (Display Data RAM)
#define LCD_SET_DDADR           0x80

SoftHD44780::SoftHD44780 (
  const uint8_t  address,
  const uint8_t  scl_pin, 
  const uint8_t  sda_pin, 
  const uint16_t speed_khz 
) :
  SoftI2C(scl_pin,sda_pin,speed_khz),
  slave_addr(address),
  led_bg(8),
  data_out_buf{0,0,0}
{
  
}

void SoftHD44780::data_out(uint8_t data)
{
  data_out_buf[0] = data & ~4 | led_bg;
  data_out_buf[1] = data |  4 | led_bg;
  data_out_buf[2] = data & ~4 | led_bg;

  transmit(slave_addr,data_out_buf,3);
}

void SoftHD44780::command(uint8_t value)
{
  data_out(value  & 0xF0);
  data_out(value <<    4);
}

void SoftHD44780::put_data(uint8_t value)
{
  data_out((value  & 0xF0) | 1);
  data_out((value <<    4) | 1);
}

void SoftHD44780::init()
{
  delay( LCD_BOOTUP_MS );
  
  // 3 times soft reset
  data_out( LCD_SOFT_RESET );
  delay( LCD_SOFT_RESET_MS1 );
  
  data_out( LCD_SOFT_RESET );
  delay( LCD_SOFT_RESET_MS2 );
  
  data_out( LCD_SOFT_RESET );
  delay( LCD_SOFT_RESET_MS3 );
  
  // 4-bit mode
  data_out( LCD_SET_FUNCTION | LCD_FUNCTION_4BIT );    
  delay( LCD_SET_4BITMODE_MS );
  
  // 4-bit mode / 2 lines (actually 4x20, but internally 2x40) / 5x7
  command( LCD_SET_FUNCTION |
           LCD_FUNCTION_4BIT |
           LCD_FUNCTION_2LINE |
           LCD_FUNCTION_5X7 );
  
  // Display on / Cursor off / Blinking off
  command( LCD_SET_DISPLAY |
           LCD_DISPLAY_ON |
           LCD_CURSOR_OFF |
           LCD_BLINKING_OFF); 
  
  // Cursor increment / no scrolling
  command( LCD_SET_ENTRY |
           LCD_ENTRY_INCREASE |
           LCD_ENTRY_NOSHIFT );
  
  clear();  
}

void SoftHD44780::clear()
{
  command( LCD_CLEAR_DISPLAY );
  delay( LCD_CLEAR_DISPLAY_MS );  
}

void SoftHD44780::home()
{
  command( LCD_CURSOR_HOME );
  delay( LCD_CURSOR_HOME_MS );  
}

void SoftHD44780::set_cursor(const uint8_t x, const uint8_t y)
{
  uint8_t data;

  switch (y)
  {
    case 0:    // 1. line
        data = LCD_SET_DDADR + LCD_DDADR_LINE1 + x;
        break;

    case 1:    // 2. line
        data = LCD_SET_DDADR + LCD_DDADR_LINE2 + x;
        break;

    case 2:    // 3. line
        data = LCD_SET_DDADR + LCD_DDADR_LINE3 + x;
        break;

    case 3:    // 4. line
        data = LCD_SET_DDADR + LCD_DDADR_LINE4 + x;
        break;

    default:
        return;
  }

  command( data );  
}

void SoftHD44780::print( const char *data )
{
  while( *data != '\0' )
    put_data( *data++ );
}

void SoftHD44780::set_LED_background(const bool on)
{
  led_bg = on ? 8 : 0;
}

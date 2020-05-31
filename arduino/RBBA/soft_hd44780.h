#ifndef SOFT_HD44780_H
#define SOFT_HD44780_H

/*
  LCD HD44780 control via I2C module

  code inspired by https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/LCD-Ansteuerung
  
*/

#include <stdint.h>
#include "soft_i2c.h"

class SoftHD44780 : protected SoftI2C {

  const uint8_t slave_addr;

  uint8_t led_bg;

public:  

  SoftHD44780(
    const uint8_t  address,
    const uint8_t  scl_pin, 
    const uint8_t  sda_pin, 
    const uint16_t speed_khz // max 250khz
  );

  void command(uint8_t value);

  void put_data(uint8_t value);

  void init();

  void clear();

  void home();

  void set_cursor(const uint8_t x, const uint8_t y);

  void print(const char *data);

  void set_LED_background(const bool on);

private:

  uint8_t data_out_buf[3];

  void data_out(uint8_t data);

};

#endif

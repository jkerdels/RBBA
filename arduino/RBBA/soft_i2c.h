#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdint.h>

class SoftI2C {

  const uint8_t scl_pin;
  const uint8_t sda_pin;
  const uint8_t quarter_delay_uS;

public:  

  SoftI2C(
    const uint8_t  _scl_pin, 
    const uint8_t  _sda_pin, 
    const uint16_t speed_khz // max 250khz
  );

  bool transmit( // returns true if succesful
    uint8_t slave_address,
    uint8_t *data,
    uint8_t data_size
  );
   
  bool receive( // returns true if succesful
    uint8_t slave_address,
    uint8_t *data,
    uint8_t data_size    
  );  

  bool transceive_wr( // returns true if succesful
    uint8_t slave_address,
    uint8_t *write_data,
    uint8_t write_data_size,
    uint8_t *read_data,
    uint8_t read_data_size
  );

  bool transceive_rw( // returns true if succesful
    uint8_t slave_address,
    uint8_t *read_data,
    uint8_t read_data_size,
    uint8_t *write_data,
    uint8_t write_data_size
  );

private:

  void scl_low();
  void scl_high();
  void sda_low();
  void sda_high();

  void    write_bit(uint8_t bit_val);
  uint8_t read_bit();

  void send_stop();
  void send_start();

  uint8_t send_byte(uint8_t value);
  uint8_t read_byte(uint8_t ack);
  
};

#endif

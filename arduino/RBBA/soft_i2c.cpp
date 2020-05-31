#include <Arduino.h>
#include "soft_i2c.h"

SoftI2C::SoftI2C(
  const uint8_t  _scl_pin, 
  const uint8_t  _sda_pin, 
  const uint16_t speed_khz
) :
  scl_pin(_scl_pin),
  sda_pin(_sda_pin),
  quarter_delay_uS((uint16_t)1000 / speed_khz / (uint16_t)4)
{
  pinMode(scl_pin,INPUT_PULLUP);
  pinMode(sda_pin,INPUT_PULLUP);  
}

void SoftI2C::scl_low()
{
  pinMode(scl_pin,OUTPUT);
  digitalWrite(scl_pin,LOW);
}

void SoftI2C::scl_high()
{
  pinMode(scl_pin,INPUT_PULLUP);
}

void SoftI2C::sda_low()
{
  pinMode(sda_pin,OUTPUT);
  digitalWrite(sda_pin,LOW);  
}

void SoftI2C::sda_high()
{
  pinMode(sda_pin,INPUT_PULLUP);
}

void SoftI2C::send_stop()
{
    scl_low();
    delayMicroseconds(quarter_delay_uS);
    sda_low();
    delayMicroseconds(quarter_delay_uS);
    scl_high();
    delayMicroseconds(quarter_delay_uS);
    sda_high();
    delayMicroseconds(quarter_delay_uS);
}

void SoftI2C::send_start()
{
  delayMicroseconds(quarter_delay_uS);
  sda_low();
  delayMicroseconds(quarter_delay_uS);  
}

void SoftI2C::write_bit(uint8_t bit_val)
{
    scl_low();
    delayMicroseconds(quarter_delay_uS);
    if (bit_val) {
      sda_high();
    } else {
      sda_low();
    }
    delayMicroseconds(quarter_delay_uS);
    scl_high();
    delayMicroseconds(quarter_delay_uS*2);
}

uint8_t SoftI2C::read_bit()
{
  uint8_t result = 0;
  scl_low();
  delayMicroseconds(quarter_delay_uS);
  sda_high();
  delayMicroseconds(quarter_delay_uS);
  scl_high();
  delayMicroseconds(quarter_delay_uS);
  result = digitalRead(sda_pin);
  delayMicroseconds(quarter_delay_uS);
  return result;
}

uint8_t SoftI2C::send_byte(uint8_t value)
{
  for (int8_t b = 7; b >= 0; --b) {
    write_bit((value >> b) & 1);
  }
  return read_bit();
}

uint8_t SoftI2C::read_byte(uint8_t ack)
{
  uint8_t result = 0;
  for (int8_t b = 0; b < 8; ++b) {
    result = (result << 1) | read_bit();
  }
  write_bit(ack);
  return result;
}

bool SoftI2C::transmit( // returns true if succesful
  uint8_t slave_address,
  uint8_t *data,
  uint8_t data_size)
{
  // issue start condition
  send_start();
  // write 7 bit address, MSB first, plus write bit
  if (send_byte(slave_address << 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // send data
  uint8_t *end = data + data_size;
  while (data != end) {
    if (send_byte(*data) == 1) {
      // nack received, send a stop condition
      send_stop();
      return false;
    }
    ++data;
  }
  send_stop();
  return true;
}
 
bool SoftI2C::receive( // returns true if succesful
  uint8_t slave_address,
  uint8_t *data,
  uint8_t data_size)
{
  // issue start condition
  send_start();
  // write 7 bit address, MSB first, plus read bit
  if (send_byte((slave_address << 1) | 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // read data
  uint8_t *end  = data + data_size;
  uint8_t *last = end - 1;
  while (data != end) {
    *data = read_byte(data == last);
    ++data;
  }
  send_stop();
  return true;  
}

bool SoftI2C::transceive_wr( // returns true if succesful
  uint8_t slave_address,
  uint8_t *write_data,
  uint8_t write_data_size,
  uint8_t *read_data,
  uint8_t read_data_size)
{
  // issue start condition
  send_start();
  // write 7 bit address, MSB first, plus write bit
  if (send_byte(slave_address << 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // send data
  uint8_t *end = write_data + write_data_size;
  while (write_data != end) {
    if (send_byte(*write_data) == 1) {
      // nack received, send a stop condition
      send_stop();
      return false;
    }
    ++write_data;
  }
  // send repeated start
  send_start();
  // write 7 bit address, MSB first, plus read bit
  if (send_byte((slave_address << 1) | 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // read data
  end = read_data + read_data_size;
  uint8_t *last = end - 1;
  while (read_data != end) {
    *read_data = read_byte(read_data == last);
    ++read_data;
  }
  send_stop();
  return true;  
}

bool SoftI2C::transceive_rw( // returns true if succesful
  uint8_t slave_address,
  uint8_t *read_data,
  uint8_t read_data_size,
  uint8_t *write_data,
  uint8_t write_data_size)
{
  // issue start condition
  send_start();
  // write 7 bit address, MSB first, plus read bit
  if (send_byte((slave_address << 1) | 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // read data
  uint8_t *end  = read_data + read_data_size;
  uint8_t *last = end - 1;
  while (read_data != end) {
    *read_data = read_byte(read_data == last);
    ++read_data;
  }
  // send repeated start
  send_start();
  // write 7 bit address, MSB first, plus write bit
  if (send_byte(slave_address << 1) == 1) {
    // nack received, send a stop condition
    send_stop();
    return false;
  }
  // send data
  end = write_data + write_data_size;
  while (write_data != end) {
    if (send_byte(*write_data) == 1) {
      // nack received, send a stop condition
      send_stop();
      return false;
    }
    ++write_data;
  }
  send_stop();
  return true;  
}

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

class Encoder {

  const uint8_t enc_pin_A;
  const uint8_t enc_pin_B;

  volatile int16_t encoder_value;
  volatile uint8_t prev_enc_pin_A;

public:
  Encoder(const uint8_t _enc_pin_A, const uint8_t _enc_pin_B);

  void update();

  void set_value(int16_t value);

  int16_t get_value();
  
};


#endif

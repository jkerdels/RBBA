#include <Arduino.h>
#include "encoder.h"

Encoder::Encoder(const uint8_t _enc_pin_A, const uint8_t _enc_pin_B) :
  enc_pin_A(_enc_pin_A),
  enc_pin_B(_enc_pin_B)
{
  pinMode(enc_pin_A,INPUT_PULLUP);
  pinMode(enc_pin_B,INPUT_PULLUP);

  encoder_value  = 0;
  prev_enc_pin_A = digitalRead(enc_pin_A);
}
  
void Encoder::update()
{
    const uint8_t val_A = digitalRead(enc_pin_A);
    const uint8_t val_B = digitalRead(enc_pin_B);

    encoder_value += (val_A ^ prev_enc_pin_A) * (1 - ((val_A ^ val_B) << 1));
    
    prev_enc_pin_A = val_A;  
}

void Encoder::set_value(int16_t value) {
  noInterrupts();
  encoder_value = value;
  interrupts();
}

int16_t Encoder::get_value() {
  noInterrupts();
  int16_t result = encoder_value;
  interrupts();
  return result;
}

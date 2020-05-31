#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include "text_buffer.h"

#define USE_TIMER2_OC2B

class Encoder;

class MotorControl {

const uint8_t PWM_epsilon        = 16;
const int8_t  encoder_reversal   =  1;
const uint8_t direction_reversal =  0;
const int8_t  speed_p            =  6;

const uint8_t position_epsilon = 5;
const uint8_t position_p       = 4;

const uint8_t motor_enable_pin; 
const uint8_t motor_direction_pin_A;
const uint8_t motor_direction_pin_B;
const uint8_t end_stop_opened_pin;
const uint8_t end_stop_closed_pin;

const uint16_t update_interval_uS;

Encoder &encoder;

int16_t last_encoder_value;
int16_t set_speed;
int16_t cur_PWM;
uint8_t cur_dir;

int16_t  set_position;
int16_t  max_speed;
int16_t  max_enc_value;

bool raw_mode;

public:
  const uint8_t home_PWM = 100;

  MotorControl(
    const uint8_t   en_pin, 
    const uint8_t   dir_pin_A, 
    const uint8_t   dir_pin_B,
    const uint8_t   es_opened_pin,
    const uint8_t   es_closed_pin,
    const uint16_t  upd_interval_uS,
    Encoder  &enc
  );

  void update();

  void hard_stop();

  void home();

  void calibrate();

  void move_const_time(const uint8_t pos, const uint16_t duration_mS);
  void move_const_speed(const uint8_t pos, const uint8_t speed);

  int16_t get_max_encoder() const;
  void set_max_encoder(const int16_t value);

  bool open_end_stop()  { return (digitalRead(end_stop_opened_pin) == LOW); } 
  bool close_end_stop() { return (digitalRead(end_stop_closed_pin) == LOW); } 

  void move_raw(const uint8_t pwm, const uint8_t dir);
  void zero_enc() { encoder.set_value(0); }
  void max_enc() { max_enc_value = encoder.get_value() * (int16_t)encoder_reversal; }

  int16_t get_encoder_value() { return encoder.get_value() * (int16_t)encoder_reversal; }

private:

  void set_pwm(const uint8_t pwm);

  void    set_direction(const uint8_t dir);
  uint8_t get_direction() const;
  
};



#endif

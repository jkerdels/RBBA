#include <Arduino.h>
#include "encoder.h"
#include "motor_control.h"

MotorControl::MotorControl(    
  const uint8_t   en_pin, 
  const uint8_t   dir_pin_A, 
  const uint8_t   dir_pin_B,
  const uint8_t   es_opened_pin,
  const uint8_t   es_closed_pin,
  const uint16_t  upd_interval_uS,
  Encoder  &enc
) :
  motor_enable_pin(en_pin),
  motor_direction_pin_A(dir_pin_A),
  motor_direction_pin_B(dir_pin_B),
  end_stop_opened_pin(es_opened_pin),
  end_stop_closed_pin(es_closed_pin),
  update_interval_uS(upd_interval_uS),
  encoder(enc),
  last_encoder_value(0),
  set_speed(0),
  cur_PWM(0),
  cur_dir(1),
  set_position(0),
  max_speed(0),
  max_enc_value(0),
  raw_mode(false)
{
  pinMode(motor_enable_pin,OUTPUT);
  digitalWrite(motor_enable_pin,LOW);
  pinMode(motor_direction_pin_A,OUTPUT);
  digitalWrite(motor_direction_pin_A,LOW);
  pinMode(motor_direction_pin_B,OUTPUT);
  digitalWrite(motor_direction_pin_B,HIGH);

  // end switches
  pinMode(end_stop_opened_pin,INPUT_PULLUP);
  pinMode(end_stop_closed_pin,INPUT_PULLUP);

  // setup timer2 to get rid of the awful 1khz noise of analogWrite
#ifdef USE_TIMER2_OC2B  
  PRR &= ~(1 << 6);
  OCR2B = 0;
  TCCR2A = 1 << 5 | 1;
  TCCR2B = 1; // prescaler of 1
#endif    
}

void MotorControl::set_pwm(const uint8_t pwm)
{
#ifdef USE_TIMER2_OC2B    
    OCR2B = pwm;
#else    
    analogWrite(motor_enable_pin,pwm);
#endif        
}

void MotorControl::set_direction(const uint8_t dir)
{
  set_pwm(0);
  if (dir == direction_reversal) {
    digitalWrite(motor_direction_pin_A,HIGH);
    digitalWrite(motor_direction_pin_B,LOW);
  } else {
    digitalWrite(motor_direction_pin_A,LOW);
    digitalWrite(motor_direction_pin_B,HIGH);
  }
  cur_dir = dir;
}

void MotorControl::move_raw(const uint8_t pwm, const uint8_t dir)
{
  raw_mode = true;
  set_direction(dir);
  set_pwm(pwm);
}

uint8_t MotorControl::get_direction() const
{
  return cur_dir;
}

void MotorControl::hard_stop()
{
  raw_mode = false;
  set_pwm(0);
  cur_PWM   = 0;
  set_speed = 0;
  last_encoder_value = encoder.get_value() * (int16_t)encoder_reversal;
}

void MotorControl::home()
{
  if (digitalRead(end_stop_opened_pin) == LOW) {
    set_direction(1);
    set_pwm(home_PWM);
    while (digitalRead(end_stop_opened_pin) == LOW);
    delay(200);
  }
  set_direction(0);
  set_pwm(home_PWM);
  while (digitalRead(end_stop_opened_pin));
  hard_stop();
  encoder.set_value(0);    
  last_encoder_value = 0;
}

void MotorControl::calibrate()
{
  if (digitalRead(end_stop_opened_pin) == LOW) {
    set_direction(1);
    set_pwm(home_PWM);
    while (digitalRead(end_stop_opened_pin) == LOW);
    delay(200);
  }
  set_direction(0);
  set_pwm(home_PWM);
  while (digitalRead(end_stop_opened_pin));
  hard_stop();
  encoder.set_value(0);
  last_encoder_value = 0;
  set_direction(1);
  set_pwm(home_PWM);
  while (digitalRead(end_stop_closed_pin));
  hard_stop();
  max_enc_value = encoder.get_value() * (int16_t)encoder_reversal;
  last_encoder_value = max_enc_value;
}

void MotorControl::move_const_time(const uint8_t pos, const uint16_t duration_mS)
{
  set_position         = (int32_t)max_enc_value * (int32_t)pos / (int32_t)100;
  int16_t cur_position = encoder.get_value() * (int16_t)encoder_reversal;
  max_speed            = (int32_t)abs(set_position - cur_position) * (int32_t)1000 / (int32_t)duration_mS;
}

void MotorControl::move_const_speed(const uint8_t pos, const uint8_t speed)
{
  set_position         = (int32_t)max_enc_value * (int32_t)pos / (int32_t)100;
  max_speed            = (int32_t)max_enc_value * (int32_t)speed / (int32_t)100;
}

void MotorControl::update() 
{
  if (raw_mode)
    return;
  // position control section
  int16_t cur_enc_value = encoder.get_value() * (int16_t)encoder_reversal;
  int16_t cur_delta = set_position - cur_enc_value;

  if (abs(cur_delta) < position_epsilon) {
    cur_delta = 0;
  }

  set_speed = cur_delta * position_p;
  if (set_speed >  max_speed) set_speed =  max_speed;
  if (set_speed < -max_speed) set_speed = -max_speed;
  
  // speed control section
  cur_delta = cur_enc_value - last_encoder_value;

  // speed in encoder ticks per second
  int16_t cur_speed = (int16_t)((int32_t)1000000 / (int32_t)update_interval_uS) * cur_delta;
  
  int16_t speed_delta = set_speed - cur_speed;

  cur_PWM += speed_delta * (int16_t)speed_p / (int16_t)4;

  const int16_t k = 32;
  if (cur_PWM >  (int16_t)255*k) cur_PWM =  (int16_t)255*k;
  if (cur_PWM < (int16_t)-255*k) cur_PWM = (int16_t)-255*k;
  
  if ((cur_PWM < 0) && (get_direction() == 1)) {
    set_direction(0);
  } else
  if ((cur_PWM > 0) && (get_direction() == 0)) {
    set_direction(1);
  }

  uint8_t pwm_out = abs(cur_PWM / k);
  if (((set_speed < 0) && (digitalRead(end_stop_opened_pin) == LOW)) ||
      ((set_speed > 0) && (digitalRead(end_stop_closed_pin) == LOW)) ||
      (pwm_out < PWM_epsilon) ||
      (set_speed == 0))
  {
    set_pwm(0);
  } else {
    set_pwm(pwm_out);
  }

  last_encoder_value = cur_enc_value;
}

int16_t MotorControl::get_max_encoder() const
{
  return max_enc_value;
}

void MotorControl::set_max_encoder(const int16_t value)
{
  max_enc_value = value;
}

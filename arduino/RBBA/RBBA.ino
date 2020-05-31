/* Overview:

*/

#include <stdint.h>
#include <EEPROM.h>
#include "encoder.h"
#include "motor_control.h"
#include "soft_bmp280.h"
#include "soft_hd44780.h"
#include "lcd_menu.h"
#include "text_buffer.h"
#include "state_machine.h"
#include "string_constants.h"

// configuration
const uint8_t motor_enable_pin      = 3; // OC2B of timer2
const uint8_t motor_direction_pin_A = 4;
const uint8_t motor_direction_pin_B = 5;
const uint8_t end_stop_opened_pin   = 7;
const uint8_t end_stop_closed_pin   = 8;
const uint8_t motor_encoder_pin_A   = 2; // needs to be on an interrupt pin
const uint8_t motor_encoder_pin_B   = 6;

const uint8_t control_loop_delay = 20;

const uint8_t p1_slave_address = 0x77;
const uint8_t p1_scl_pin       = A0;
const uint8_t p1_sda_pin       = A1;
const uint8_t p1_speed_khz     = 200;

const uint8_t lcd_slave_address = 0x27;
const uint8_t lcd_scl_pin       = 11;
const uint8_t lcd_sda_pin       = 12;
const uint8_t lcd_speed_khz     = 100;

const uint8_t knob_encoder_pin_A = A4;
const uint8_t knob_encoder_pin_B = A3;
const uint8_t knob_push_pin      = A5;

// implementation

Encoder encoder(motor_encoder_pin_A, motor_encoder_pin_B);

MotorControl mc(
  motor_enable_pin,
  motor_direction_pin_A,
  motor_direction_pin_B,
  end_stop_opened_pin,
  end_stop_closed_pin,
  (uint16_t)control_loop_delay*(uint16_t)1000,
  encoder
);

SoftBMP280 p1(p1_slave_address,p1_scl_pin,p1_sda_pin,p1_speed_khz);

SoftHD44780 lcd(lcd_slave_address,lcd_scl_pin,lcd_sda_pin,lcd_speed_khz);

Encoder knob_encoder(knob_encoder_pin_A, knob_encoder_pin_B);

// global state
TextBufferImpl<20> tbuf;

uint8_t patient_height_cm = 175;

uint16_t bag_vol_calib_data[9];

bool run_motor_encoder_calibration = false;
int16_t mc_calibrate_enc = 0;

bool run_bag_volume_calibration = false;
uint8_t bag_vol_calib_step = 1;
bool bag_vol_calib_step_go = false;
uint16_t bag_vol_value = 0;
bool bag_vol_calib_next = false;


// config storage
void store_calibration() {
  mc_calibrate_enc = mc.get_max_encoder();
  EEPROM.put(0,mc_calibrate_enc);
  for(uint8_t i = 0; i < 9; ++i) {
    uint16_t tmp = bag_vol_calib_data[i];
    EEPROM.put(2+i*2,tmp);
  }
}

void load_calibration() {
  EEPROM.get(0,mc_calibrate_enc);
  mc.set_max_encoder(mc_calibrate_enc);
  for(uint8_t i = 0; i < 9; ++i) {
    uint16_t tmp;
    EEPROM.get(2+i*2,tmp);
    bag_vol_calib_data[i] = tmp;
  }  
}


enum class panel_id : uint8_t {
  MAIN_SCREEN,
  NEW_PATIENT_CONFIG,
  CALIBRATION_MAIN,
  ABOUT,
  ENCODER_CAL,
  BAG_CAL_1,
  BAG_CAL_2
};

// setting up menu
auto lcd_menu = make_menu(
  lcd,
  knob_encoder,
  knob_push_pin,
  panel_id::MAIN_SCREEN,
  // panel 0, main panel
  Panel<panel_id>({ 
    panel_id::MAIN_SCREEN,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(1,0,str_RBBA),
        new LCDMenuTextElement<panel_id>(15,0,str_v10),
        new LCDMenuTextElement<panel_id>(2,1,str_nPatient,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::NEW_PATIENT_CONFIG);
        }),
        new LCDMenuTextElement<panel_id>(2,2,str_Calibration,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::CALIBRATION_MAIN);
        }),
        new LCDMenuTextElement<panel_id>(2,3,str_About,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::ABOUT);
        })
      );
    }
  }),
  // panel 1, new patient config
  Panel<panel_id>({ 
    panel_id::NEW_PATIENT_CONFIG,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(0,0,str_patient_data),
        new LCDMenuIntElement<panel_id,uint8_t,3,0>(1,1,&patient_height_cm,str_cm,8,true,110,855),
        new LCDMenuTextElement<panel_id>(14,3,str_back,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::MAIN_SCREEN);
        })
      );
    }
  }),
  // panel 2, calibration
  Panel<panel_id>({ 
    panel_id::CALIBRATION_MAIN,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(0,0,str_calibration),
        new LCDMenuTextElement<panel_id>(2,1,str_motor_encoder,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          run_motor_encoder_calibration = true;
          menu->switch_to_panel(panel_id::ENCODER_CAL);
        }),
        new LCDMenuTextElement<panel_id>(2,2,str_bag_volume,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          run_bag_volume_calibration = true;
          menu->switch_to_panel(panel_id::BAG_CAL_1);
        }),
        new LCDMenuTextElement<panel_id>(14,3,str_back,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::MAIN_SCREEN);
        })
      );
    }
  }),
  // panel 3, about
  Panel<panel_id>({ 
    panel_id::ABOUT,  
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(0,0,str_RBBA_about),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(0,1,bag_vol_calib_data,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(4,1,bag_vol_calib_data+1,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(8,1,bag_vol_calib_data+2,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(12,1,bag_vol_calib_data+3,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(16,1,bag_vol_calib_data+4,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(0,2,bag_vol_calib_data+5,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(4,2,bag_vol_calib_data+6,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(8,2,bag_vol_calib_data+7,str_comma),
        new LCDMenuIntElement<panel_id,uint16_t,3,0>(12,2,bag_vol_calib_data+8,str_empty),
        new LCDMenuIntElement<panel_id,int16_t,3,0>(0,3,&mc_calibrate_enc,str_empty),
        new LCDMenuTextElement<panel_id>(14,3,str_back,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          menu->switch_to_panel(panel_id::MAIN_SCREEN);
        })
      );
    }
  }),
  // panel 4, encoder calibration
  Panel<panel_id>({ 
    panel_id::ENCODER_CAL,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuBufferElement<panel_id>(0,0,tbuf.get_buffer()),
        new LCDMenuTextElement<panel_id>(0,1,str_max_enc),
        new LCDMenuIntElement<panel_id,int16_t,5,0>(9,1,&mc_calibrate_enc,str_empty,8),
        new LCDMenuTextElement<panel_id>(14,3,str_back,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          if (run_motor_encoder_calibration == false)
            menu->switch_to_panel(panel_id::CALIBRATION_MAIN);
        })
      );
    }
  }),
  // panel 5, bag calibration 1/2
  Panel<panel_id>({ 
    panel_id::BAG_CAL_1,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(0,0,str_bag_vol_calib),
        new LCDMenuTextElement<panel_id>(0,2,str_Step),
        new LCDMenuIntElement<panel_id,uint8_t,1,0>(6,2,&bag_vol_calib_step,str_of_nine,8),
        new LCDMenuTextElement<panel_id>(12,2,str_go,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          bag_vol_calib_step_go = true;
        })
      );
    }
  }),
  // panel 6, bag calibration 2/2
  Panel<panel_id>({ 
    panel_id::BAG_CAL_2,
    [](){
      return make_panel<panel_id>( 
        new LCDMenuTextElement<panel_id>(0,0,str_bag_vol_calib),
        new LCDMenuTextElement<panel_id>(0,2,str_enter_vol), 
        new LCDMenuIntElement<panel_id,uint16_t,4,0>(12,2,&bag_vol_value,str_ml,8,true,0,1000),
        new LCDMenuTextElement<panel_id>(10,3,str_ok,[](LCDMenuBase<panel_id> *menu, LCDMenuElement<panel_id> *element){
          bag_vol_calib_next = true;
        })
      );
    }
  })
);




// state machines
uint16_t mc_calibrate_dtime;

enum class mc_calibrate_state : uint8_t {
  start,
  clear_endswitch,
  move_home,
  move_max,
  finish,
  return_
};

auto mc_calibrate = make_state_machine(
  mc_calibrate_state::start,
  mc_calibrate_state::return_,

  make_state( mc_calibrate_state::start,  
  []() -> mc_calibrate_state {
    tbuf.put("moving to min");
    if (mc.open_end_stop()) {
      mc_calibrate_dtime = millis() & (uint32_t)1023;
      mc.move_raw(mc.home_PWM,1);
      return mc_calibrate_state::clear_endswitch;
    }
    return mc_calibrate_state::move_home;
  }),
  
  make_state( mc_calibrate_state::clear_endswitch,
  []() -> mc_calibrate_state {
    if (mc.open_end_stop())
      return mc_calibrate_state::clear_endswitch;
    uint16_t now = millis() & (uint32_t)1023;
    if (((now + 1024 - mc_calibrate_dtime) & 1023) > 200)
      return mc_calibrate_state::move_home;
    return mc_calibrate_state::clear_endswitch;
  }),

  make_state( mc_calibrate_state::move_home,
  []() -> mc_calibrate_state {
    mc.move_raw(mc.home_PWM,0);
    return mc_calibrate_state::move_max;
  }),
  
  make_state( mc_calibrate_state::move_max,
  []() -> mc_calibrate_state {
    if (mc.open_end_stop()) {
      tbuf.put("moving to max");
      mc.hard_stop();
      mc.zero_enc();
      mc_calibrate_enc = 0;
      mc.move_raw(mc.home_PWM,1);
      return mc_calibrate_state::finish;
    }
    return mc_calibrate_state::move_max;
  }),

  make_state( mc_calibrate_state::finish,
  []() -> mc_calibrate_state {
    mc_calibrate_enc = mc.get_encoder_value();
    if (mc.close_end_stop()) {
      tbuf.put("encoder cal done");
      mc.hard_stop();
      mc.max_enc();
      mc.move_const_speed(10,100);
      return mc_calibrate_state::return_;
    }
    return mc_calibrate_state::finish;
  })
);





enum class bag_calib_state : uint8_t {
  start,
  begin_step,
  end_step,
  return_
};

auto bag_calibrate = make_state_machine(
  bag_calib_state::start,
  bag_calib_state::return_,

  make_state( bag_calib_state::start,  
  []() -> bag_calib_state {
    bag_vol_calib_step = 1;
    bag_vol_value = 0;
    bag_vol_calib_step_go = false;
    bag_vol_calib_next = false;
    return bag_calib_state::begin_step;
  }),

  make_state( bag_calib_state::begin_step,  
  []() -> bag_calib_state {
    if (bag_vol_calib_step_go) {
      bag_vol_calib_step_go = false;
      lcd_menu.switch_to_panel(panel_id::BAG_CAL_2);
      mc.move_const_speed(10+bag_vol_calib_step*10,75);
      return bag_calib_state::end_step;
    }
    return bag_calib_state::begin_step;
  }),

  make_state( bag_calib_state::end_step,  
  []() -> bag_calib_state {
    if (bag_vol_calib_next) {
      bag_vol_calib_next = false;
      bag_vol_calib_data[bag_vol_calib_step-1] = bag_vol_value;
      if (bag_vol_calib_step < 9) {
        ++bag_vol_calib_step;
        lcd_menu.switch_to_panel(panel_id::BAG_CAL_1);
        mc.move_const_speed(10,75);
        return bag_calib_state::begin_step;
      }
      mc.move_const_speed(10,75);
      lcd_menu.switch_to_panel(panel_id::CALIBRATION_MAIN);
      return bag_calib_state::return_;
    }
    return bag_calib_state::end_step;
  })

);




ISR(PCINT1_vect)
{
  knob_encoder.update();
}


void setup() {
//  Serial.begin(115200);
  
  lcd.init();
  
  lcd.set_cursor(0,0);
  lcd.print("Initializing...");

  delay(1000);

  // attach encoder update to interrupt
  attachInterrupt(digitalPinToInterrupt(motor_encoder_pin_A),[](){encoder.update();},CHANGE);

  // attach knob encoder to interrupt (the avr way)
  PCICR  |= 2;
  PCMSK1 |= 16; // pin PC4 aka A4

  lcd.set_cursor(0,1);
  lcd.print("p1 init");
  // calibrate pressure sensor
  p1.load_calibration();
  
  // configure pressure sensor
  p1.set_temp_oversampling(SoftBMP280::Oversampling::x1);
  p1.set_pressure_oversampling(SoftBMP280::Oversampling::x4);
  p1.set_filter_coeff(SoftBMP280::FilterCoeff::off);
  p1.set_standby_time(SoftBMP280::StandbyTime::ms0_5);
  
  p1.set_mode(SoftBMP280::Mode::normal);

  load_calibration();

  // setting up timer1 for the main loop
  // in normal mode
  TCCR1A = 0;
  TCCR1B = 3; // prescaler of 64 -> 250 ticks per millisecond
  TCCR1C = 0;  
}

const uint16_t ticks_per_ms = 250;

uint8_t tmpcnt = 0;

void loop() {
  
  TCNT1 = 0;
  
  mc.update(); // measured as 15 ticks -> 60uS

  // lets make this first slot 100uS then (4uS per tick)
  while(TCNT1 < 100 / 4);

  p1.read_sensor_data();

  // this slot neets to be 1000 ticks long (4ms)
  while(TCNT1 < 1000);

  if ((run_motor_encoder_calibration) && (mc_calibrate.execute_step())) {
    run_motor_encoder_calibration = false;
    store_calibration();
  }

  if ((run_bag_volume_calibration) && (bag_calibrate.execute_step())) {
    run_bag_volume_calibration = false;
    store_calibration();
  }

  lcd_menu.update();
    
  while(TCNT1 < (const uint16_t)control_loop_delay * ticks_per_ms);

}

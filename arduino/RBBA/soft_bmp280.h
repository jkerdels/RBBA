#ifndef SOFT_BMP280_H
#define SOFT_BMP280_H

#include <stdint.h>
#include "soft_i2c.h"

class SoftBMP280 : protected SoftI2C {

  const uint8_t slave_addr;

  static const uint8_t calib_data_size = 24;

  union {
    uint8_t data[calib_data_size];
    struct {
      uint16_t t1;
      int16_t  t2;
      int16_t  t3;
      uint16_t p1;
      int16_t  p2;
      int16_t  p3;
      int16_t  p4;
      int16_t  p5;
      int16_t  p6;
      int16_t  p7;
      int16_t  p8;
      int16_t  p9;
    } coeff;
  } calibration;

  int32_t  latest_temp;
  uint32_t latest_pressure;

  int32_t adc_temp;
  int32_t adc_pressure;

public:

  enum class Mode : uint8_t {
    sleep,
    normal,
    forced,
    error
  };

  enum class Oversampling : uint8_t {
    skipped,
    x1,
    x2,
    x4,
    x8,
    x16,
    error
  };

  enum class FilterCoeff : uint8_t {
    off,
    c2,
    c4,
    c8,
    c16,
    error
  };

  enum class StandbyTime : uint8_t {
    ms0_5,
    ms62_5,
    ms125,
    ms250,
    ms500,
    ms1000,
    ms2000,
    ms4000,
    error
  };

  enum class Status : uint8_t {
    inactive,
    active,
    error
  };

  SoftBMP280(
    const uint8_t  address,
    const uint8_t  scl_pin, 
    const uint8_t  sda_pin, 
    const uint16_t speed_khz // max 250khz
  );

  bool load_calibration(); // returns true if successful

  Oversampling get_temp_oversampling();
  Oversampling get_pressure_oversampling();
  FilterCoeff  get_filter_coeff();
  Mode         get_mode();
  StandbyTime  get_standby_time();
  Status       is_measuring();
  Status       is_updating();

  void set_temp_oversampling(Oversampling os);
  void set_pressure_oversampling(Oversampling os);
  void set_filter_coeff(FilterCoeff fc);
  void set_mode(Mode m);
  void set_standby_time(StandbyTime t);
  
  void read_sensor_data();

  int32_t get_latest_temp() const { return latest_temp; }
  uint32_t get_latest_pressure() const { return latest_pressure; }

  int32_t get_latest_adc_temp() const { return adc_temp; }
  int32_t get_latest_adc_pressure() const { return adc_pressure; }

  // debug
  void print_calibration();
  void print_mode(Mode mode);
  void print_oversampling(Oversampling os);
  void print_filter_coeff(FilterCoeff fc);
  void print_standby_time(StandbyTime st);
  void print_status(Status st);

  void print_configuration();

private:

  int32_t  fine_temp; // intermediate result shared between compensate_temp and compensate_pressure
  int32_t  compensate_temp(int32_t u_temp);
  uint32_t compensate_pressure(int32_t u_press); // requires call to compensate_temp beforehand

};


#endif

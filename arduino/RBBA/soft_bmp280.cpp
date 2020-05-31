#include <Arduino.h>
#include "soft_bmp280.h"

SoftBMP280::SoftBMP280(
  const uint8_t  address,
  const uint8_t  scl_pin, 
  const uint8_t  sda_pin, 
  const uint16_t speed_khz 
) :
  SoftI2C(scl_pin,sda_pin,speed_khz),
  slave_addr(address),
  calibration(),
  latest_temp(0),
  latest_pressure(0),
  adc_temp(0),
  adc_pressure(0),
  fine_temp(0)
{}

bool SoftBMP280::load_calibration()
{
  uint8_t register_addr = 0x88;
  return transceive_wr(slave_addr,&register_addr,1,calibration.data,calib_data_size);  
}

void SoftBMP280::print_calibration()
{
  Serial.print("t1: "); Serial.println(calibration.coeff.t1);
  Serial.print("t2: "); Serial.println(calibration.coeff.t2);
  Serial.print("t3: "); Serial.println(calibration.coeff.t3);
  Serial.print("p1: "); Serial.println(calibration.coeff.p1);
  Serial.print("p2: "); Serial.println(calibration.coeff.p2);
  Serial.print("p3: "); Serial.println(calibration.coeff.p3);
  Serial.print("p4: "); Serial.println(calibration.coeff.p4);
  Serial.print("p5: "); Serial.println(calibration.coeff.p5);
  Serial.print("p6: "); Serial.println(calibration.coeff.p6);
  Serial.print("p7: "); Serial.println(calibration.coeff.p7);
  Serial.print("p8: "); Serial.println(calibration.coeff.p8);
  Serial.print("p9: "); Serial.println(calibration.coeff.p9);
}

int32_t SoftBMP280::compensate_temp(int32_t u_temp)
{
  int32_t var1, var2, T;
  var1 = ((((u_temp>>3) - ((int32_t)calibration.coeff.t1<<1))) * ((int32_t)calibration.coeff.t2)) >> 11;
  var2 = (((((u_temp>>4) - ((int32_t)calibration.coeff.t1)) * ((u_temp>>4) - ((int32_t)calibration.coeff.t1))) >> 12) *
  ((int32_t)calibration.coeff.t3)) >> 14;
  fine_temp = var1 + var2;
  T = (fine_temp * 5 + 128) >> 8;
  return T;  
  /*
  u_temp >>= 3;
  int32_t v1   = ((u_temp - ((int32_t)calibration.coeff.t1 << 1)) * (int32_t)calibration.coeff.t2) >> 11;
  
  u_temp >>= 1;
  int32_t u_t1 = u_temp - (int32_t)calibration.coeff.t1;
  int32_t v2   = (((u_t1 * u_t1) >> 12) * (int32_t)calibration.coeff.t3) >> 14;

  fine_temp = v1 + v2;
  return (fine_temp * 5 + 128) >> 8;
  */
}

// requires call to compensate_temp beforehand as it uses 
// the fine_temp intermediate result set by compensate_temp
uint32_t SoftBMP280::compensate_pressure(int32_t u_press)
{
  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)fine_temp)>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)calibration.coeff.p6);
  var2 = var2 + ((var1*((int32_t)calibration.coeff.p5))<<1);
  var2 = (var2>>2)+(((int32_t)calibration.coeff.p4)<<16);
  var1 = (((calibration.coeff.p3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)calibration.coeff.p2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((int32_t)calibration.coeff.p1))>>15);
  if (var1 == 0)
  {
  return 0; // avoid exception caused by division by zero
  }
  p = (((uint32_t)(((int32_t)1048576)-u_press)-(var2>>12)))*3125;
  if (p < 0x80000000)
  {
  p = (p << 1) / ((uint32_t)var1);
  }
  else
  {
  p = (p / (uint32_t)var1) * 2;
  }
  var1 = (((int32_t)calibration.coeff.p9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)calibration.coeff.p8))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + calibration.coeff.p7) >> 4));
  return p;

  /*
  int32_t v1   = (fine_temp >> 1) - (int32_t)64000;
  int32_t v1d4 = v1 >> 2;
  int32_t v2   = ((v1d4 * v1d4) >> 11) * (int32_t)calibration.coeff.p6;

  v2 += (v1 * (int32_t)calibration.coeff.p5) << 1;
  
  v2 = (v2 >> 2) + (int32_t)calibration.coeff.p4 << 16;

  v1 = ((((int32_t)calibration.coeff.p3 * ((v1d4 * v1d4) >> 13)) >> 3) + (((int32_t)calibration.coeff.p2 * v1) >> 1)) >> 18;

  v1 = (((int32_t)32768 + v1) * (int32_t)calibration.coeff.p1) >> 15;

  if (v1 == 0) return 0;

  uint32_t p = ((uint32_t)((int32_t)1048576 - u_press) - (v2 >> 12)) * (int32_t)3125;

  if (p < 0x80000000) {
    p = (p << 1) / (uint32_t)v1;
  } else {
    p = (p / (uint32_t)v1) * 2;
  }

  uint32_t pd8 = p >> 3;

  v1 = ((int32_t)calibration.coeff.p9 * (int32_t)((pd8 * pd8) >> 13)) >> 12;

  v2 = ((int32_t)(p >> 2) * (int32_t)calibration.coeff.p8) >> 13;

  return ((v1 + v2 + (int32_t)calibration.coeff.p7) >> 4) + (int32_t)p;
  */
}

SoftBMP280::Oversampling SoftBMP280::get_temp_oversampling() {
  uint8_t register_addr = 0xF4;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return Oversampling::error;
  }

  register_data >>= 5;
  switch (register_data) {
    case 0  : return Oversampling::skipped;
    case 1  : return Oversampling::x1;
    case 2  : return Oversampling::x2;
    case 3  : return Oversampling::x4;
    case 4  : return Oversampling::x8;
    case 5  : 
    case 6  : 
    case 7  : return Oversampling::x16;
    default : return Oversampling::error;
  }
  
  return Oversampling::error;
}

SoftBMP280::Oversampling SoftBMP280::get_pressure_oversampling() {
  uint8_t register_addr = 0xF4;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return Oversampling::error;
  }

  register_data = (register_data >> 2) & 7;
  switch (register_data) {
    case 0  : return Oversampling::skipped;
    case 1  : return Oversampling::x1;
    case 2  : return Oversampling::x2;
    case 3  : return Oversampling::x4;
    case 4  : return Oversampling::x8;
    case 5  : 
    case 6  : 
    case 7  : return Oversampling::x16;
    default : return Oversampling::error;
  }
  
  return Oversampling::error;  
}

SoftBMP280::FilterCoeff SoftBMP280::get_filter_coeff() {
  uint8_t register_addr = 0xF5;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return FilterCoeff::error;
  }

  register_data = (register_data >> 2) & 7;
  switch (register_data) {
    case 0  : return FilterCoeff::off;
    case 1  : return FilterCoeff::c2;
    case 2  : return FilterCoeff::c4;
    case 3  : return FilterCoeff::c8;
    case 4  : 
    case 5  : 
    case 6  : 
    case 7  : return FilterCoeff::c16;
    default : return FilterCoeff::error;
  }
  
  return FilterCoeff::error;    
}

SoftBMP280::Mode SoftBMP280::get_mode() {
  uint8_t register_addr = 0xF4;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return Mode::error;
  }

  register_data = register_data & 3;
  switch (register_data) {
    case 0  : return Mode::sleep;
    case 1  : 
    case 2  : return Mode::forced;
    case 3  : return Mode::normal;
    default : return Mode::error;
  }  
  return Mode::error;
}

SoftBMP280::StandbyTime SoftBMP280::get_standby_time() {
  uint8_t register_addr = 0xF5;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return StandbyTime::error;
  }

  register_data >>= 5 ;
  switch (register_data) {
    case 0  : return StandbyTime::ms0_5;
    case 1  : return StandbyTime::ms62_5;
    case 2  : return StandbyTime::ms125;
    case 3  : return StandbyTime::ms250;
    case 4  : return StandbyTime::ms500;
    case 5  : return StandbyTime::ms1000;
    case 6  : return StandbyTime::ms2000;
    case 7  : return StandbyTime::ms4000;
    default : return StandbyTime::error;
  }
 
  return StandbyTime::error;      
}

SoftBMP280::Status SoftBMP280::is_measuring() {
  uint8_t register_addr = 0xF3;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return Status::error;
  }
  return (register_data & 8) > 0 ? Status::active : Status::inactive;
}

SoftBMP280::Status SoftBMP280::is_updating() {
  uint8_t register_addr = 0xF3;
  uint8_t register_data = 0;
  if (!transceive_wr(slave_addr,&register_addr,1,&register_data,1)) {
    return Status::error;
  }
  return (register_data & 1) > 0 ? Status::active : Status::inactive;  
}

void SoftBMP280::set_temp_oversampling(Oversampling os) {
  uint8_t data[2] = {0xF4,0};
  if (!transceive_wr(slave_addr,data,1,data+1,1)) {
    return;
  }

  uint8_t os_flags = 0;
  switch (os) {
    case Oversampling::skipped : os_flags = 0; break;
    case Oversampling::x1      : os_flags = 1; break;
    case Oversampling::x2      : os_flags = 2; break;
    case Oversampling::x4      : os_flags = 3; break;
    case Oversampling::x8      : os_flags = 4; break;
    case Oversampling::x16     : os_flags = 5; break;
    case Oversampling::error   : os_flags = 0; break;
  }
  data[1] = (data[1] & ~(7 << 5)) | (os_flags << 5);

  transmit(slave_addr,data,2);
}

void SoftBMP280::set_pressure_oversampling(Oversampling os) {
  uint8_t data[2] = {0xF4,0};
  if (!transceive_wr(slave_addr,data,1,data+1,1)) {
    return;
  }

  uint8_t os_flags = 0;
  switch (os) {
    case Oversampling::skipped : os_flags = 0; break;
    case Oversampling::x1      : os_flags = 1; break;
    case Oversampling::x2      : os_flags = 2; break;
    case Oversampling::x4      : os_flags = 3; break;
    case Oversampling::x8      : os_flags = 4; break;
    case Oversampling::x16     : os_flags = 5; break;
    case Oversampling::error   : os_flags = 0; break;
  }
  data[1] = (data[1] & ~(7 << 2)) | (os_flags << 2);

  transmit(slave_addr,data,2);  
}

void SoftBMP280::set_filter_coeff(FilterCoeff fc) {
  uint8_t data[2] = {0xF5,0};
  if (!transceive_wr(slave_addr,data,1,data+1,1)) {
    return;
  }

  uint8_t fc_flags = 0;
  switch (fc) {
    case FilterCoeff::off   : fc_flags = 0; break;
    case FilterCoeff::c2    : fc_flags = 1; break;
    case FilterCoeff::c4    : fc_flags = 2; break;
    case FilterCoeff::c8    : fc_flags = 3; break;
    case FilterCoeff::c16   : fc_flags = 4; break;
    case FilterCoeff::error : fc_flags = 0; break;
  }
  data[1] = (data[1] & ~(7 << 2)) | (fc_flags << 2);

  transmit(slave_addr,data,2);    
}

void SoftBMP280::set_mode(Mode m) {
  uint8_t data[2] = {0xF4,0};
  if (!transceive_wr(slave_addr,data,1,data+1,1)) {
    return;
  }

  uint8_t m_flags = 0;
  switch (m) {
    case Mode::sleep  : m_flags = 0; break;
    case Mode::normal : m_flags = 3; break;
    case Mode::forced : m_flags = 1; break;
    case Mode::error  : m_flags = 0; break;
  }
  data[1] = (data[1] & ~3) | m_flags;

  transmit(slave_addr,data,2);  
}

void SoftBMP280::set_standby_time(StandbyTime t) {
  uint8_t data[2] = {0xF5,0};
  if (!transceive_wr(slave_addr,data,1,data+1,1)) {
    return;
  }

  uint8_t t_flags = 0;
  switch (t) {
    case StandbyTime::ms0_5  : t_flags = 0; break;
    case StandbyTime::ms62_5 : t_flags = 1; break;
    case StandbyTime::ms125  : t_flags = 2; break;
    case StandbyTime::ms250  : t_flags = 3; break;
    case StandbyTime::ms500  : t_flags = 4; break;
    case StandbyTime::ms1000 : t_flags = 5; break;
    case StandbyTime::ms2000 : t_flags = 6; break;
    case StandbyTime::ms4000 : t_flags = 7; break;
    case StandbyTime::error  : t_flags = 0; break;
  }
  data[1] = (data[1] & ~(7 << 5)) | (t_flags << 5);

  transmit(slave_addr,data,2);  
}

void SoftBMP280::read_sensor_data()
{
  uint8_t register_addr = 0xF7;
  uint8_t register_data[6];
  if (!transceive_wr(slave_addr,&register_addr,1,register_data,6)) {
    return;
  }

  adc_pressure = ((int32_t)register_data[0] << 12) | ((int32_t)register_data[1] << 4) | (int32_t)(register_data[2] >> 4);
  adc_temp     = ((int32_t)register_data[3] << 12) | ((int32_t)register_data[4] << 4) | (int32_t)(register_data[5] >> 4);
  
  latest_temp = compensate_temp(adc_temp);  
  latest_pressure = compensate_pressure(adc_pressure);  
}

void SoftBMP280::print_mode(Mode mode){
  switch(mode) {
    case Mode::sleep  : Serial.print("sleep");  break;
    case Mode::normal : Serial.print("normal"); break;
    case Mode::forced : Serial.print("forced"); break;
    case Mode::error  : Serial.print("error");  break;
  }
}

void SoftBMP280::print_oversampling(Oversampling os){
  switch(os) {
    case Oversampling::skipped : Serial.print("skipped"); break;
    case Oversampling::x1      : Serial.print("x1");      break;
    case Oversampling::x2      : Serial.print("x2");      break;
    case Oversampling::x4      : Serial.print("x4");      break;
    case Oversampling::x8      : Serial.print("x8");      break;
    case Oversampling::x16     : Serial.print("x16");     break;
    case Oversampling::error   : Serial.print("error");   break;
  }
}

void SoftBMP280::print_filter_coeff(FilterCoeff fc){
  switch(fc) {
    case FilterCoeff::off   : Serial.print("off");   break;
    case FilterCoeff::c2    : Serial.print("c2");    break;
    case FilterCoeff::c4    : Serial.print("c4");    break;
    case FilterCoeff::c8    : Serial.print("c8");    break;
    case FilterCoeff::c16   : Serial.print("c16");   break;
    case FilterCoeff::error : Serial.print("error"); break;
  }
}

void SoftBMP280::print_standby_time(StandbyTime st){
  switch(st) {
    case StandbyTime::ms0_5  : Serial.print("0.5 ms");  break;
    case StandbyTime::ms62_5 : Serial.print("62.5 ms"); break;
    case StandbyTime::ms125  : Serial.print("125 ms");  break;
    case StandbyTime::ms250  : Serial.print("250 ms");  break;
    case StandbyTime::ms500  : Serial.print("500 ms");  break;
    case StandbyTime::ms1000 : Serial.print("1000 ms"); break;
    case StandbyTime::ms2000 : Serial.print("2000 ms"); break;
    case StandbyTime::ms4000 : Serial.print("4000 ms"); break;
    case StandbyTime::error  : Serial.print("error");   break;
  }
}

void SoftBMP280::print_status(Status st){
  switch(st) {
    case Status::inactive : Serial.print("inactive"); break;
    case Status::active   : Serial.print("active");   break;
    case Status::error    : Serial.print("error");    break;
  }
}

void SoftBMP280::print_configuration()
{
  Serial.print("Mode:                    "); print_mode(get_mode());                          Serial.println("");
  Serial.print("Temp oversampling:       "); print_oversampling(get_temp_oversampling());     Serial.println("");
  Serial.print("Pressure oversampling:   "); print_oversampling(get_pressure_oversampling()); Serial.println("");
  Serial.print("IIR filter coefficients: "); print_filter_coeff(get_filter_coeff());          Serial.println("");
  Serial.print("Standby time:            "); print_standby_time(get_standby_time());          Serial.println("");
  Serial.print("Is Measuring:            "); print_status(is_measuring());                    Serial.println("");
  Serial.print("Is Updating:             "); print_status(is_updating());                     Serial.println("");  
}

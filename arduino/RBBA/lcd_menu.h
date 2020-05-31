#ifndef LCD_MENU_H
#define LCD_MENU_H

#include <Arduino.h>
#include <stdint.h>
#include <string.h>
#include "soft_hd44780.h"
#include "encoder.h"
//#include "tuple.h"

//using namespace tpl;

template<typename E>
class LCDMenuBase {
 
  SoftHD44780 &display;

  E cur_panel;
  bool refresh_panel;

public:  
  LCDMenuBase(SoftHD44780 &_display, E start_panel) :
    display(_display),
    cur_panel(start_panel),
    refresh_panel(true)
  {}

  LCDMenuBase(const LCDMenuBase<E> &other) :
    display(other.display),
    cur_panel(other.cur_panel),
    refresh_panel(other.refresh_panel)
  {}

  SoftHD44780& get_display() { return display; }

  void switch_to_panel(const E id) {
    cur_panel = id;
    refresh_panel = true;
  }

  virtual void bind_knob(int16_t min_val, int16_t max_val, int16_t start_val) = 0;
  virtual void unbind_knob() = 0;

  bool needs_refresh() { return refresh_panel; }

  E get_cur_panel() { return cur_panel; }

  void set_refresh_panel(bool refresh) { refresh_panel = refresh; }
};

template<typename E>
class LCDMenuElement;

template<typename E>
using DrawFunc = void (*)(LCDMenuBase<E> *menu, LCDMenuElement<E> *element);

template<typename E>
using OnPushFunc = void (*)(LCDMenuBase<E> *menu, LCDMenuElement<E> *element);

template<typename E>
using OnEncChangeFunc = void (*)(LCDMenuBase<E> *menu, LCDMenuElement<E> *element, int16_t enc_data);

template<typename E>
class LCDMenuElement {
  uint8_t pos_x;
  uint8_t pos_y;

  uint8_t needs_refresh;

  uint8_t select_id;

public:
  DrawFunc<E>        draw;
  OnPushFunc<E>      on_push;
  OnEncChangeFunc<E> on_enc_change;
  
  LCDMenuElement(const uint8_t x, const uint8_t y) :
    pos_x(x),
    pos_y(y),
    needs_refresh(0),
    select_id(255),
    draw(nullptr),
    on_push(nullptr),
    on_enc_change(nullptr)
  {}

  LCDMenuElement(const LCDMenuElement<E> &other) :
    pos_x(other.pos_x),
    pos_y(other.pos_y),
    needs_refresh(other.needs_refresh),
    select_id(other.select_id),
    draw(other.draw),
    on_push(other.on_push),
    on_enc_change(other.on_enc_change)
  {}
  
  virtual ~LCDMenuElement() {}

  void set_draw(DrawFunc<E> draw_func) { draw = draw_func; }
  void set_push(OnPushFunc<E> push_func) { on_push = push_func; }
  void set_enc_change(OnEncChangeFunc<E> enc_func) { on_enc_change = enc_func; }
  void set_refresh(uint8_t refresh) { needs_refresh = refresh; }
  void set_select(uint8_t id) { select_id = id; }

  uint8_t get_refresh() const { return needs_refresh; }
  uint8_t get_select() const { return select_id; }

  uint8_t get_x() const { return pos_x; }
  uint8_t get_y() const { return pos_y; }
};

template<typename E>
struct LCDMenuTextElement : public LCDMenuElement<E> {

  const char *text;

  LCDMenuTextElement(const uint8_t x, const uint8_t y, const char *_text, OnPushFunc<E> _on_push = nullptr) :
    LCDMenuElement<E>(x,y),
    text(_text)
  {
    LCDMenuElement<E>::set_draw([](LCDMenuBase<E> *menu, LCDMenuElement<E> *element) {
      menu->get_display().print(static_cast<LCDMenuTextElement*>(element)->text);
    });
    LCDMenuElement<E>::set_push(_on_push);
  }

  LCDMenuTextElement(const LCDMenuTextElement<E> &other) :
    LCDMenuElement<E>(other),
    text(other.text)
  {}

  ~LCDMenuTextElement() 
  {
  }

};

template<typename E>
struct LCDMenuBufferElement : public LCDMenuElement<E> {

  char *buffer;

  LCDMenuBufferElement(const uint8_t x, const uint8_t y, char *_buffer, uint8_t auto_refresh = 8, OnPushFunc<E> _on_push = nullptr) :
    LCDMenuElement<E>(x,y),
    buffer(_buffer)
  {
    LCDMenuElement<E>::set_refresh(auto_refresh);
    LCDMenuElement<E>::set_draw([](LCDMenuBase<E> *menu, LCDMenuElement<E> *element) {
      menu->get_display().print(static_cast<LCDMenuBufferElement*>(element)->buffer);
    });
    LCDMenuElement<E>::set_push(_on_push);
  }

  LCDMenuBufferElement(const LCDMenuBufferElement<E> &other) :
    LCDMenuElement<E>(other),
    buffer(other.buffer)
  {}
  
};

template<typename E, typename IntType, uint8_t Digits, uint8_t DecimalPlaces>
struct LCDMenuIntElement : public LCDMenuElement<E> {

  IntType *value;

  const char *suffix;

  bool bound_knob;
  int16_t min_val;
  int16_t max_val;

  LCDMenuIntElement(const uint8_t x, const uint8_t y, IntType *_value, const char *_suffix, uint8_t auto_refresh = 0, bool editable = false, int16_t _min_val = 0, int16_t _max_val = 255) :
    LCDMenuElement<E>(x,y),
    value(_value),
    suffix(_suffix),
    bound_knob(false),
    min_val(_min_val),
    max_val(_max_val)
  {
    LCDMenuElement<E>::set_refresh(auto_refresh);
    LCDMenuElement<E>::set_draw([](LCDMenuBase<E> *menu, LCDMenuElement<E> *element) {
      LCDMenuIntElement &int_elem = *(static_cast<LCDMenuIntElement*>(element));
      IntType value = *(int_elem.value);
      char digits[Digits];
      memset(digits,' ',Digits);
      char sign = ' ';
      if (value < 0) {
        sign = '-';
        value *= -1;
      }
      int8_t i = 0;
      while(i < (int8_t)Digits) {
        digits[i++] = (char)((uint8_t)(value % 10) + (uint8_t)'0');
        if ((DecimalPlaces) && (DecimalPlaces == i)) {
          digits[i++] = '.';
        }
        value /= (IntType)10;
        if (value == 0) {
          if (i < Digits) {
            digits[i] = sign;
          }
          break;
        }
      }
      for (i = Digits-1; i >= 0; --i)
        menu->get_display().put_data(digits[i]);
      menu->get_display().print(int_elem.suffix);
    });
    if (editable) {
      LCDMenuElement<E>::set_push([](LCDMenuBase<E> *menu, LCDMenuElement<E> *element) {
        LCDMenuIntElement &int_elem = *(static_cast<LCDMenuIntElement*>(element));
        if (int_elem.bound_knob) {
          menu->unbind_knob();
          int_elem.bound_knob = false;
        } else {
          menu->bind_knob(int_elem.min_val,int_elem.max_val,(int16_t)*(int_elem.value));
          int_elem.bound_knob = true;
        }
      });
      LCDMenuElement<E>::set_enc_change([](LCDMenuBase<E> *menu, LCDMenuElement<E> *element, int16_t enc_value) {
        LCDMenuIntElement &int_elem = *(static_cast<LCDMenuIntElement*>(element));
        *(int_elem.value) = (IntType)enc_value;
      });
    }
  }

  LCDMenuIntElement(const LCDMenuIntElement<E,IntType,Digits,DecimalPlaces> &other) :
    LCDMenuElement<E>(other),
    value(other.value),
    suffix(other.suffix),
    bound_knob(other.bound_knob),
    min_val(other.min_val),
    max_val(other.max_val)
  {}

  ~LCDMenuIntElement()
  {
  }

};

template<typename E>
struct LCDMenuPanelBase {

  virtual uint8_t get_element_count() const = 0;
  virtual LCDMenuElement<E>* get_element(uint8_t id) = 0;
  virtual uint8_t get_select_count() const = 0;

  LCDMenuPanelBase() 
  {}

  virtual ~LCDMenuPanelBase() {}
  
};

template<typename E, typename... MenuElements>
struct LCDMenuPanel : public LCDMenuPanelBase<E> {

  static const uint8_t nr_of_elements = sizeof...(MenuElements);

  //tuple<MenuElements...> menu_elements;
  LCDMenuElement<E>* menu_elements[nr_of_elements];

  uint8_t select_cnt;

  LCDMenuPanel(MenuElements... elements) :
    LCDMenuPanelBase<E>(),
    menu_elements{static_cast<LCDMenuElement<E>*>(elements)...},
    //menu_elements{elements...},
    select_cnt(0)
  { 
    for (uint8_t i = 0; i < nr_of_elements; ++i) {
      LCDMenuElement<E> *cur_elem = get_element(i);
      if (cur_elem->on_push != nullptr) {
        cur_elem->set_select(select_cnt++);
      }
    }
  }

  LCDMenuPanel(const LCDMenuPanel<E,MenuElements...> &other) :
    LCDMenuPanelBase<E>(other),
    menu_elements(other.menu_elements),
    select_cnt(other.select_cnt)
  {}

  ~LCDMenuPanel() {
    for (uint8_t i = 0; i < nr_of_elements; ++i) {
      delete menu_elements[i];
    }
  }

  uint8_t get_element_count() const { return nr_of_elements; }
  LCDMenuElement<E>* get_element(uint8_t id) {
    return menu_elements[id];
    //return get_p<LCDMenuElement<E>>(id,menu_elements);
  }
  uint8_t get_select_count() const { return select_cnt; }
  
};

/*
template<typename E, typename... MenuElements>
auto make_panel(E pid, MenuElements... elements) -> LCDMenuPanel<E,MenuElements...>* {
  return new LCDMenuPanel<E,MenuElements...>(pid,elements...);
}
*/

template<typename E, typename... MenuElements>
LCDMenuPanelBase<E>* make_panel(MenuElements... elements) {
  return static_cast<LCDMenuPanelBase<E>*>(new LCDMenuPanel<E,MenuElements...>(elements...));
}

template<typename E>
using MenuPanelFunc = LCDMenuPanelBase<E>* (*)();

template<typename E>
struct Panel {
  E pid;
  MenuPanelFunc<E> create;
};

template<typename E, typename... MenuPanels>
class LCDMenu : public LCDMenuBase<E> {

  static const uint8_t nr_of_panels = sizeof...(MenuPanels);

  //LCDMenuPanelBase<E>* menu_panels[nr_of_panels];
  Panel<E> menu_panels[nr_of_panels];
  LCDMenuPanelBase<E>* current_panel;
  
  Encoder &knob;
  uint8_t knob_pin;

  int16_t knob_min;
  int16_t knob_max;
  int16_t knob_cur;

  static const int16_t knob_res = -2;

  uint8_t cur_select_cnt;
  uint8_t cur_selection;

  bool pressed;

  int16_t old_knob_cur;

  uint16_t upd_cnt;

  bool knob_bound;

  Panel<E>& get_panel(uint8_t id) {
    return menu_panels[id];
  }

public:

  LCDMenu(SoftHD44780 &display, Encoder &_knob, uint8_t _knob_pin, E start_panel, MenuPanels... panels) :
    LCDMenuBase<E>(display,start_panel),
    //menu_panels{static_cast<LCDMenuPanelBase<E>*>(panels)...},
    menu_panels{panels...},
    current_panel(nullptr),
    knob(_knob),
    knob_pin(_knob_pin),
    knob_min(0),
    knob_max(0),
    knob_cur(0),
    cur_select_cnt(0),
    cur_selection(0),
    pressed(false),
    old_knob_cur(0),
    upd_cnt(0),
    knob_bound(false)
  { 
    current_panel = menu_panels[0].create();
    knob.set_value(0);
    pinMode(knob_pin,INPUT_PULLUP);
  }

  ~LCDMenu() {
    if (current_panel) 
      delete current_panel;
    /*
    for (uint8_t i = 0; i < nr_of_panels; ++i)
      delete menu_panels[i];      
      */
  }

  void bind_knob(int16_t min_val, int16_t max_val, int16_t start_val) {
    old_knob_cur = knob_cur;
    knob_cur   = start_val;
    knob_min   = min_val;
    knob_max   = max_val;
    knob.set_value(knob_cur * knob_res);
    knob_bound = true;
  }
  
  void unbind_knob() {
    knob_cur   = old_knob_cur;
    knob_min   = 0;
    knob_max   = cur_select_cnt-1;
    knob.set_value(knob_cur * knob_res);
    knob_bound = false;
  }

  void update() {
    knob_cur = knob.get_value() / knob_res;
    if (knob_cur > knob_max) {
      if (knob_cur > knob_max+2) {
        knob.set_value((knob_max+2) * knob_res);
      }
      knob_cur = knob_max;      
    }
    if (knob_cur < knob_min) {
      if (knob_cur < knob_min-2) {
        knob.set_value((knob_min-2) * knob_res);
      }
      knob_cur = knob_min;      
    }
    // do something with knob turn
    if (knob_bound) {
      LCDMenuPanelBase<E> &cur_p = *current_panel;
      for(uint8_t i = 0; i < cur_p.get_element_count(); ++i) {
        LCDMenuElement<E> &cur_element = *(cur_p.get_element(i));
        if (cur_element.get_select() == cur_selection) {
          cur_element.on_enc_change(this,&cur_element,knob_cur);
          break;
        }
      }
    } else {
      if (cur_select_cnt > 0) {
        uint8_t old_sel = cur_selection;
        cur_selection = knob_cur;
        if (cur_selection != old_sel) {
          LCDMenuPanelBase<E> &cur_p = *current_panel;
          for(uint8_t i = 0; i < cur_p.get_element_count(); ++i) {
            LCDMenuElement<E> &cur_element = *(cur_p.get_element(i));
            if (cur_element.get_select() == old_sel) {
              LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x()-1,cur_element.get_y());
              LCDMenuBase<E>::get_display().put_data(' ');          
              cur_element.draw(this,&cur_element);
              LCDMenuBase<E>::get_display().put_data(' ');              
            } else
            if (cur_element.get_select() == cur_selection) {
              LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x()-1,cur_element.get_y());
              LCDMenuBase<E>::get_display().put_data('<');          
              cur_element.draw(this,&cur_element);
              LCDMenuBase<E>::get_display().put_data('>');              
            }
          }
        }
      }
    }
    uint8_t knob_pushed = digitalRead(knob_pin);
    if ((knob_pushed == LOW) && (pressed == false)) {
      // do something with knop push
      pressed = true;
      if (cur_select_cnt > 0) {
        LCDMenuPanelBase<E> &cur_p = *current_panel;
        for(uint8_t i = 0; i < cur_p.get_element_count(); ++i) {
          LCDMenuElement<E> &cur_element = *(cur_p.get_element(i));
          if (cur_element.get_select() != cur_selection)
            continue;
          cur_element.on_push(this,&cur_element);
        }        
      }
    } else if ((pressed == true) && (knob_pushed == HIGH)) {
      pressed = false;
    }
    if (LCDMenuBase<E>::needs_refresh()) {
      LCDMenuBase<E>::get_display().clear();
      for(uint8_t i = 0; i < nr_of_panels; ++i){
        Panel<E> &cp = get_panel(i);
        if (cp.pid == LCDMenuBase<E>::get_cur_panel()) {
          if (current_panel)
            delete current_panel;
          current_panel = cp.create();
          break;
        }      
      }
      LCDMenuPanelBase<E> &cur_p = *current_panel;
      cur_select_cnt = cur_p.get_select_count();
      cur_selection = cur_select_cnt > 0 ? 0 : 254;
      knob_cur = 0;
      knob_min = 0;
      knob_max = cur_select_cnt-1;
      knob.set_value(knob_cur * knob_res);
      for(uint8_t i = 0; i < cur_p.get_element_count(); ++i) {
        LCDMenuElement<E> &cur_element = *(cur_p.get_element(i));
        if (cur_element.get_select() != cur_selection) {
          LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x(),cur_element.get_y());
          cur_element.draw(this,&cur_element);
        } else {
          LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x()-1,cur_element.get_y());
          LCDMenuBase<E>::get_display().put_data(knob_bound ? '|' : '<');
          cur_element.draw(this,&cur_element);
          LCDMenuBase<E>::get_display().put_data(knob_bound ? '|' : '>');
        }
      }
      LCDMenuBase<E>::set_refresh_panel(false);
    } else {
      LCDMenuPanelBase<E> &cur_p = *current_panel;
      for(uint8_t i = 0; i < cur_p.get_element_count(); ++i) {
        LCDMenuElement<E> &cur_element = *(cur_p.get_element(i));
        if ((!cur_element.get_refresh()) || (upd_cnt % cur_element.get_refresh()))
          continue;
        if (cur_element.get_select() != cur_selection) {
          LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x(),cur_element.get_y());
          cur_element.draw(this,&cur_element);
        } else {
          LCDMenuBase<E>::get_display().set_cursor(cur_element.get_x()-1,cur_element.get_y());
          LCDMenuBase<E>::get_display().put_data(knob_bound ? '|' : '<');          
          cur_element.draw(this,&cur_element);
          LCDMenuBase<E>::get_display().put_data(knob_bound ? '|' : '>');
        }
      }
    }
    ++upd_cnt;
  }
  
};

template<typename E, typename... MenuPanels>
auto make_menu(SoftHD44780 &display, Encoder &_knob, uint8_t _knob_pin, E start_panel, MenuPanels... panels) -> LCDMenu<E,MenuPanels...> {
  return LCDMenu<E,MenuPanels...>(display,_knob,_knob_pin,start_panel,panels...);
}


#endif

/**
 * @file 7segment_gpio.h
 */

#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include <vector>

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <esp32-hal-timer.h>
#endif

namespace esphome {
namespace lcd_digits {

constexpr uint8_t max_digit_count = 4;

struct LcdData {
  uint8_t buffer_[max_digit_count] = {};
  bool colon_on = false;
  bool degree_on = false;
};

enum DisplayType { CommonAnode, CommonCathode };

struct LcdDigitsData : LcdData {
  std::vector<GPIOPin *> digit_pins;
  std::vector<GPIOPin *> segment_pins;
  GPIOPin *colon_pin = nullptr;
  GPIOPin *degree_pin = nullptr;

  uint8_t cycles_to_skip = 0;
  uint8_t current_frame = 0;

  bool compensate_brightness = false;
  bool iterate_digits = true;

  uint8_t intensity_delay = 0;

  // ➜ НОВЕ: затримка перед вмиканням цифри
  uint16_t blank_delay_us = 50;

  void IRAM_ATTR timer_interrupt();
};

class LcdDigitsComponent : public PollingComponent {
public:
  enum Mode { BufferMode, ProgressMode, DisabledMode };

  void set_segment_pins(std::vector<GPIOPin *> pins);
  void set_digit_pins(std::vector<GPIOPin *> pins);
  void set_colon_pin(GPIOPin *pin);
  void set_degree_pin(GPIOPin *pin);

  void set_display_type(DisplayType arg);
  void set_compensate_brightness(bool arg);
  void set_iterate_digits(bool arg);
  void set_intensity(uint8_t arg);

  // ➜ НОВЕ: setter затримки
  void set_blank_delay_us(uint16_t v) {
    interrupt_data_.blank_delay_us = v;
  }

  void setup() override;
  void update() override;
  void dump_config() override;

  uint8_t print(const char *str);
  uint8_t print(uint8_t pos, const char *str);
  uint8_t printf(uint8_t pos, const char *fmt, ...);

  void set_raw(uint64_t raw);

  void set_colon_on(bool on = true);
  void set_degree_on(bool on = true);

  void set_mode(Mode mode);
  void set_progress(float progress);

private:
  static constexpr auto TAG = "lcd_digits";
  hw_timer_t *timer = nullptr;

  LcdDigitsData interrupt_data_;
  LcdData display_data_;

  Mode mode_ = DisabledMode;
};

} // namespace lcd_digits
} // namespace esphome

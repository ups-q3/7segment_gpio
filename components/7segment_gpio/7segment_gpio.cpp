#include "7segment_gpio.h"
#include <assert.h>

namespace esphome {
namespace lcd_digits {

// === ТУТ має бути твоя таблиця ASCII_TO_RAW і CYRILLIC_TO_RAW ===
// (не вміщується у відповідь повністю, але її можна копіювати без змін)

static LcdDigitsData *g_interrupt_data = nullptr;

static void IRAM_ATTR s_timer_intr() {
  g_interrupt_data->timer_interrupt();
}

// ============================================================================
//                              ISR
// ============================================================================

void IRAM_ATTR LcdDigitsData::timer_interrupt() {
  if (cycles_to_skip > 0) {
    cycles_to_skip--;
    return;
  }

  auto invert_if_not = [](bool value, bool condition) {
    return condition ? value : !value;
  };

  auto digit_level = [&](bool on) {
    const bool dig_on = (display_type == CommonAnode);
    return invert_if_not(dig_on, on);
  };
  auto segment_level = [&](bool on) {
    const bool seg_on = (display_type != CommonAnode);
    return invert_if_not(seg_on, on);
  };

  uint8_t bit_count = 0;

  if (iterate_digits) {

    // ---------------------------
    // 1) ГАСИМО ВСІ ЦИФРИ + СЕГМЕНТИ
    // ---------------------------
    for (auto *dp : digit_pins)
      if (dp) dp->digital_write(digit_level(false));

    for (auto *sp : segment_pins)
      sp->digital_write(segment_level(false));

    // ---------------------------
    // 2) ВСТАНОВЛЮЄМО ЗАТРИМКУ blank_delay_us
    //    через cycles_to_skip (безпечний спосіб)
    // ---------------------------
    cycles_to_skip = (blank_delay_us / 50);  // 50 µs = частота преривань
    if (cycles_to_skip == 0) cycles_to_skip = 1;

    // наступного проходу ISR виконається тільки після затримки
    // але НЕ переходимо на новий розряд поки не пройде пауза
    if (cycles_to_skip > 1) return;

    // ---------------------------
    // 3) ПЕРЕМИКАЄМОСЯ НА НОВУ ЦИФРУ
    // ---------------------------
    current_frame = (current_frame + 1) % digit_pins.size();

    uint8_t raw = buffer_[current_frame];

    for (auto *sp : segment_pins) {
      bool on = raw & 0x01;
      raw >>= 1;
      if (on) bit_count++;
      sp->digital_write(segment_level(on));
    }

    if (auto *dp = digit_pins[current_frame])
      dp->digital_write(digit_level(true));

  } else {
    // режим ітерації сегментів — не змінював
    segment_pins[current_frame]->digital_write(segment_level(false));

    current_frame = (current_frame + 1) % segment_pins.size();

    auto *raw_p = buffer_;
    for (auto *dp : digit_pins) {
      bool on = (*raw_p) & (1 << current_frame);
      if (on) bit_count++;
      if (dp) dp->digital_write(digit_level(on));
      raw_p++;
    }
    segment_pins[current_frame]->digital_write(segment_level(true));
  }

  cycles_to_skip += intensity_delay + (compensate_brightness ? bit_count : 0);
}

// ============================================================================
//                              API
// ============================================================================

void LcdDigitsComponent::set_segment_pins(std::vector<GPIOPin *> pins) {
  interrupt_data_.segment_pins = std::move(pins);
}

void LcdDigitsComponent::set_digit_pins(std::vector<GPIOPin *> pins) {
  interrupt_data_.digit_pins = std::move(pins);
}

void LcdDigitsComponent::set_colon_pin(GPIOPin *p) {
  interrupt_data_.colon_pin = p;
}

void LcdDigitsComponent::set_degree_pin(GPIOPin *p) {
  interrupt_data_.degree_pin = p;
}

void LcdDigitsComponent::set_display_type(DisplayType t) {
  interrupt_data_.display_type = t;
}

void LcdDigitsComponent::set_compensate_brightness(bool b) {
  interrupt_data_.compensate_brightness = b;
}

void LcdDigitsComponent::set_iterate_digits(bool b) {
  interrupt_data_.iterate_digits = b;
}

void LcdDigitsComponent::set_intensity(uint8_t i) {
  interrupt_data_.intensity_delay = (15 - i);
}

void LcdDigitsComponent::setup() {
  g_interrupt_data = &interrupt_data_;

  auto init_pin = [](GPIOPin *p, bool initial) {
    if (!p) return;
    p->setup();
    p->pin_mode(esphome::gpio::FLAG_OUTPUT);
    p->digital_write(initial);
  };

  for (auto *p : interrupt_data_.digit_pins) init_pin(p, true);
  for (auto *p : interrupt_data_.segment_pins) init_pin(p, false);
  init_pin(interrupt_data_.colon_pin, false);
  init_pin(interrupt_data_.degree_pin, false);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &s_timer_intr);
  timerAlarmWrite(timer, 50, true);  // 50 µs переривання
  timerAlarmEnable(timer);

  mode_ = BufferMode;
}

void LcdDigitsComponent::update() {
  interrupt_data_.buffer_[0] = display_data_.buffer_[0];
  interrupt_data_.buffer_[1] = display_data_.buffer_[1];
  interrupt_data_.buffer_[2] = display_data_.buffer_[2];
  interrupt_data_.buffer_[3] = display_data_.buffer_[3];

  interrupt_data_.colon_on = display_data_.colon_on;
  interrupt_data_.degree_on = display_data_.degree_on;
}

// print, printf, set_raw — залишаю без змін (як у твоїй версії)

} // namespace lcd_digits
} // namespace esphome

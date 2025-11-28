#include "7segment_gpio.h"
#include <assert.h>

namespace esphome {
namespace lcd_digits {

namespace {

constexpr uint8_t UNKNOWN_CHAR = 0xff;

constexpr uint8_t ASCII_TO_RAW[95] = {
    0b11111111,   // ' ', ord 0x20
    0b10110000,   // '!'
    0b00100010,   // '"'
    UNKNOWN_CHAR, // '#'
    UNKNOWN_CHAR, // '$'
    0b01001001,   // '%'
    UNKNOWN_CHAR, // '&'
    0b00000010,   // '''
    0b01001110,   // '('
    0b01111000,   // ')'
    0b01000000,   // '*'
    UNKNOWN_CHAR, // '+'
    0b00010000,   // ','
    0b00000001,   // '-'
    0b10000000,   // '.'
    UNKNOWN_CHAR, // '/'
    0b11111111,   // '0' (інверсія 0b01111110)
    0b11101110,   // '1'
    0b11011101,   // '2'
    0b11001100,   // '3'
    0b10111011,   // '4'
    0b10101010,   // '5'
    0b10011001,   // '6'
    0b10001000,   // '7'
    0b01110177,   // '8'
    0b01100110,   // '9'
    0b01001000,   // ':'
    0b01011000,   // ';'
    0b01000011,   // '<'
    0b00001001,   // '='
    0b01100001,   // '>'
    0b01100101,   // '?'
    0b01101111,   // '@'
    0b01110111,   // 'A'
    0b01111111,   // 'B'
    0b01001110,   // 'C'
    0b01111001,   // 'D'
    0b01001111,   // 'E'
    0b01000111,   // 'F'
    0b01011110,   // 'G'
    0b00110111,   // 'H'
    0b00000110,   // 'I'
    0b00111000,   // 'J'
    0b01010111,   // 'K'
    0b00001110,   // 'L'
    0b01101011,   // 'M'
    0b01110110,   // 'N'
    0b01111110,   // 'O'
    0b01100111,   // 'P'
    0b01101011,   // 'Q'
    0b01101111,   // 'R'
    0b01011011,   // 'S'
    0b01000110,   // 'T'
    0b00111110,   // 'U'
    0b00111010,   // 'V'
    0b01011100,   // 'W'
    0b01001001,   // 'X'
    0b00101011,   // 'Y'
    0b01101101,   // 'Z'
    0b01001110,   // '['
    UNKNOWN_CHAR, // '\'
    0b01111000,   // ']'
    UNKNOWN_CHAR, // '^'
    0b00001000,   // '_'
    0b00100000,   // '`'
    0b00011001,   // 'a'
    0b00011111,   // 'b'
    0b00001101,   // 'c'
    0b00111101,   // 'd'
    0b00001100,   // 'e'
    0b00000111,   // 'f'
    0b01001101,   // 'g'
    0b00010111,   // 'h'
    0b01001100,   // 'i'
    0b01011000,   // 'j'
    0b01011000,   // 'k'
    0b00000110,   // 'l'
    0b01010101,   // 'm'
    0b00010101,   // 'n'
    0b00011101,   // 'o'
    0b01100111,   // 'p'
    0b01110011,   // 'q'
    0b00000101,   // 'r'
    0b00011000,   // 's'
    0b00001111,   // 't'
    0b00011100,   // 'u'
    0b00011000,   // 'v'
    0b00101010,   // 'w'
    0b00001001,   // 'x'
    0b00111011,   // 'y'
    0b00001001,   // 'z'
    0b00110001,   // '{'
    0b00000110,   // '|'
    0b00000111,   // '}'
    0b01100011,   // '~'
};

// тут можна доопрацювати кирилицю за потреби, залишаю як було
constexpr uint8_t CYRILLIC_TO_RAW[] = {
    0b01110111,   // `А`
    0b01011111,   // `Б`
    0b01111111,   // `В`
    0b01000110,   // `Г`
    UNKNOWN_CHAR, // `Д`
    0b01001111,   // `Е`
    UNKNOWN_CHAR, // `Ж`
    0b01111001,   // `З`
    UNKNOWN_CHAR, // `И`
    UNKNOWN_CHAR, // `Й`
    UNKNOWN_CHAR, // `К`
    UNKNOWN_CHAR, // `Л`
    UNKNOWN_CHAR, // `М`
    0b00110111,   // `Н`
    0b01111110,   // `О`
    0b01110160,   // `П`
    0b01100111,   // `Р`
    0b01001110,   // `С`
    UNKNOWN_CHAR, // `Т`
    0b00111011,   // `У`
    UNKNOWN_CHAR, // `Ф`
    UNKNOWN_CHAR, // `Х`
    UNKNOWN_CHAR, // `Ц`
    0b00110011,   // `Ч`
    UNKNOWN_CHAR, // `Ш`
    UNKNOWN_CHAR, // `Щ`
    UNKNOWN_CHAR, // `Ъ`
    UNKNOWN_CHAR, // `Ы`
    0b00011111,   // `Ь`
    0b01111001,   // `Э`
    UNKNOWN_CHAR, // `Ю`
    UNKNOWN_CHAR, // `Я`

    UNKNOWN_CHAR, // `а`
    UNKNOWN_CHAR, // `б`
    UNKNOWN_CHAR, // `в`
    0b00000011,   // `г`
    0b01111101,   // `д`
    UNKNOWN_CHAR, // `е`
    UNKNOWN_CHAR, // `ж`
    UNKNOWN_CHAR, // `з`
    0b00011100,   // `и`
    0b01011100,   // `й`
    UNKNOWN_CHAR, // `к`
    UNKNOWN_CHAR, // `л`
    UNKNOWN_CHAR, // `м`
    UNKNOWN_CHAR, // `н`
    UNKNOWN_CHAR, // `о`
    UNKNOWN_CHAR, // `п`
    UNKNOWN_CHAR, // `р`
    UNKNOWN_CHAR, // `с`
    UNKNOWN_CHAR, // `т`
    UNKNOWN_CHAR, // `у`
    UNKNOWN_CHAR, // `ф`
    UNKNOWN_CHAR, // `х`
    UNKNOWN_CHAR, // `ц`
    UNKNOWN_CHAR, // `ч`
    UNKNOWN_CHAR, // `ш`
    UNKNOWN_CHAR, // `щ`
    UNKNOWN_CHAR, // `ъ`
    UNKNOWN_CHAR, // `ы`
    UNKNOWN_CHAR, // `ь`
    UNKNOWN_CHAR, // `э`
    UNKNOWN_CHAR, // `ю`
    UNKNOWN_CHAR, // `я`
};

LcdDigitsData *g_interrupt_data = nullptr;

static void IRAM_ATTR HOT s_timer_intr() {
  g_interrupt_data->timer_interrupt();
}

} // namespace

void IRAM_ATTR HOT LcdDigitsData::timer_interrupt() {
  if (cycles_to_skip > 0) {
    cycles_to_skip--;
    return;
  }

  auto invert_if_not = [](bool value, bool condition) {
    return condition ? value : !value;
  };

  auto digit_level = [&](bool on) {
    const auto digit_on_level = display_type == CommonAnode;
    return invert_if_not(digit_on_level, on);
  };

  auto segment_level = [&](bool on) {
    const auto segment_on_level = display_type != CommonAnode;
    return invert_if_not(segment_on_level, on);
  };

  uint8_t bit_count = 0;

  // ---------------------------
  // BLANK-ФАЗА: все гасимо
  // ---------------------------
  if (!in_blank_phase && blank_cycles > 0) {
    // Переходимо в фазу гасіння перед наступним кадром
    in_blank_phase = true;

    // Вимикаємо всі digit
    for (auto *pin : digit_pins)
      if (pin)
        pin->digital_write(digit_level(false));

    // Вимикаємо всі сегменти
    for (auto *pin : segment_pins)
      pin->digital_write(segment_level(false));

    // Вимикаємо двокрапку та градус, якщо є
    if (colon_pin)
      colon_pin->digital_write(segment_level(false));
    if (degree_pin)
      degree_pin->digital_write(segment_level(false));

    cycles_to_skip = blank_cycles;
    return;
  }

  // Якщо ми тут і були в blank-фазі — виходимо з неї та малюємо наступний кадр
  in_blank_phase = false;

  if (iterate_digits) {
    // ---- РЕЖИМ: перебір цифр ----

    // вимикаємо поточний digit (запам’ятований)
    if (auto digit_pin = digit_pins[current_frame])
      digit_pin->digital_write(digit_level(false));

    // наступна цифра
    if (!digit_pins.empty())
      current_frame = (current_frame + 1) % digit_pins.size();

    auto raw_digit = buffer_[current_frame];
    for (const auto &segment_pin : segment_pins) {
      const bool segment_on = raw_digit & 0x01;
      raw_digit >>= 1;
      bit_count += segment_on ? 1 : 0;
      segment_pin->digital_write(segment_level(segment_on));
    }

    if (auto digit_pin = digit_pins[current_frame])
      digit_pin->digital_write(digit_level(true));

  } else {
    // ---- РЕЖИМ: перебір сегментів ----

    if (!segment_pins.empty())
      segment_pins[current_frame]->digital_write(segment_level(false));

    if (!segment_pins.empty())
      current_frame = (current_frame + 1) % segment_pins.size();

    auto const *raw_digit = buffer_;
    for (const auto &digit_pin : digit_pins) {
      const bool digit_on = (*raw_digit) & (0x01 << current_frame);
      bit_count += digit_on ? 1 : 0;
      if (digit_pin)
        digit_pin->digital_write(digit_level(digit_on));
      raw_digit++;
    }

    if (!segment_pins.empty())
      segment_pins[current_frame]->digital_write(segment_level(true));
  }

  // двокрапка та градус — тільки у фазі показу, а не в blank
  if (colon_pin)
    colon_pin->digital_write(segment_level(colon_on && current_frame == 0));

  if (degree_pin)
    degree_pin->digital_write(segment_level(degree_on && current_frame == 0));

  cycles_to_skip = intensity_delay + (compensate_brightness ? bit_count : 0);
}

// ===== НИЖЧЕ – ТВОЇ СТАРІ МЕТОДИ, МАЙЖЕ БЕЗ ЗМІН =====

void LcdDigitsComponent::set_segment_pins(std::vector<GPIOPin *> segment_pins) {
  ESP_LOGV(TAG, "Setting up segment pins");
  assert(timer == nullptr);
  interrupt_data_.segment_pins = std::move(segment_pins);
  interrupt_data_.segment_pins.resize(
      std::min(size_t(8), interrupt_data_.segment_pins.size()));
}

void LcdDigitsComponent::set_degree_pin(GPIOPin *arg) {
  ESP_LOGV(TAG, "Setting up degree pin");
  assert(timer == nullptr);
  interrupt_data_.degree_pin = arg;
}

void LcdDigitsComponent::set_colon_pin(GPIOPin *arg) {
  ESP_LOGV(TAG, "Setting up colon");
  assert(timer == nullptr);
  interrupt_data_.colon_pin = arg;
}

void LcdDigitsComponent::set_digit_pins(std::vector<GPIOPin *> digit_pins) {
  ESP_LOGV(TAG, "Setting up digit pins");
  assert(timer == nullptr);
  interrupt_data_.digit_pins = std::move(digit_pins);
  interrupt_data_.digit_pins.resize(
      std::min(size_t(max_digit_count), interrupt_data_.digit_pins.size()));
}

void LcdDigitsComponent::set_writer(lcd_digits_writer_t &&writer) {
  ESP_LOGV(TAG, "Setting up writer");
  assert(timer == nullptr);
  writer_ = std::move(writer);
}

void LcdDigitsComponent::set_display_type(DisplayType arg) {
  ESP_LOGV(TAG, "set display type: %d", arg);
  InterruptLock lock;
  interrupt_data_.display_type = arg;
}

void LcdDigitsComponent::set_compensate_brightness(bool arg) {
  ESP_LOGV(TAG, "Setting up brightness to %d", arg);
  InterruptLock lock;
  interrupt_data_.compensate_brightness = arg;
}

void LcdDigitsComponent::set_iterate_digits(bool arg) {
  ESP_LOGV(TAG, "Setting up iterate digits to %d", arg);
  InterruptLock lock;
  interrupt_data_.iterate_digits = arg;
}

void LcdDigitsComponent::set_intensity(uint8_t arg) {
  ESP_LOGV(TAG, "Setting up intensity to %d", arg);
  InterruptLock lock;
  interrupt_data_.intensity_delay = (15 - arg);
}

void LcdDigitsComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "LCD Digits:");
  for (auto *pin : interrupt_data_.digit_pins) {
    if (pin) {
      LOG_PIN("  Digit Pin: ", pin);
    }
  }
  for (auto *pin : interrupt_data_.segment_pins) {
    LOG_PIN("  Segment Pin: ", pin);
  }
  LOG_PIN("  Semicolon Pin: ", interrupt_data_.colon_pin);
  LOG_PIN("  Degree Pin: ", interrupt_data_.degree_pin);
}

void LcdDigitsComponent::update() {
  if (writer_.has_value())
    (*writer_)(*this);
  ESP_LOGV(TAG, "Updating interrupt data");
  InterruptLock lock;
  static_cast<LcdData &>(interrupt_data_) = display_data_;
}

uint8_t LcdDigitsComponent::print(const char *str) {
  ESP_LOGV(TAG, "Printing %s", str);
  return print(0, str);
}

void LcdDigitsComponent::set_degree_on(bool arg) {
  ESP_LOGV(TAG, "Setting degree on %d", arg);
  display_data_.degree_on = arg;
}

void LcdDigitsComponent::set_colon_on(bool arg) {
  ESP_LOGV(TAG, "Setting colon on %d", arg);
  display_data_.colon_on = arg;
}

void LcdDigitsComponent::set_mode(LcdDigitsComponent::Mode mode) {
  if (mode_ == mode)
    return;

  switch (mode) {
  case BufferMode:
    if (timer)
      timerAlarmEnable(timer);
    break;
  case ProgressMode:
    if (timer)
      timerAlarmDisable(timer);
    break;
  case DisabledMode:
    if (timer)
      timerAlarmDisable(timer);
    break;
  }
  mode_ = mode;
}

void LcdDigitsComponent::set_progress(float progress) {
  set_mode(ProgressMode);

  const uint8_t total_digits = interrupt_data_.digit_pins.size();
  if (total_digits == 0)
    return;

  const uint8_t total_steps = 6 * total_digits;
  const uint8_t current_step = total_steps * progress;
  const uint8_t current_digit = current_step / 6;
  const uint8_t current_segment = 1 + current_step % 6;

  auto invert_if_not = [](bool value, bool condition) {
    return condition ? value : !value;
  };

  auto digit_level = [&](bool on) {
    const auto digit_on_level = interrupt_data_.display_type == CommonAnode;
    return invert_if_not(digit_on_level, on);
  };

  auto segment_level = [&](bool on) {
    const auto segment_on_level = interrupt_data_.display_type != CommonAnode;
    return invert_if_not(segment_on_level, on);
  };

  // off all digits
  for (auto *pin : interrupt_data_.digit_pins)
    if (pin)
      pin->digital_write(digit_level(false));

  // off all segments
  for (auto *pin : interrupt_data_.segment_pins)
    pin->digital_write(segment_level(false));

  if (current_segment < interrupt_data_.segment_pins.size())
    interrupt_data_.segment_pins[current_segment]->digital_write(segment_level(true));

  if (auto pin = interrupt_data_.digit_pins[current_digit])
    pin->digital_write(digit_level(true));
}

void LcdDigitsComponent::strftime(uint8_t pos, const char *format,
                                  ESPTime time) {
  ESP_LOGV(TAG, "Setting strftime pos: %u, format: %s", pos, format);
  char buffer[8];
  size_t ret = time.strftime(buffer, sizeof(buffer), format);
  if (ret > 0)
    print(pos, buffer);
}

void LcdDigitsComponent::setup() {
  ESP_LOGV(TAG, "Setting up");
  g_interrupt_data = &interrupt_data_;

  const auto setup_output_pin = [&](GPIOPin *pin, bool initial) {
    pin->setup();
    pin->pin_mode(esphome::gpio::Flags::FLAG_OUTPUT);
    pin->digital_write(initial);
  };

  for (const auto &pin : interrupt_data_.digit_pins) {
    if (pin) {
      setup_output_pin(pin, true);
    }
  }

  for (const auto &pin : interrupt_data_.segment_pins)
    setup_output_pin(pin, false);

  if (interrupt_data_.colon_pin)
    setup_output_pin(interrupt_data_.colon_pin, false);

  if (interrupt_data_.degree_pin)
    setup_output_pin(interrupt_data_.degree_pin, false);

  assert(timer == nullptr);
  timer = timerBegin(0, 80, true);
  if (timer) {
    timerAttachInterrupt(timer, &s_timer_intr, true);
    // 4000 тік при 1 МГц → ~4 мс
    timerAlarmWrite(timer, 4000, true);
    timerAlarmEnable(timer);
  } else {
    ESP_LOGE(TAG, "Can't initialize timer");
  }
}

uint8_t LcdDigitsComponent::print(uint8_t start_pos, const char *in_str) {
  ESP_LOGV(TAG, "Printing pos: %d %s", start_pos, in_str);
  const auto &digits_count = interrupt_data_.digit_pins.size();
  if (start_pos >= digits_count)
    return 0;

  uint8_t pos = start_pos;
  auto &buffer = display_data_.buffer_;
  for (auto str = in_str; *str != '\0'; str++) {
    uint8_t data = UNKNOWN_CHAR;
    if (*str >= ' ' && *str <= '~')
      data = ASCII_TO_RAW[*str - ' '];

    if (data == UNKNOWN_CHAR) {
      ESP_LOGW(TAG,
               "Encountered character '%c' with no representation while "
               "translating string!",
               *str);
    }

    if (*str == '.') {
      if (pos != start_pos)
        pos--;
      buffer[digits_count - pos - 1] |= 0b10000000;
    } else {
      if (pos >= digits_count) {
        ESP_LOGE(TAG, "String '%s' is too long for the display!", in_str);
        break;
      }
      buffer[digits_count - pos - 1] = data;
    }
    pos++;
  }

  return pos - start_pos;
}

uint8_t LcdDigitsComponent::printf(uint8_t pos, const char *format, ...) {
  ESP_LOGV(TAG, "printf pos: %d %s", pos, format);
  va_list arg;
  va_start(arg, format);
  char buffer[8];
  int ret = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);
  if (ret > 0)
    return print(pos, buffer);
  return 0;
}

void LcdDigitsComponent::set_raw(uint64_t raw) {
  ESP_LOGV(TAG, "Setting raw %llu", (unsigned long long) raw);
  for (auto &d : display_data_.buffer_) {
    d = raw & 0xff;
    raw >>= 8;
  }
}

} // namespace lcd_digits
} // namespace esphome

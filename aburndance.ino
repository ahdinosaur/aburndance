/*

TODO:
- change mode or param on button up
- change brightness while holding prev mode
- change color correction while holding next mode

*/

#include <Encoder.h>
#include "FastLED.h"
#include "LinkedList.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"


#define LED_A_DATA_PIN 13
#define LED_A_CLOCK_PIN 12
#define LED_B_DATA_PIN 27
#define LED_B_CLOCK_PIN 15
#define LED_CHIPSET APA102
#define LED_COLOR_ORDER BGR

#define LED_VOLTAGE 5
#define LED_MAX_MILLIAMPS 2000

#define LED_COLOR_CORRECTION TypicalLEDStrip

//
// FastLED provides these pre-conigured incandescent color profiles:
//   - Candle: 1900K
//   - Tungsten40W: 2850K
//   - Tungsten100W: 2850K
//   - Halogen: 3200K
//   - CarbonArc: 5200K
//   - HighNoonSun: 5400K
//   - DirectSunlight: 6000K
//   - OvercastSky: 7000K
//   - ClearBlueSky: 20000K
// FastLED provides these pre-configured gaseous-light color profiles:
//     WarmFluorescent, StandardFluorescent, CoolWhiteFluorescent,
//     FullSpectrumFluorescent, GrowLightFluorescent, BlackLightFluorescent,
//     MercuryVapor, SodiumVapor, MetalHalide, HighPressureSodium,
// FastLED also provides an "Uncorrected temperature" profile
//    UncorrectedTemperature;

#define NUM_TEMPERATURES 10
ColorTemperature color_temperatures[NUM_TEMPERATURES] = {
  UncorrectedTemperature,
  Candle,
  Tungsten40W,
  Tungsten100W,
  Halogen,
  CarbonArc,
  HighNoonSun,
  DirectSunlight,
  OvercastSky,
  ClearBlueSky
};

#define DEFAULT_LED_COLOR_TEMPERATURE UncorrectedTemperature

#define NUM_BUTTONS 5

#define MODE_PREV_BUTTON_INDEX 0
#define MODE_PREV_BUTTON_PIN 25
#define MODE_NEXT_BUTTON_INDEX 1
#define MODE_NEXT_BUTTON_PIN 33

#define PARAM_PREV_BUTTON_INDEX 2
#define PARAM_PREV_BUTTON_PIN 32
#define PARAM_NEXT_BUTTON_INDEX 3
#define PARAM_NEXT_BUTTON_PIN 14

#define PARAM_RESET_BUTTON_INDEX 4
#define PARAM_RESET_BUTTON_PIN 26

#define PARAM_ENCODER_PIN_A 4
#define PARAM_ENCODER_PIN_B 21

int8_t button_pins[NUM_BUTTONS] = {
  MODE_PREV_BUTTON_PIN,
  MODE_NEXT_BUTTON_PIN,
  PARAM_PREV_BUTTON_PIN,
  PARAM_NEXT_BUTTON_PIN,
  PARAM_RESET_BUTTON_PIN
};
/*
button_pins[MODE_PREV_BUTTON_INDEX] = MODE_PREV_BUTTON_PIN;
button_pins[MODE_NEXT_BUTTON_INDEX] = MODE_NEXT_BUTTON_PIN;
button_pins[PARAM_PREV_BUTTON_INDEX] = PARAM_PREV_BUTTON_PIN;
button_pins[PARAM_NEXT_BUTTON_INDEX] = PARAM_NEXT_BUTTON_PIN;
button_pins[PARAM_RESET_BUTTON_INDEX] = PARAM_RESET_BUTTON_PIN;
*/

#define BUTTON_HOLD_MILLIS 2000
bool button_is_pressed_down[NUM_BUTTONS] = { false };
bool button_is_pressed_up[NUM_BUTTONS] = { false };
bool button_is_held[NUM_BUTTONS] = { false };
uint32_t button_start_press_time[NUM_BUTTONS] = { 0 };

#define NUM_LEDS_A 36
#define NUM_LEDS_B 36

#define FRAMES_PER_SECOND 60

CRGB leds_a[NUM_LEDS_A];
CRGB leds_b[NUM_LEDS_B];

Encoder encoder(PARAM_ENCODER_PIN_A, PARAM_ENCODER_PIN_B);

#define NUM_MODES 3
int8_t mode_index = 0;

#define DEFAULT_BRIGHTNESS 128
uint8_t brightness = DEFAULT_BRIGHTNESS;
uint8_t temperature_index = 0;

#define MAX_NUM_PARAMS 3
int8_t num_params_by_mode[NUM_MODES] = { 2, 2, 1 };
int16_t params[NUM_MODES][MAX_NUM_PARAMS] = {
  0, 168, 0,
  128, 0, 0,
  64, 0, 0
};
int8_t param_index = 0;

TaskHandle_t ControlTaskHandle, RenderTaskHandle;

void setup () {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  Serial.begin(115200);
  Serial.println("start");

  control_setup();
  render_setup();

//  xTaskCreatePinnedToCore(
//    ControlTask,                  /* pvTaskCode */
//    "Control",            /* pcName */
//    1000,                   /* usStackDepth */
//    NULL,                   /* pvParameters */
//    1,                      /* uxPriority */
//    &ControlTaskHandle,                 /* pxCreatedTask */
//    1                       /* xCoreID */
//  );
//
//  xTaskCreatePinnedToCore(
//    RenderTask,                  /* pvTaskCode */
//    "Render",            /* pcName */
//    1000,                   /* usStackDepth */
//    NULL,                   /* pvParameters */
//    1,                      /* uxPriority */
//    &RenderTaskHandle,                 /* pxCreatedTask */
//    0                       /* xCoreID */
//  );
}

void loop () {
  control_loop();
  render_loop();
}

void control_setup () {
  pinMode(MODE_PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(MODE_NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PARAM_PREV_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PARAM_NEXT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PARAM_RESET_BUTTON_PIN, INPUT_PULLUP);
}

void control_loop () {
  read_mode_buttons();
  read_param_buttons();  
  read_encoder();
}

void render_setup () {
  FastLED.addLeds<LED_CHIPSET, LED_A_DATA_PIN, LED_A_CLOCK_PIN, LED_COLOR_ORDER>(leds_a, NUM_LEDS_A);
  FastLED.addLeds<LED_CHIPSET, LED_B_DATA_PIN, LED_B_CLOCK_PIN, LED_COLOR_ORDER>(leds_b, NUM_LEDS_B);
  FastLED.setCorrection(LED_COLOR_CORRECTION);
  FastLED.setMaxRefreshRate(FRAMES_PER_SECOND);
  FastLED.countFPS(30);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_MAX_MILLIAMPS);
}

void render_loop () {
  Serial.print("fps: ");
  Serial.print(LEDS.getFPS());
  Serial.println();

  FastLED.setTemperature(color_temperatures[temperature_index]);
  FastLED.setBrightness(brightness);

  if (button_is_held[MODE_PREV_BUTTON_INDEX] || button_is_held[MODE_NEXT_BUTTON_INDEX]) {
    solid_white();
  } else {
    run_mode();
  }
  FastLED.show();
  fade_all();
}

/* controls */


bool read_button_press (int8_t button_index) {
  int8_t button_pin = button_pins[button_index];
  int8_t button_was_pressed_down = button_is_pressed_down[button_index];
  int8_t button_was_held = button_is_held[button_index];
  int64_t button_pressed_time = button_start_press_time[button_index];
  
  int8_t button_value = digitalRead(button_pin);

  if (button_value == LOW) {
    // button is pushed down
    
    button_is_pressed_down[button_index] = true;
    button_is_pressed_up[button_index] = false;
    if (button_pressed_time == 0) {
      button_start_press_time[button_index] = millis();
    } else if (millis() - button_pressed_time > BUTTON_HOLD_MILLIS) {
      button_is_held[button_index] = true;
    }
  } else if (button_value == HIGH) {
    // button is not pushed down
    
    button_start_press_time[button_index] = 0;
    button_is_pressed_down[button_index] = false;
    button_is_held[button_index] = false;
    
    if (button_was_pressed_down && !button_was_held) {
      button_is_pressed_up[button_index] = true;
    } else {
      button_is_pressed_up[button_index] = false;
    }
  }
}

void read_mode_buttons () {
  bool has_mode_changed = false;
  bool buttons_were_held = button_is_held[MODE_PREV_BUTTON_INDEX] || button_is_held[MODE_NEXT_BUTTON_INDEX];

  read_button_press(MODE_PREV_BUTTON_INDEX);
  if (button_is_pressed_up[MODE_PREV_BUTTON_INDEX]) {
    mode_index = mod(mode_index - 1, NUM_MODES);
    has_mode_changed = true;
  }

  read_button_press(MODE_NEXT_BUTTON_INDEX);
  if (button_is_pressed_up[MODE_NEXT_BUTTON_INDEX]) {
    mode_index = mod(mode_index + 1, NUM_MODES);
    has_mode_changed = true;
  }

  bool buttons_are_held = button_is_held[MODE_PREV_BUTTON_INDEX] || button_is_held[MODE_NEXT_BUTTON_INDEX];

  if (buttons_were_held && !buttons_are_held) {
    encoder.write(params[mode_index][param_index]);
  } else if (!buttons_were_held && buttons_are_held) {
    if (button_is_held[MODE_PREV_BUTTON_INDEX]) {
      encoder.write(brightness);
    } else if (button_is_held[MODE_NEXT_BUTTON_INDEX]) {
      encoder.write(temperature_index);
    }
  }
  
  if (has_mode_changed) {
    Serial.print("mode: ");
    Serial.print(mode_index);
    Serial.println();

    param_index = 0;
    encoder.write(params[mode_index][param_index]);
  }
}

void read_param_buttons () {
  bool has_param_changed = false;
  int8_t num_params = num_params_by_mode[mode_index];

  read_button_press(PARAM_PREV_BUTTON_INDEX);
  if (button_is_pressed_up[PARAM_PREV_BUTTON_INDEX]) {
    param_index = mod(param_index - 1, num_params);
    has_param_changed = true;
  }

  read_button_press(PARAM_NEXT_BUTTON_INDEX);
  if (button_is_pressed_up[PARAM_NEXT_BUTTON_INDEX]) {
    param_index = mod(param_index + 1, num_params);
    has_param_changed = true;
  }

  if (has_param_changed) {
    Serial.print("param: ");
    Serial.print(param_index);
    Serial.println();

    encoder.write(params[mode_index][param_index]);
  }
}

void read_encoder () {
  int16_t encoder_value = encoder.read();
  int16_t value = constrain(encoder_value, 0, 255);
  encoder.write(value);

  if (button_is_held[MODE_PREV_BUTTON_INDEX]) {
    brightness = value;
    Serial.print("brightness =");
    Serial.print(brightness);
    Serial.println();
    return;
  }

  if (button_is_held[MODE_NEXT_BUTTON_INDEX]) {
    temperature_index = map(value, 0, 255, 0, 10);
    Serial.print("temperature index =");
    Serial.print(temperature_index);
    Serial.println();
    return;
  }
  
  int16_t current_param = params[mode_index][param_index];
  if (value != current_param) {
    params[mode_index][param_index] = value;

    Serial.print("param[");
    Serial.print(param_index);
    Serial.print("] =");
    Serial.print(value);
    Serial.println();
  }

  read_button_press(PARAM_RESET_BUTTON_INDEX);
  if (button_is_pressed_up[PARAM_RESET_BUTTON_INDEX]) {
    Serial.println("reset encoders to zero");
    encoder.write(0);
  }
}

uint8_t read_brightness () {
  uint8_t sensor = DEFAULT_BRIGHTNESS;
  uint8_t brightness = map(sensor, 0, 255, 0, 255);
  Serial.print("brightness: ");
  Serial.print(brightness);
  Serial.println();
  return brightness;
}

/* view modes */

// TODO strobe
// TODO chase

void solid_white () {
  fill_solid(leds_a, NUM_LEDS_A, CRGB::White);
  fill_solid(leds_b, NUM_LEDS_B, CRGB::White);
}

void run_mode () {
  switch (mode_index) {
    case 0:
      rainbow_loop();
      break;
    case 1:
      star_field();
      break;
    case 2:
      convergence();
      break;
  }
}

float rainbow_index = 0.0;

void rainbow_loop () {
  float speed = mapl(params[mode_index][0], 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  float zoom = mapl(params[mode_index][1], 0.0, 255.0, 0.05, 35.0);
  Serial.print("zoom: ");
  Serial.print(zoom);
  Serial.println();

  fill_rainbow(leds_a, NUM_LEDS_A, floor(rainbow_index), floor(zoom));
  fill_rainbow(leds_b, NUM_LEDS_B, floor(rainbow_index), floor(zoom));

  rainbow_index = fmod(rainbow_index + speed, 256.0);
}

struct Star {
  float index;
  CRGB color;
};
LinkedList<Star*> stars = LinkedList<Star*>();

void star_field () {
  float speed = mapl(params[mode_index][0], 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  float probability_of_new_star = mapf(params[mode_index][1], 0.0, 255.0, 0.01, 0.25);
  Serial.print("probability of new star: ");
  Serial.print(probability_of_new_star);
  Serial.println();

  bool has_new_star = random_float() < probability_of_new_star;
  if (has_new_star) {
    struct Star* star = (Star*) malloc(sizeof(Star));
    star->index = 0;
    star->color = random_color();
    stars.add(star);
  }

  Serial.print("num stars: ");
  Serial.print(stars.size());
  Serial.println();

  int star_index = 0;
  while (star_index < stars.size()) {
    struct Star* star = stars.get(star_index);
    int led_index = floor(star->index);
    if (led_index >= (NUM_LEDS_A + NUM_LEDS_B)) {
      stars.remove(star_index);
      free(star);
    } else {
      for (int i = 0; i < speed; i++) {
        if ((led_index + i) < NUM_LEDS_A) {
          leds_a[NUM_LEDS_A - (led_index + i) - 1] = star->color;
        } else {
          leds_b[led_index + i - NUM_LEDS_A] = star->color;
        }
      }
      star->index += speed;
      star_index++;
    }
  }
}

float convergence_index = 0;
CRGB convergence_color = random_color();
uint16_t convergence_midpoint = NUM_LEDS_A / 2;

void convergence () {
  float speed = mapl(params[mode_index][0], 0.0, 255.0, 0.01, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  if (convergence_index > convergence_midpoint) {
    convergence_color = random_color();
    convergence_index = 0;
  }

  convergence_index += speed;

  fill_solid(leds_a, convergence_index, convergence_color);
  fill_solid(leds_a + (NUM_LEDS_A - (uint16_t) convergence_index), convergence_index, convergence_color);

  fill_solid(leds_b, convergence_index, convergence_color);
  fill_solid(leds_b + (NUM_LEDS_B - (uint16_t) convergence_index), convergence_index, convergence_color);
}

void fade_all () {
  for (int i = 0; i < NUM_LEDS_A; i++) {
    leds_a[i].nscale8(250);
  }
  for (int i = 0; i < NUM_LEDS_B; i++) {
    leds_b[i].nscale8(250);
  }
}

/* util */

uint8_t random_param () {
  return random(0, 256);
}

float random_float () {
  return (float) random(0, LONG_MAX) / (LONG_MAX - 1);
}

CRGB random_color () {
  uint8_t hue = random(0, 256);
  uint8_t saturation = 255;
  uint8_t value = 255;
  return CHSV(hue, saturation, value);
}

float mapf (float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float mapl (float x, float in_min, float in_max, float out_min, float out_max) {
  out_min = log(out_min);
  out_max = log(out_max);

  float scale = (out_max - out_min) / (in_max - in_min);

  return exp(out_min + scale * (x - in_min));
}

int mod (int x, int y) {
  return x < 0 ? ((x + 1) % y ) + y - 1 : x % y;
}

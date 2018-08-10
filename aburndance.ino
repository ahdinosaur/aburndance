#include <Encoder.h>
#include "FastLED.h"
#include "LinkedList.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"

#define NUM_LEDS (36)
#define FRAMES_PER_SECOND 60

#define LED_DATA_PIN 13
#define LED_CLOCK_PIN 12
#define LED_CHIPSET APA102
#define LED_COLOR_ORDER BGR

#define LED_VOLTAGE 5
#define LED_MAX_MILLIAMPS 2000

#define LED_COLOR_CORRECTION TypicalSMD5050

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

#define LED_COLOR_TEMPERATURE UncorrectedTemperature

#define MODE_PREV_BUTTON_PIN 26
#define MODE_NEXT_BUTTON_PIN 25

#define PARAM_PREV_BUTTON_PIN 34
#define PARAM_NEXT_BUTTON_PIN 39

#define PARAM_RESET_BUTTON_PIN 36

#define PARAM_ENCODER_PIN_A 4
#define PARAM_ENCODER_PIN_B 21

CRGB leds[NUM_LEDS];

Encoder encoder(PARAM_ENCODER_PIN_A, PARAM_ENCODER_PIN_B);

#define NUM_MODES 3
int8_t mode_index = 0;

#define MAX_NUM_PARAMS 3
int8_t num_params_by_mode[NUM_MODES] = { 2, 2, 1 };
int16_t params[NUM_MODES][MAX_NUM_PARAMS] = { 0 };
int8_t param_index = 0;

uint8_t brightness = 255;

TaskHandle_t ControlTaskHandle, RenderTaskHandle;

void setup () {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  Serial.begin(115200);
  Serial.println("start");

  xTaskCreatePinnedToCore(
    ControlTask,                  /* pvTaskCode */
    "Control",            /* pcName */
    1000,                   /* usStackDepth */
    NULL,                   /* pvParameters */
    1,                      /* uxPriority */
    &ControlTaskHandle,                 /* pxCreatedTask */
    1                       /* xCoreID */
  );

  xTaskCreatePinnedToCore(
    RenderTask,                  /* pvTaskCode */
    "Render",            /* pcName */
    1000,                   /* usStackDepth */
    NULL,                   /* pvParameters */
    1,                      /* uxPriority */
    &RenderTaskHandle,                 /* pxCreatedTask */
    0                       /* xCoreID */
  );
}

void loop () {

}

void ControlTask (void * parameter) {
  pinMode(MODE_PREV_BUTTON_PIN, INPUT);
  pinMode(MODE_NEXT_BUTTON_PIN, INPUT);
  pinMode(PARAM_PREV_BUTTON_PIN, INPUT);
  pinMode(PARAM_NEXT_BUTTON_PIN, INPUT);
  pinMode(PARAM_RESET_BUTTON_PIN, INPUT);

  for (;;) {
    read_mode_index();
    read_param_index();
    read_param_encoder();

    delay(20);
  }
}

void RenderTask (void * parameter) {
  FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_CLOCK_PIN, LED_COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(LED_COLOR_CORRECTION);
  FastLED.setTemperature(LED_COLOR_TEMPERATURE);
  FastLED.setMaxRefreshRate(FRAMES_PER_SECOND);
  FastLED.countFPS(30);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_MAX_MILLIAMPS);

  for (;;) {
    Serial.print("fps: ");
    Serial.print(LEDS.getFPS());
    Serial.println();

    LEDS.setBrightness(brightness);

    run_mode();
    FastLED.show();
    fade_all();
  }
}

/* controls */

void read_mode_index () {
  bool has_mode_changed = false;

  int8_t prev_mode_sensor = digitalRead(MODE_PREV_BUTTON_PIN);
  if (prev_mode_sensor == HIGH) {
    mode_index = mod(mode_index - 1, NUM_MODES);
    has_mode_changed = true;
  }

  int8_t next_mode_sensor = digitalRead(MODE_NEXT_BUTTON_PIN);
  if (next_mode_sensor == HIGH) {
    mode_index = mod(mode_index + 1, NUM_MODES);
    has_mode_changed = true;
  }

  if (has_mode_changed) {
    Serial.print("mode: ");
    Serial.print(mode_index);
    Serial.println();

    param_index = 0;
    encoder.write(params[mode_index][param_index]);

    delay(250);
  }
}

void read_param_index () {
  bool has_param_changed = false;
  int8_t num_params = num_params_by_mode[mode_index];

  int8_t prev_param_sensor = digitalRead(PARAM_PREV_BUTTON_PIN);
  if (prev_param_sensor == HIGH) {
    param_index = mod(param_index - 1, num_params);
    has_param_changed = true;
  }

  int8_t next_param_sensor = digitalRead(PARAM_NEXT_BUTTON_PIN);
  if (next_param_sensor == HIGH) {
    param_index = mod(param_index + 1, num_params);
  }

  if (has_param_changed) {
    Serial.print("param: ");
    Serial.print(param_index);
    Serial.println();

    encoder.write(params[mode_index][param_index]);

    delay(250);
  }
}

void read_param_encoder () {
  int16_t next_param_raw = encoder.read();
  int16_t current_param = params[mode_index][param_index];
  if (next_param_raw != current_param) {
    int16_t next_param = constrain(next_param_raw, 0, 255);
    params[mode_index][param_index] = next_param;
    encoder.write(next_param);

    Serial.print("param[");
    Serial.print(param_index);
    Serial.print("] =");
    Serial.print(next_param);
    Serial.println();
  }

  int8_t reset_param_sensor = digitalRead(PARAM_RESET_BUTTON_PIN);
  if (reset_param_sensor == HIGH) {
    Serial.println("reset encoders to zero");
    encoder.write(0);
    delay(250);
  }
}

uint8_t read_brightness () {
  uint8_t sensor = 255;
  uint8_t brightness = map(sensor, 0, 255, 0, 255);
  Serial.print("brightness: ");
  Serial.print(brightness);
  Serial.println();
  return brightness;
}

/* view modes */

// TODO strobe
// TODO chase

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

  fill_rainbow(leds, NUM_LEDS, floor(rainbow_index), floor(zoom));

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
    if (led_index >= NUM_LEDS) {
      stars.remove(star_index);
      free(star);
    } else {
      for (int i = 0; i < speed; i++) {
        leds[led_index + i] = star->color;
      }
      star->index += speed;
      star_index++;
    }
  }
}

float convergence_index = 0;
CRGB convergence_color = random_color();
uint16_t convergence_midpoint = NUM_LEDS / 2;

void convergence () {
  float speed = mapl(params[mode_index][0], 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  if (convergence_index > convergence_midpoint) {
    convergence_color = random_color();
    convergence_index = 0;
  }

  convergence_index += speed;

  fill_solid(leds, convergence_index, convergence_color);
  fill_solid(leds + (NUM_LEDS - (uint16_t) convergence_index), convergence_index, convergence_color);
}

void fade_all () {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
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

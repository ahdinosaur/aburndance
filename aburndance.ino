#include <Encoder.h>
#include "FastLED.h"
#include "LinkedList.h"

#define NUM_LEDS (60*2)
#define FRAMES_PER_SECOND 60

#define LED_DATA_PIN 25
#define LED_CLOCK_PIN 26
#define LED_CHIPSET APA102
#define LED_COLOR_ORDER BGR

#define LED_VOLTAGE 5
// #define LED_MAX_MILLIAMPS 1750 // maxed by battery
#define LED_MAX_MILLIAMPS 750 // maxed by power converter

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

#define LED_COLOR_TEMPERATURE DirectSunlight

#define MODE_BUTTON_PIN 4

CRGB leds[NUM_LEDS];

Encoder encoder_a(32, 14);
Encoder encoder_b(15, 33);
Encoder encoder_c(27, 12);
Encoder encoder_d(13, 21);
 
void setup () {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_CLOCK_PIN, LED_COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(LED_COLOR_CORRECTION);
  FastLED.setTemperature(LED_COLOR_TEMPERATURE);
  FastLED.setMaxRefreshRate(FRAMES_PER_SECOND);
  FastLED.countFPS(30);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_MAX_MILLIAMPS);

  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Start");
}

int16_t param_a = 0;
int16_t param_b = 0;
int16_t param_c = 0;
int16_t param_d = 0;

uint8_t brightness = 255;

uint8_t num_modes = 3;
uint8_t mode_index = 0;

void loop () {
  Serial.print("fps: ");
  Serial.print(LEDS.getFPS());
  Serial.println();

  read_mode();
  read_encoders();
  
  brightness = get_brightness(param_d);
  LEDS.setBrightness(brightness);
  
  run_mode();
  FastLED.show();
  fade_all();
}

void read_mode () {
  int8_t mode_sensor = digitalRead(MODE_BUTTON_PIN);
  if (mode_sensor == LOW) {
    mode_index = (mode_index + 1) % num_modes;
    Serial.print("mode: ");
    Serial.print(mode_index);
    Serial.println();
    delay(250);
  }
}

void read_encoders () {
  int16_t new_param_a = encoder_a.read();
  if (new_param_a != param_a) {
     param_a = constrain(new_param_a, 0, 255);
     encoder_a.write(param_a);
     Serial.print("param a = ");
     Serial.print(param_a);
     Serial.println();
  }
  int16_t new_param_b = encoder_b.read();
  if (new_param_b != param_b) {
     param_b = constrain(new_param_b, 0, 255);
     encoder_b.write(param_b);
     Serial.print("param b = ");
     Serial.print(param_b);
     Serial.println();
  }
  int16_t new_param_c = encoder_c.read();
  if (new_param_c != param_c) {
     param_c = constrain(new_param_c, 0, 255);
     encoder_c.write(param_c);
     Serial.print("param c = ");
     Serial.print(param_c);
     Serial.println();
  }
  int16_t new_param_d = encoder_d.read();
  if (new_param_d != param_d) {
     param_d = constrain(new_param_d, 0, 255);
     encoder_d.write(param_d);
     Serial.print("param d = ");
     Serial.print(param_d);
     Serial.println();
  }
  // if a character is sent from the serial monitor,
  // reset both back to zero.
  if (Serial.available()) {
    Serial.read();
    Serial.println("Reset encoders to zero");
    encoder_a.write(0);
    encoder_b.write(0);
    encoder_c.write(0);
    encoder_d.write(0);
  }
}

uint8_t get_brightness (int16_t param) {
  uint8_t brightness = map(param, 0, 255, 255, 0);
  Serial.print("brightness: ");
  Serial.print(brightness);
  Serial.println();
  return brightness;
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

uint8_t rainbow_index = 0;

void rainbow_loop () {
  float speed = mapf(param_a, 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  float zoom = mapl(param_b, 0.0, 255.0, 0.05, 35.0);
  Serial.print("zoom: ");
  Serial.print(zoom);
  Serial.println(); 

  fill_rainbow(leds, NUM_LEDS, rainbow_index, floor(zoom));

  rainbow_index = (rainbow_index + (uint8_t) speed) % 256;
}

struct Star {
  float index;
  CRGB color;
};
LinkedList<Star*> stars = LinkedList<Star*>();

void star_field () {
  float speed = mapf(param_a, 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  float probability_of_new_star = mapf(param_b, 0.0, 255.0, 0.01, 0.25);
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
  float speed = mapl(param_a, 0.0, 255.0, 0.1, 10.0);
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
  for(int i = 0; i < NUM_LEDS; i++) {
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


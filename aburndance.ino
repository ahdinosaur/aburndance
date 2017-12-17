#include <Encoder.h>
#include "FastLED.h"
#include "LinkedList.h"

#define NUM_LEDS 60 * 2
#define FRAMES_PER_SECOND 120

#define LED_DATA_PIN 25
#define LED_CLOCK_PIN 26
#define LED_CHIPSET APA102
#define LED_COLOR_ORDER BGR

#define LED_COLOR_CORRECTION TypicalSMD5050

//
// FastLED provides these pre-conigured incandescent color profiles:
//     Candle, Tungsten40W, Tungsten100W, Halogen, CarbonArc,
//     HighNoonSun, DirectSunlight, OvercastSky, ClearBlueSky,
// FastLED provides these pre-configured gaseous-light color profiles:
//     WarmFluorescent, StandardFluorescent, CoolWhiteFluorescent,
//     FullSpectrumFluorescent, GrowLightFluorescent, BlackLightFluorescent,
//     MercuryVapor, SodiumVapor, MetalHalide, HighPressureSodium,
// FastLED also provides an "Uncorrected temperature" profile
//    UncorrectedTemperature;

#define LED_COLOR_TEMPERATURE DirectSunlight

#define MODE_BUTTON_PIN 4

CRGB leds[NUM_LEDS];

Encoder encoderA(32, 14);
Encoder encoderB(15, 33);
Encoder encoderC(27, 21);
Encoder encoderD(12, 13);
 
void setup () {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);

  FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_CLOCK_PIN, LED_COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(LED_COLOR_CORRECTION);
  FastLED.setTemperature(LED_COLOR_TEMPERATURE);
  FastLED.setMaxRefreshRate(FRAMES_PER_SECOND);
  FastLED.countFPS(25);

  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Start");
}

int16_t paramA = 0;
int16_t paramB = 0;
int16_t paramC = 0;
int16_t paramD = 0;

uint8_t brightness = 0;

uint8_t num_modes = 2;
uint8_t mode_index = 0;

void loop () {
  Serial.print("fps: ");
  Serial.print(LEDS.getFPS());
  Serial.println();

  read_mode();
  read_encoders();
  
  brightness = get_brightness(paramD);
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
  int16_t newParamA = encoderA.read();
  int16_t newParamB = encoderB.read();
  int16_t newParamC = encoderC.read();
  int16_t newParamD = encoderD.read();
  if (
    newParamA != paramA ||
    newParamB != paramB ||
    newParamC != paramC ||
    newParamD != paramD
   ) {
    paramA = constrain(newParamA, 0, 256);
    paramB = constrain(newParamB, 0, 256);
    paramC = constrain(newParamC, 0, 256);
    paramD = constrain(newParamD, 0, 256);
    encoderA.write(paramA);
    encoderB.write(paramB);
    encoderC.write(paramC);
    encoderD.write(paramD);
    Serial.print("A = ");
    Serial.print(paramA);
    Serial.print(", B = ");
    Serial.print(paramB);
    Serial.print(", C = ");
    Serial.print(paramC);
    Serial.print(", D = ");
    Serial.print(paramD);
    Serial.println();

  }
  // if a character is sent from the serial monitor,
  // reset both back to zero.
  if (Serial.available()) {
    Serial.read();
    Serial.println("Reset encoders to zero");
    encoderA.write(0);
    encoderB.write(0);
    encoderC.write(0);
    encoderD.write(0);
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
  if (mode_index == 0) {
    star_field();
  }
}

struct Star {
  float index;
  CRGB color;
};
LinkedList<Star*> stars = LinkedList<Star*>();

void star_field () {
  float speed = mapf(paramA, 0.0, 255.0, 0.1, 10.0);
  Serial.print("speed: ");
  Serial.print(speed);
  Serial.println();

  float probability_of_new_star = mapf(paramB, 0.0, 255.0, 0.01, 0.25);
  Serial.print("probability of new star: ");
  Serial.print(probability_of_new_star);
  Serial.println();

  bool hasNewStar = random_float() < probability_of_new_star;
  if (hasNewStar) {
    struct Star* star = (Star*) malloc(sizeof(Star));
    star->index = 0;
    star->color = random_color();
    stars.add(star);
  }

  Serial.print("num stars: ");
  Serial.print(stars.size());
  Serial.println();


  int starIndex = 0;
  while (starIndex < stars.size()) {
    struct Star* star = stars.get(starIndex);
    int ledIndex = floor(star->index);
    if (ledIndex >= NUM_LEDS) {
      stars.remove(starIndex);
      free(star);
    } else {
      for (int i = 0; i < speed; i++) {
        leds[ledIndex + i] = star->color;
      }
      star->index += speed;
      starIndex++;
    }
  }
}

void fade_all () {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}

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

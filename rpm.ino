
#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

bool debug = true;

void setup() {
  analogWrite(3, 0);
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  if (!ts.begin()) {
    while (1);
  }
  pinMode(A0, INPUT);
  digitalWrite(A0, HIGH);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  for(int i = 0; i < 8; i++) {
    tft.setCursor(40*i + 10, 10);
    tft.fillRect(40*i + 14, 30, 2, 4, ILI9341_WHITE);
    tft.print(i);
  }

  render_buttons();
  
  for( byte i = 0; i < 255; i+=5 ) {
    analogWrite(3, i);
    delay(20);
  }

  for( uint32_t i = 0; i < 7000; i+=50) {
    render_line(i);
    delay(1);
  }

  for( uint32_t i = 7000; i > 1000; i-=50) {
    render_line(i);
    delay(1);
  }

  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(1);
  render(0);  
}

void render_buttons() {
  tft.setTextColor(tft.color565(100,100,100));
  tft.setTextSize(2);
  tft.fillRoundRect(50, 100, 40, 40, 5, tft.color565(0, 20, 20));
  tft.setCursor(65, 114);
  tft.print("D");
  tft.fillRoundRect(140, 100, 40, 40, 5, tft.color565(0, 20, 20));
  tft.setCursor(155, 114);
  tft.print("-");
  tft.fillRoundRect(230, 100, 40, 40, 5, tft.color565(0, 20, 20));
  tft.setCursor(245, 114);
  tft.print("+");
  
}

uint32_t probe() {
  auto period_start = millis();
  uint8_t probe_count = 0;
  uint32_t probes[5];
  
  if (!pulseIn(A0, LOW, 100000)) return 0;
  do {
    auto probe_start = micros();
    if (!pulseIn(A0, LOW, 100000)) return 0;
    probes[probe_count++] = micros() - probe_start;
  } while (probe_count < 5 && period_start - millis() < 100);
  
  if(probe_count == 0){
    return 0;
  } else {
    uint32_t result = 0;
    for (int i = 0; i < probe_count; i++) {
      result += probes[i];
    }
    return result / probe_count;
  }
}

void clear_debug() {
  tft.fillRect(0, tft.height() - 25, 100, tft.height(), ILI9341_BLACK);
}

void render(uint32_t microseconds) {
  uint32_t rpm = 60000000.0 / microseconds;
  if (debug) {
    clear_debug();
    tft.setCursor(0, tft.height() - 25);
    tft.print("t ");
    tft.println(microseconds);
    tft.print("Hz ");
    tft.println(1000000.0 / microseconds);
    tft.print("RPM ");
    tft.print(rpm);
  }
 
  render_line(rpm);
}

uint32_t calc_line_length(uint32_t rpm) {
  return rpm*40/1000;
}

void render_line(uint32_t rpm) {
  static uint32_t prev_line_length = 0;

  if (rpm > 8000) return;
  
  uint32_t line_length = calc_line_length(rpm);
  
  if(line_length > prev_line_length) {
    if (prev_line_length <= 3000) {
      uint32_t green_line_length = line_length > calc_line_length(3000) ? calc_line_length(3000) : line_length;
      tft.drawFastHLine(14, 36, green_line_length, ILI9341_GREEN);
      tft.drawFastHLine(14, 37, green_line_length, ILI9341_GREEN);
      tft.drawFastHLine(14, 38, green_line_length, ILI9341_GREEN);
      tft.drawFastHLine(14, 39, green_line_length, ILI9341_GREEN);
    }

    if (rpm > 3000 && prev_line_length <= 4000) {
      uint32_t yellow_line_length = line_length > calc_line_length(4000) ? calc_line_length(4000 - 3000) : line_length - calc_line_length(3000);
      tft.drawFastHLine(14 + calc_line_length(3000), 36, yellow_line_length, ILI9341_YELLOW);
      tft.drawFastHLine(14 + calc_line_length(3000), 37, yellow_line_length, ILI9341_YELLOW);
      tft.drawFastHLine(14 + calc_line_length(3000), 38, yellow_line_length, ILI9341_YELLOW);
      tft.drawFastHLine(14 + calc_line_length(3000), 39, yellow_line_length, ILI9341_YELLOW);
    }

    if (rpm > 4000) {
      uint32_t red_line_length = line_length - calc_line_length(4000);
      tft.drawFastHLine(14 + calc_line_length(4000), 36, red_line_length, ILI9341_RED);
      tft.drawFastHLine(14 + calc_line_length(4000), 37, red_line_length, ILI9341_RED);
      tft.drawFastHLine(14 + calc_line_length(4000), 38, red_line_length, ILI9341_RED);
      tft.drawFastHLine(14 + calc_line_length(4000), 39, red_line_length, ILI9341_RED);
    }
    
  }

  if(line_length < prev_line_length) {
    tft.drawFastHLine(14 + line_length, 36, prev_line_length - line_length, ILI9341_BLACK);
    tft.drawFastHLine(14 + line_length, 37, prev_line_length - line_length, ILI9341_BLACK);
    tft.drawFastHLine(14 + line_length, 38, prev_line_length - line_length, ILI9341_BLACK);
    tft.drawFastHLine(14 + line_length, 39, prev_line_length - line_length, ILI9341_BLACK);
  }

  prev_line_length = line_length;
}

void clearTouchBuffer() {
  while (!ts.bufferEmpty()) ts.getPoint();
}

void touch_events() {
  static uint8_t brightness = 255;
  while (! ts.bufferEmpty()) {  
    // Retrieve a point  
    TS_Point p = ts.getPoint();
   
    // Scale from ~0->4000 to tft.width using the calibration #'s
    int16_t x = map(p.y, TS_MAXY, TS_MINY, 0, tft.width());
    int16_t y = map(p.x, TS_MAXX, TS_MINY, 0, tft.height());

    if(x < 115) {
      debug = !debug;
      clear_debug();
    } else if (x < 205) {
      if(brightness > 0) brightness -= 5;
      analogWrite(3, brightness);
    } else {
      if(brightness < 255) brightness += 5;
      analogWrite(3, brightness);
    }

    //clearTouchBuffer();
  }
}

void loop() {
  static uint8_t zeroes = 0;
  auto rpm = probe() * 2;

  if(rpm == 0) {
    zeroes++;
    if (zeroes > 5) {
      render(0);
      zeroes = 0;
    }
  } else {
    render(rpm);
  }
  touch_events();
}



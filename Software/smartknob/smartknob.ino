#include <HardwareSerial.h>
#include <LovyanGFX.hpp>

#include <CST816S.h>

#include <Adafruit_NeoPixel.h>
#include "Adafruit_MCP9808.h"
#include <SparkFun_VEML7700_Arduino_Library.h>
#include "MT6701.h"
#include <SimpleFOC.h>


//DISPLAY PINS
#define TFT_MOSI 26
#define TFT_SCLK 21
#define TFT_CS   34
#define TFT_DC   33
#define TFT_RST  47

//MOTOR PINS
#define TMC_UH 42
#define TMC_VH 41
#define TMC_WH 40
#define TMC_UL 39
#define TMC_VL 37
#define TMC_WL 38

// Temperature Sensor, Light Sensor, Touch Controller
#define SDA 2
#define SCL 4

#define TOUCH_RST 13
#define TOUCH_INT 35

//MAGNETIC ENCODER
#define SDA_ME 17
#define SCL_ME 16

//RGB LEDS
#define NUMPIXELS 8
#define LEDDATA 10


HardwareSerial mySerial(1);  // Use UART1 (UART0 is often used for USB-Serial)

// // Motor instance
BLDCMotor motor = BLDCMotor(4);  // Number of pole pairs

// Driver instance
BLDCDriver6PWM driver = BLDCDriver6PWM(TMC_UH,TMC_UL,TMC_VH,TMC_VL,TMC_WH,TMC_WL);  

//RGB leds instance 
Adafruit_NeoPixel pixels(NUMPIXELS, LEDDATA, NEO_GRB + NEO_KHZ800);

// Create the MCP9808 temperature sensor instance
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

//Brightness sensor instance
VEML7700 lightsensor; 

//Magnetic Encoder instance
MT6701 encoder;

//touch controller instance
CST816S touch(SDA, SCL, TOUCH_RST, TOUCH_INT);	


// Display objects
lgfx::LGFX_Device tft;
lgfx::Panel_GC9A01 panel;
lgfx::Bus_SPI bus;
LGFX_Sprite sprite(&tft);


//Setup for Magnetic Encoder
float lastAngle = 0;
const float angleThreshold = 25.0;


//to measure last time in menu and avoid "ghost" touches 
unsigned long lastMenuExitTime = 0;  


// Menu setup
const int screenWidth = 240;
const int screenHeight = 240;
const int optionCount = 4;
const String menuOptions[optionCount] = { "Data", "Lights", "Motor", "Music" };

int selectedIndex = 0;
int targetIndex = 0;
float slideOffset = 0.0;
const float slideSpeed = 0.1;
const int optionSpacing = 120;  // More space between items

void setupDisplay() {
  // Configure SPI bus
  auto cfg = bus.config();
  cfg.spi_host = SPI2_HOST;
  cfg.spi_mode = 0;
  cfg.freq_write = 40000000;
  cfg.freq_read  = 16000000;
  cfg.spi_3wire  = true;
  cfg.use_lock   = true;
  cfg.dma_channel = SPI_DMA_CH_AUTO;
  cfg.pin_sclk = TFT_SCLK;
  cfg.pin_mosi = TFT_MOSI;
  cfg.pin_miso = -1;
  cfg.pin_dc   = TFT_DC;
  bus.config(cfg);

  panel.setBus(&bus);

  // Configure panel
  auto panel_cfg = panel.config();
  panel_cfg.pin_cs   = TFT_CS;
  panel_cfg.pin_rst  = TFT_RST;
  panel_cfg.pin_busy = -1;
  panel_cfg.memory_width  = 240;
  panel_cfg.memory_height = 240;
  panel_cfg.panel_width   = 240;
  panel_cfg.panel_height  = 240;
  panel_cfg.offset_x = 0;
  panel_cfg.offset_y = 0;
  panel_cfg.offset_rotation = 0;
  panel_cfg.dummy_read_pixel = 8;
  panel_cfg.dummy_read_bits  = 1;
  panel_cfg.readable = false;
  panel_cfg.invert    = true;
  panel_cfg.rgb_order = false;
  panel_cfg.dlen_16bit = false;
  panel_cfg.bus_shared = false;
  panel.config(panel_cfg);

  // Connect everything together
  tft.setPanel(&panel);
  tft.init();
  tft.setRotation(3); //270 degrees to the right
}

void drawArrow() {
  int centerX = screenWidth / 2;
  int tipY = screenHeight / 2 - 30;  // Text vertical center
  int arrowHeight = 12;
  int arrowWidth = 24;

  sprite.fillTriangle(
    centerX, tipY - 1,                           // Tip just above text center
    centerX - arrowWidth / 2, tipY - arrowHeight,
    centerX + arrowWidth / 2, tipY - arrowHeight,
    TFT_WHITE
  );
}

void drawMenu() {
  sprite.fillScreen(TFT_BLACK);
  int centerX = screenWidth / 2;

  for (int i = 0; i < optionCount; ++i) {
    int dx = (i - selectedIndex) * optionSpacing - slideOffset;
    int x = centerX + dx;
    int y = screenHeight / 2; //- 20;

    uint16_t color = (i == selectedIndex) ? TFT_WHITE : TFT_WHITE;
    sprite.setTextColor(color);
    sprite.setTextDatum(MC_DATUM);
    sprite.drawString(menuOptions[i], x, y, 2);
  }

  drawArrow();
  sprite.pushSprite(0, 0);
}

void updateSlide() {
  float diff = (targetIndex - selectedIndex) * optionSpacing - slideOffset;
  if (abs(diff) < 1.0) {
    selectedIndex = targetIndex;
    slideOffset = 0.0;
  } else {
    slideOffset += diff * slideSpeed;
  }
}

void showDataScreen(int screenID) {
  // 🛑 Wait for the SELECT button to be released
  motor.enable();//activate motor
  while (touch.available());

  const int screenWidth = tft.width();
  const int screenHeight = tft.height();

  // Create sprite (same size as screen)
  sprite.createSprite(screenWidth, screenHeight);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.setTextSize(2);

  //wake up tempsensor
  tempsensor.wake();   // wake up, ready to read!

  while (true) {
    sprite.fillScreen(TFT_BLACK);  // Clear sprite every frame

    switch (screenID) {
      case 0: {
        String first_title = "Temperature:";
        int first_titleWidth = sprite.textWidth(first_title);
        sprite.setCursor((screenWidth - first_titleWidth) / 2, 70);
        sprite.println(first_title);

        float c = tempsensor.readTempC();
        String temp_value = String(c);//"24.5 C";
        int temp_valueWidth = sprite.textWidth(temp_value);
        sprite.setCursor((screenWidth - temp_valueWidth) / 2, 110);
        sprite.println(temp_value);

        String second_title = "Lux:";
        int second_titleWidth = sprite.textWidth(second_title);
        sprite.setCursor((screenWidth - second_titleWidth) / 2, 150);
        sprite.println(second_title);

        float f = lightsensor.getLux();
        String flux_value = String(f);
        int flux_valueWidth = sprite.textWidth(flux_value);
        sprite.setCursor((screenWidth - flux_valueWidth) / 2, 180);
        sprite.println(flux_value);


        break;
      }
      case 1: {
        String title = "Lights:";
        int titleWidth = sprite.textWidth(title);
        sprite.setCursor((screenWidth - titleWidth) / 2, 60);
        sprite.println(title);


        //RGB LEDS visual effect
        for (int i = 0; i < NUMPIXELS; i++) {
          pixels.clear();  // Turn off all LEDs
          pixels.setPixelColor(i, pixels.Color(0, 0, 255));  // Set current LED to blue
          pixels.show();
          delay(100);
        }

        // String value = "24:05";
        // int valueWidth = sprite.textWidth(value);
        // sprite.setCursor((screenWidth - valueWidth) / 2, 110);
        // sprite.println(value);
        break;
      }
      case 2: {
        String title = "Motor:";
        int titleWidth = sprite.textWidth(title);
        sprite.setCursor((screenWidth - titleWidth) / 2, 60);
        sprite.println(title);

        String value = "All okay";
        int valueWidth = sprite.textWidth(value);
        sprite.setCursor((screenWidth - valueWidth) / 2, 110);
        sprite.println(value);

        motor.loopFOC();
        motor.move(2);  // rad/s

        break;
      }
      case 3: {
        String title = "Music:";
        int titleWidth = sprite.textWidth(title);
        sprite.setCursor((screenWidth - titleWidth) / 2, 60);
        sprite.println(title);

        String value = "Coming soon";
        int valueWidth = sprite.textWidth(value);
        sprite.setCursor((screenWidth - valueWidth) / 2, 110);
        sprite.println(value);

        //Audioamplifier code
        

        break;
      }
    }

    sprite.pushSprite(0, 0);  // Push entire sprite to screen

    
    delay(10);
 
    //Exit Menu
    if(touch.available()){
      while(touch.available());
      mySerial.println("Exiting option!");
      
      //deactivate motor's torque
      motor.move(0);     // Stop motor
      motor.disable();   // Disable driver to remove resistance
      lastAngle = encoder.angleRead();  // Synchronize last read angle

      //turn off all leds
      pixels.clear();  // Turn off all LEDs
      pixels.show();

      //reset the touch controller 
      digitalWrite(TOUCH_RST, LOW);
      delay(5);
      digitalWrite(TOUCH_RST, HIGH);  
      delay(50);

      tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere
      sprite.deleteSprite();  // Clean up sprite memory
      
      lastMenuExitTime = millis();
      delay(300);//wait for motor to really stop moving
      break;
    }
  }
  //Tempsensor sleep
  tempsensor.shutdown_wake(1); // shutdown MSP9808 - power consumption ~0.1 mikro Ampere

  sprite.deleteSprite();  // Clean up sprite memory
}



void setup() {
  //Initialize Serial Output
  mySerial.begin(115200, SERIAL_8N1, 44, 43);  // RX, TX

  //Initialize Magnetic Encoder  
  Wire.setPins(SDA_ME,SCL_ME);
  Wire.begin();
  encoder.initializeI2C();

  //Initialize Temperature and Light Sensors
  Wire1.begin(SDA,SCL);
  tempsensor.begin(0x18, &Wire1);
  tempsensor.setResolution(3);

  lightsensor.begin(Wire1);

  //Initialize RGB Leds
  pixels.begin();

  //Initialize Touch Controller
  pinMode(TOUCH_RST, OUTPUT);//first reset the controller
  digitalWrite(TOUCH_RST, LOW);
  delay(5);
  digitalWrite(TOUCH_RST, HIGH);  
  delay(50);

  touch.begin();

  //Initialize Gimbal Motor
  driver.voltage_power_supply = 12;  // Set according to your power supply (e.g., 6V)
  // limit the maximal dc voltage the driver can set
  // as a protection measure for the low-resistance motors
  // this value is fixed on startup
  driver.voltage_limit = 6;
  driver.init();

  motor.linkDriver(&driver);
  motor.controller = MotionControlType::velocity_openloop;
  motor.init();

  //Initialize Display
  setupDisplay();

  sprite.setColorDepth(8);
  sprite.createSprite(screenWidth, screenHeight);

  sprite.setFont(&fonts::Font2);
  sprite.setTextSize(2);

}



void loop() {

  updateSlide();
  drawMenu();

   if (touch.available() && millis() - lastMenuExitTime > 1000) { //avoid debounces and automatic entering in menu if last time in menu <1000s
    delay(400);
    mySerial.println("Entering option!");
    showDataScreen(targetIndex);
    delay(200);

    //recreate menu after exiting the options
    sprite.createSprite(screenWidth, screenHeight);
    sprite.setFont(&fonts::Font2);
    sprite.setTextSize(2);

  }


  float angle = encoder.angleRead();

  float delta = angle - lastAngle;

  // Handle wrap-around at 0°/360°
  if (delta > 180.0) delta -= 360.0;
  if (delta < -180.0) delta += 360.0;

  if (abs(delta) >= angleThreshold) {
    // Rotation detected
    if (delta > 0) {
      mySerial.println("Rotated counter-clockwise 25°");
      targetIndex = (targetIndex > 0) ? targetIndex - 1 : 3;
      
    } else {
      mySerial.println("Rotated clockwise 25°");
      targetIndex = (targetIndex < optionCount -1 ) ? targetIndex + 1 : 0;
    }

    lastAngle = angle;  // Update reference point
  }

  delay(16);

  

}

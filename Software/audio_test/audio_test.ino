extern "C" {
#include "driver/i2c_master.h"
}

#include "FreeRTOS.h"
#include "task.h"
#include <nau88c22.hh>
#include <AudioPlayer.hh>
#include "fanfare.h"
#include <HardwareSerial.h>
//#include <Wire.h> issues when including Wire.h conflicts with i2c_new_master_bus


#define AUDIO 1


HardwareSerial mySerial(1); 

//PINS
constexpr gpio_num_t PIN_I2C_SDA = (gpio_num_t)2;
constexpr gpio_num_t PIN_I2C_SCL = (gpio_num_t)4;

constexpr gpio_num_t PIN_I2S_MCLK = (gpio_num_t)5;
constexpr gpio_num_t PIN_I2S_FS = (gpio_num_t)9;
constexpr gpio_num_t PIN_I2S_DAC = (gpio_num_t)6;
constexpr gpio_num_t PIN_I2S_ADC = (gpio_num_t)7;
constexpr gpio_num_t PIN_I2S_BCLK = (gpio_num_t)8;


// Global variables
AudioPlayer::Player *mp3player{nullptr};
i2c_master_bus_handle_t i2c_master_handle{nullptr};




void audioTask(void *pvParameters){
  
  while(1){
    mp3player->Loop();
    }

  
 }

    void AudioLoop(void *pvParameters){
        TickType_t lastWakeTime;
        const TickType_t FREQUENCY = pdMS_TO_TICKS(20);
        while(mp3player){
            xTaskDelayUntil( &lastWakeTime, FREQUENCY);
            mp3player->Loop();
        }
    }

void setup() {

mySerial.begin(115200, SERIAL_8N1, 44, 43);


   
    // I2C configuration
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority=0,
        .trans_queue_depth=0,
        .flags={
            .enable_internal_pullup=1,
        }
    };

    // Initialize I2C
    esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &i2c_master_handle);
    if (err != ESP_OK) {
        // Handle error
    }

    //Initialize codec and player
    nau88c22::M *codec = new nau88c22::M(i2c_master_handle, PIN_I2S_MCLK, PIN_I2S_BCLK, PIN_I2S_FS, PIN_I2S_DAC);
    mp3player = new AudioPlayer::Player(codec);
    mp3player->Init();

    mp3player->PlayMP3(&fanfare_mp3[0], fanfare_mp3_len, 255, true);

    //Task??
    xTaskCreate(audioTask,"AudioTask",8192*4,NULL,1,NULL);
    
}

void loop() {
  #if 0
  
    if(mp3player){
        mp3player->Loop();
       
      }
   #endif

      
 }

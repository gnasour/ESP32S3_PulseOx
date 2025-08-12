#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "freertos/event_groups.h"
#include "esp_system.h"

#include <lwip/netdb.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

extern bool MAX_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);
extern void setup(uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, int sampleRate, int pulseWidth, int adcRange);
extern uint8_t available(void);
extern uint16_t check(void);
extern void nextSample(void);


void wifi_init(void);
void server_init(void);

const char *TAG_MAIN = "MAIN: ";



void run()
{

  i2c_master_bus_handle_t bus_handle;
  i2c_master_dev_handle_t dev_handle;
  MAX_init(&bus_handle, &dev_handle);
  ESP_LOGI(TAG_MAIN, "I2C initialized successfully");

  // Pulse Ox setup initial values
  uint8_t ledBrightness = 80; // Options: 0=Off to 255=50mA
  uint8_t sampleAverage = 4; // Options: 1, 2, 4, 8, 16, 32
  uint8_t ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 3200;      // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;       // Options: 69, 118, 215, 411
  int adcRange = 16384;       // Options: 2048, 4096, 8192, 16384

  setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  while (true)
  {

    while (available() == 0) // do we have new data?
      check();               // Check the sensor for new data

    nextSample(); // We're finished with this sample so move to next sample
  }
}

void app_main(void)
{

  wifi_init();
  server_init();
  
  TaskHandle_t xHandle;
  xTaskCreate(run, "run", 4096, NULL, 5, &xHandle);
}
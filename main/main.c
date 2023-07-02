#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_idf_version.h>
#include <max7219.h>
#include <stdbool.h>

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 0, 0)
#define HOST    HSPI_HOST
#else
#define HOST    SPI2_HOST
#endif

#define CASCADE_SIZE 1
#define MOSI_PIN 23
#define CS_PIN 5
#define CLK_PIN 18
# define NO_DISPLAY 0x00000000000000000
# define DIRECTIONS 3

// Define arrow symbols
static const uint64_t symbols[] = {
    // Orientation: main pins to the right
    0x383838fe7c381000, // arrow up
    //0x10387cfe38383800, // arrow down
    0x10307efe7e301000, // arrow right
    0x1018fcfefc181000, // arrow left
    0x8142241818244281, // X symbol (No space available)
    
};
//static const size_t symbols_size = sizeof(symbols) / sizeof(symbols[0]);

void task(void *pvParameter)
{
    spi_bus_config_t cfg = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HOST, &cfg, 1));

    max7219_t dev = {
        .cascade_size = CASCADE_SIZE,
        .digits = 0,
        .mirrored = true
    };
    ESP_ERROR_CHECK(max7219_init_desc(&dev, HOST, MAX7219_MAX_CLOCK_SPEED_HZ, CS_PIN));
    ESP_ERROR_CHECK(max7219_init(&dev));

    size_t arrow_index = 0;

    while (1)
    {   
        uint64_t available_zones[3] = {NO_DISPLAY, NO_DISPLAY, NO_DISPLAY};


        // Read garage_status.txt file and get information
        int zone1_count = 32;
        int zone2_count = 33;
        int zone3_count = 3;
        int zone1_capacity = 20;
        int zone2_capacity = 20;
        int zone3_capacity = 20;
        
        // Check if zone 1 is full
        bool zone1_available = zone1_count < zone1_capacity;
        if(zone1_available){
            available_zones[0] = symbols[1];
        }

        // Check if zone 2 is full
        bool zone2_available = zone2_count < zone2_capacity;
        if(zone2_available){
            available_zones[1] = symbols[2];
        }

        // Check if zone 3 is full
        bool zone3_available = zone3_count < zone3_capacity;
        if(zone3_available){
            available_zones[2] = symbols[0];
        }

        bool garage_full = (!zone1_available) && (!zone2_available) && (!zone3_available);
        if(garage_full){
            available_zones[0] = symbols[3];
            available_zones[1] = symbols[3];
            available_zones[2] = symbols[3];
        }

        
        max7219_draw_image_8x8(&dev, 0, (uint8_t *)&available_zones[arrow_index]);
        vTaskDelay(pdMS_TO_TICKS(820));

        // Move to the next arrow symbol
        if (++arrow_index == DIRECTIONS){
            arrow_index = 0;
        }
    }
}

void app_main()
{
    xTaskCreatePinnedToCore(task, "task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL, APP_CPU_NUM);
}

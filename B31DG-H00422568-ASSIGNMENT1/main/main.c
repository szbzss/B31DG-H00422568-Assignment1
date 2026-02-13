#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

#define PIN_DATA      18
#define PIN_SYNC      19
#define PIN_PB_ENABLE  25
#define PIN_PB_SELECT  26


#define PARAM_A       800
#define PARAM_B       600
#define PARAM_C       17
#define PARAM_D       6500
#define T_SYNC_ON     50

static const char *TAG = "B31DG_Sun";

static volatile bool output_enabled = false;
static volatile bool alternative_mode = false;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    static uint32_t last_time_pb1 = 0;
    static uint32_t last_time_pb2 = 0;
    uint32_t now = xTaskGetTickCountFromISR();

    if (gpio_num == PIN_PB_ENABLE) {
        if (now - last_time_pb1 > pdMS_TO_TICKS(200)) { 
            output_enabled = !output_enabled;
            last_time_pb1 = now;
        }
    } else if (gpio_num == PIN_PB_SELECT) {
        if (now - last_time_pb2 > pdMS_TO_TICKS(200)) {
            alternative_mode = !alternative_mode;
            last_time_pb2 = now;
        }
    }
}

void signal_generator_task(void *pvParameter) {
    ESP_LOGI(TAG, "Signal Generator Task Started");

    while (1) {
        if (!output_enabled) {
            gpio_set_level(PIN_DATA, 0);
            gpio_set_level(PIN_SYNC, 0);
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        gpio_set_level(PIN_SYNC, 1);
        ets_delay_us(T_SYNC_ON);
        gpio_set_level(PIN_SYNC, 0);

        for (int i = 1; i <= PARAM_C; i++) {
            int n = alternative_mode ? (PARAM_C - i + 1) : i;
            uint32_t t_on = PARAM_A + ((n - 1) * 50);

            gpio_set_level(PIN_DATA, 1);
            ets_delay_us(t_on);
            gpio_set_level(PIN_DATA, 0);

            if (i < PARAM_C) {
                ets_delay_us(PARAM_B);
            }
        }

        ets_delay_us(PARAM_D);
    }
}

void app_main(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_DATA) | (1ULL << PIN_SYNC),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << PIN_PB_ENABLE) | (1ULL << PIN_PB_SELECT);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 1;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_PB_ENABLE, gpio_isr_handler, (void*) PIN_PB_ENABLE);
    gpio_isr_handler_add(PIN_PB_SELECT, gpio_isr_handler, (void*) PIN_PB_SELECT);

    xTaskCreatePinnedToCore(signal_generator_task, "sig_gen_task", 2048, NULL, 10, NULL, 1);
}
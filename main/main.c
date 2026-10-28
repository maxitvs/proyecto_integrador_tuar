// Máquina de estados principal y coordinación
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_INTEGRADO GPIO_NUM_2

static const char *TAG = "PRUEBA_CONEXION";

void app_main(void)
{
    // Configurar el pin del LED integrado como salida
    gpio_reset_pin(LED_INTEGRADO);
    gpio_set_direction(LED_INTEGRADO, GPIO_MODE_OUTPUT);

    int contador = 0;
    int estado_led = 0;

    ESP_LOGI(TAG, "¡Conexion exitosa con VS Code! Iniciando prueba...");

    while (1) {
        estado_led = !estado_led; // Invierte entre 0 y 1
        gpio_set_level(LED_INTEGRADO, estado_led);

        ESP_LOGI(TAG, "Latido #%d | LED en estado: %s", contador++, estado_led ? "ENCENDIDO" : "APAGADO");

        // Espera 1 segundo (1000 ms)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
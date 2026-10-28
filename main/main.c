// Máquina de estados principal y coordinación
// Máquina de estados principal y coordinación
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500)); // PD: evita reinicio del esp32 por pensar que está en un bucle infinito (tiempo: convierte 500 milisegundos en ticks que son pulsos de su reloj interno)
    }
}
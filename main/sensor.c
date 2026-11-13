// Disparo de pulso ultrasónico y cálculo de distancia en cm
#include "sensor.h"
#include "setup.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

static const char *TAG = "SENSOR";
#define TIMEOUT_ECO_US 30000 // Tope de espera de 30.000 us (30 ms)

void sensor_inicializar(void) {
    ESP_LOGI(TAG, "Inicializando pines del sensor ultrasonico (4 pines)...");
    gpio_reset_pin(PIN_SENSOR_TRIG);
    gpio_reset_pin(PIN_SENSOR_ECHO);
    gpio_set_direction(PIN_SENSOR_TRIG, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_SENSOR_ECHO, GPIO_MODE_INPUT);
    gpio_set_level(PIN_SENSOR_TRIG, 0);
}

estado_sensor_t sensor_leer_distancia(datos_sensor_t *datos) {
    if (datos == NULL) return SENSOR_ERROR_LECTURA;

    // 1. Disparo: pulso de 10 us en el pin TRIG
    gpio_set_level(PIN_SENSOR_TRIG, 0);
    ets_delay_us(2);
    gpio_set_level(PIN_SENSOR_TRIG, 1);
    ets_delay_us(10);
    gpio_set_level(PIN_SENSOR_TRIG, 0);

    // 2. Espera de detección (subida del pin ECHO a 1)
    int64_t t_inicio_espera = esp_timer_get_time();
    while (gpio_get_level(PIN_SENSOR_ECHO) == 0) {
        if ((esp_timer_get_time() - t_inicio_espera) > TIMEOUT_ECO_US) {
            ESP_LOGE(TAG, "Error: Sensor desconectado o sin respuesta en ECHO");
            datos->distancia_cm = -1.0f;
            datos->estado = SENSOR_ERROR_LECTURA;
            return SENSOR_ERROR_LECTURA;
        }
    }

    // 3. Medición de la duración del viaje del sonido (mientras ECHO vale 1)
    int64_t t_inicio_pulso = esp_timer_get_time();
    while (gpio_get_level(PIN_SENSOR_ECHO) == 1) {
        if ((esp_timer_get_time() - t_inicio_pulso) > TIMEOUT_ECO_US) {
            ESP_LOGI(TAG, "Camino despejado (sin objetos en rango)");
            datos->distancia_cm = 999.0f;
            datos->estado = SENSOR_SIN_OBJETO;
            return SENSOR_SIN_OBJETO;
        }
    }
    int64_t t_fin_pulso = esp_timer_get_time();

    // 4. Cálculo de distancia en centímetros
    int64_t duracion_us = t_fin_pulso - t_inicio_pulso;
    datos->distancia_cm = (duracion_us * 0.0343f) / 2.0f;
    datos->estado = SENSOR_OBJETO_DETECTADO;

    return SENSOR_OBJETO_DETECTADO;
}

// Inicialización de GPIOs y lógica de los LEDs
#include "setup.h"
#include "sensor.h"
#include "esp_log.h"

static const char *TAG = "SETUP";

void setup_inicializar_hardware(void) {
    ESP_LOGI(TAG, "Configurando pines de LEDs y finales de carrera...");

    // 1. Limpiamos y configuramos pines de LEDs como SALIDA
    gpio_reset_pin(PIN_LED_VERDE);
    gpio_reset_pin(PIN_LED_AMARILLO);
    gpio_reset_pin(PIN_LED_ROJO);

    gpio_set_direction(PIN_LED_VERDE, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_AMARILLO, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_ROJO, GPIO_MODE_OUTPUT);

    // Estado inicial seguro: todos los LEDs apagados (0V)
    gpio_set_level(PIN_LED_VERDE, 0);
    gpio_set_level(PIN_LED_AMARILLO, 0);
    gpio_set_level(PIN_LED_ROJO, 0);

    // 2. Finales de carrera: entradas con pull-up interno (switch cerrado a GND = 0)
    gpio_reset_pin(PIN_FIN_CARRERA_HORARIO);
    gpio_reset_pin(PIN_FIN_CARRERA_ANTIHORARIO);
    gpio_set_direction(PIN_FIN_CARRERA_HORARIO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_FIN_CARRERA_ANTIHORARIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_FIN_CARRERA_HORARIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_FIN_CARRERA_ANTIHORARIO, GPIO_PULLUP_ONLY);
}

// --- ENCENDIDO DE LEDS SEGÚN MEDICIÓN Y ESTADO DEL SENSOR ---
void setup_actualizar_leds(float distancia_cm, int estado_lectura) {
    // Corrección 6: Si el sensor falló (desconectado), encendemos LED ROJO por seguridad (Fail-Safe)
    if (estado_lectura == SENSOR_ERROR_LECTURA) {
        gpio_set_level(PIN_LED_VERDE, 0);
        gpio_set_level(PIN_LED_AMARILLO, 0);
        gpio_set_level(PIN_LED_ROJO, 1);
        return;
    }

    // Si el camino está despejado (fuera de rango) o el objeto está lejos (>= 40 cm) -> LED Verde
    if (estado_lectura == SENSOR_SIN_OBJETO || distancia_cm >= UMBRAL_MEDIO_CM) {
        gpio_set_level(PIN_LED_VERDE, 1);
        gpio_set_level(PIN_LED_AMARILLO, 0);
        gpio_set_level(PIN_LED_ROJO, 0);
    } 
    // Objeto muy cerca (< 15 cm) -> LED Rojo
    else if (distancia_cm < UMBRAL_CERCA_CM) {
        gpio_set_level(PIN_LED_VERDE, 0);
        gpio_set_level(PIN_LED_AMARILLO, 0);
        gpio_set_level(PIN_LED_ROJO, 1);
    } 
    // Objeto a distancia media (entre 15 y 40 cm) -> LED Amarillo
    else {
        gpio_set_level(PIN_LED_VERDE, 0);
        gpio_set_level(PIN_LED_AMARILLO, 1);
        gpio_set_level(PIN_LED_ROJO, 0);
    }
}

// Indicador visual de bloqueo por seguridad (Amarillo y Rojo encendidos fijos)
void setup_indicar_error_hardware(void) {
    gpio_set_level(PIN_LED_VERDE, 0);
    gpio_set_level(PIN_LED_AMARILLO, 1);
    gpio_set_level(PIN_LED_ROJO, 1);
}

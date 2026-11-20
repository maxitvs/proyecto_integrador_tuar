// Configuración de los switches y lógica de límite del péndulo
#include "final_carrera.h"
#include "setup.h"
#include "esp_log.h"

static const char *TAG = "FIN_CARRERA";

// ---INICIALIZAMOS LOS FINALES DE CARRERA---
// PD: configuramos los dos switches como entradas con pull-up interno. Al cerrar contra GND el pin pasa a 0.
void final_carrera_inicializar(void){
    ESP_LOGI(TAG, "Configurando finales de carrera...");

    gpio_reset_pin(PIN_FIN_CARRERA_HORARIO);
    gpio_reset_pin(PIN_FIN_CARRERA_ANTIHORARIO);
    gpio_set_direction(PIN_FIN_CARRERA_HORARIO, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_FIN_CARRERA_ANTIHORARIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_FIN_CARRERA_HORARIO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_FIN_CARRERA_ANTIHORARIO, GPIO_PULLUP_ONLY);
}

// ---LECTURA DE CADA TOPE---
// PD: el switch está activo en bajo, por eso comparamos contra 0.
bool final_carrera_horario_activado(void){
    return gpio_get_level(PIN_FIN_CARRERA_HORARIO) == 0;
}

bool final_carrera_antihorario_activado(void){
    return gpio_get_level(PIN_FIN_CARRERA_ANTIHORARIO) == 0;
}

// ---INVERSIÓN DE SENTIDO AL LLEGAR A UN TOPE---
// PD: recibe el sentido actual, revisa el tope que corresponde y devuelve el sentido invertido si llegó.
sentido_giro_t final_carrera_actualizar_sentido(sentido_giro_t sentido_actual){
    if (sentido_actual == GIRO_HORARIO && final_carrera_horario_activado()) {
        ESP_LOGI(TAG, "Tope horario alcanzado -> invirtiendo a antihorario");
        return GIRO_ANTIHORARIO;
    }
    if (sentido_actual == GIRO_ANTIHORARIO && final_carrera_antihorario_activado()) {
        ESP_LOGI(TAG, "Tope antihorario alcanzado -> invirtiendo a horario");
        return GIRO_HORARIO;
    }
    return sentido_actual;
}

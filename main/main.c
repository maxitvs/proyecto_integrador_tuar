// Máquina de estados principal y coordinación
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "setup.h"
#include "motor.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    control_motor_t motor;
    sentido_giro_t sentido = GIRO_HORARIO;

    setup_inicializar_hardware();
    motor_inicializar(&motor);

    while (1) {
        // PD: En qué sentido está girando el motor.
        if (sentido == GIRO_HORARIO) {
            if (gpio_get_level(PIN_FIN_CARRERA_HORARIO) == 0) {
                // El switch cerró a GND (nivel 0) -> el péndulo llegó al tope.
                sentido = GIRO_ANTIHORARIO; // Invertimos el sentido de giro.
                ESP_LOGI(TAG, "Tope horario alcanzado -> invirtiendo a antihorario");
            }
        }
        else {
            // si el motor NO estaba en sentido horario (está en antihorario).
            // Revisamos el final de carrera antihorario.
            if (gpio_get_level(PIN_FIN_CARRERA_ANTIHORARIO) == 0) {
                // El switch cerró a GND (nivel 0) -> el péndulo llegó al otro tope.
                sentido = GIRO_HORARIO; // Volvemos al sentido horario.
                ESP_LOGI(TAG, "Tope antihorario alcanzado -> invirtiendo a horario");
            }
        }

        motor_avanzar_paso(&motor, sentido);
        vTaskDelay(pdMS_TO_TICKS(500)); // PD: pausa entre pasos (500 ms en ticks)
    }
}
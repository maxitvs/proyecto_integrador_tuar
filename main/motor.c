// Control físico de los transistores del Puente H y lectura de finales de carrera
#include "motor.h"
#include "setup.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MOTOR";

// --- FUNCIÓN PRIVADA ANTIREBOTE (DEBOUNCE) PARA FINALES DE CARRERA ---
static bool leer_fin_carrera_estable(gpio_num_t pin) {
    if (gpio_get_level(pin) == 0) { // Detecta presión inicial (activo en bajo)
        vTaskDelay(pdMS_TO_TICKS(TIEMPO_ANTIREBOTE_MS)); // Espera 20 ms a que pasen los rebotes mecánicos
        if (gpio_get_level(pin) == 0) { // Confirma que sigue presionado
            return true;
        }
    }
    return false;
}

// --- INICIALIZAMOS EL MOTOR ---
void motor_inicializar(control_motor_t *motor) {
    ESP_LOGI(TAG, "Inicializando pines del Puente H...");

    gpio_reset_pin(PIN_MOTOR_IN1);
    gpio_reset_pin(PIN_MOTOR_IN2);
    gpio_set_direction(PIN_MOTOR_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_MOTOR_IN2, GPIO_MODE_OUTPUT);

    // En nuestro Puente H discreto (con drivers BS170), poner IN1=0 e IN2=0 
    // lleva los 4 MOSFETs de potencia al estado de CORTE (alta impedancia),
    // desenergizando la bobina mientras los diodos 1N4007 disipan la corriente inductiva.
    gpio_set_level(PIN_MOTOR_IN1, 0);
    gpio_set_level(PIN_MOTOR_IN2, 0);

    if (motor != NULL) {
        motor->sentido = GIRO_DETENIDO;
        motor->paso_actual = 0;
        motor->tiempo_recorrido_ms = 0;
    }
}

// --- DETENCIÓN DEL MOTOR ---
void motor_detener(control_motor_t *motor) {
    // Cortamos la excitación de ambas diagonales del Puente H
    gpio_set_level(PIN_MOTOR_IN1, 0);
    gpio_set_level(PIN_MOTOR_IN2, 0);
    
    if (motor != NULL) {
        motor->sentido = GIRO_DETENIDO;
    }
}

// --- EJECUCIÓN DE UN PASO VIGILANDO FINALES DE CARRERA ---
// Corrección 2: Avanza durante TIEMPO_PASO_MS (150 ms) pero comprobando cada 10 ms
// si se activó el final de carrera correspondiente para frenar en el acto.
// Devuelve true si tocó el tope físico durante el movimiento.
bool motor_ejecutar_paso(control_motor_t *motor, sentido_giro_t direccion) {
    if (motor == NULL) return false;

    // 1. Verificación previa antes de energizar el puente H
    if (direccion == GIRO_HORARIO && motor_llego_fin()) {
        motor_detener(motor);
        return true;
    }
    if (direccion == GIRO_ANTIHORARIO && motor_llego_inicio()) {
        motor_detener(motor);
        return true;
    }

    // 2. Energizamos la diagonal correspondiente del Puente H
    motor->sentido = direccion;
    if (direccion == GIRO_HORARIO) {
        gpio_set_level(PIN_MOTOR_IN1, 1);
        gpio_set_level(PIN_MOTOR_IN2, 0);
    } else if (direccion == GIRO_ANTIHORARIO) {
        gpio_set_level(PIN_MOTOR_IN1, 0);
        gpio_set_level(PIN_MOTOR_IN2, 1);
    } else {
        motor_detener(motor);
        return false;
    }

    // 3. Muestreo activo cada 10 ms mientras el motor gira
    int tiempo_transcurrido = 0;
    bool tope_alcanzado = false;

    while (tiempo_transcurrido < TIEMPO_PASO_MS) {
        vTaskDelay(pdMS_TO_TICKS(INTERVALO_MUESTREO_FC_MS));
        tiempo_transcurrido += INTERVALO_MUESTREO_FC_MS;
        motor->tiempo_recorrido_ms += INTERVALO_MUESTREO_FC_MS;

        // Comprobación rápida de nivel bajo (0V) en el pin del final de carrera
        if (direccion == GIRO_HORARIO && (gpio_get_level(PIN_FIN_CARRERA_HORARIO) == 0)) {
            if (motor_llego_fin()) { // Confirmamos con antirebote
                tope_alcanzado = true;
                break;
            }
        } else if (direccion == GIRO_ANTIHORARIO && (gpio_get_level(PIN_FIN_CARRERA_ANTIHORARIO) == 0)) {
            if (motor_llego_inicio()) { // Confirmamos con antirebote
                tope_alcanzado = true;
                break;
            }
        }
    }

    // 4. Desactivamos el Puente H al finalizar el paso o al chocar con el tope
    motor_detener(motor);
    motor->paso_actual++;

    ESP_LOGI(TAG, "Paso #%d completado (%s) | Tiempo acum.: %d ms", 
             motor->paso_actual,
             (direccion == GIRO_HORARIO) ? "HORARIO" : "ANTIHORARIO",
             motor->tiempo_recorrido_ms);

    return tope_alcanzado;
}

// --- LECTURA DE FINALES DE CARRERA CON ANTIREBOTE ---
bool motor_llego_inicio(void) {
    // Devuelve true si el final de carrera de 0° (Inicio) está presionado de forma estable
    return leer_fin_carrera_estable(PIN_FIN_CARRERA_ANTIHORARIO);
}

bool motor_llego_fin(void) {
    // Devuelve true si el final de carrera de 180° (Fin) está presionado de forma estable
    return leer_fin_carrera_estable(PIN_FIN_CARRERA_HORARIO);
}

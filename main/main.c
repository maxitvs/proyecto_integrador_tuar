// Máquina de estados principal y coordinación general del Radar
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "setup.h"
#include "motor.h"
#include "sensor.h"

static const char *TAG = "MAIN";

void app_main(void) {
    estado_sistema_t estado_actual = ESTADO_INICIALIZACION;
    sentido_giro_t direccion_barrido = GIRO_HORARIO;

    control_motor_t mi_motor;
    datos_sensor_t mi_sensor;

    while (1) {
        switch (estado_actual) {

            case ESTADO_INICIALIZACION:
                ESP_LOGI(TAG, "=== INICIANDO SISTEMA DE RADAR ===");
                setup_inicializar_hardware();
                motor_inicializar(&mi_motor);
                sensor_inicializar();

                // Corrección 3: Verificación de posición inicial (0° activado y 180° libre)
                ESP_LOGI(TAG, "Verificando posicion inicial (0°)...");
                while (!motor_llego_inicio() || motor_llego_fin()) {
                    ESP_LOGW(TAG, "Posicione manualmente el radar en el tope de 0° (Inicio) para comenzar...");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }

                ESP_LOGI(TAG, "Posicion inicial 0° confirmada. Iniciando barrido...");
                direccion_barrido = GIRO_HORARIO;
                mi_motor.paso_actual = 0;
                mi_motor.tiempo_recorrido_ms = 0;
                estado_actual = ESTADO_LECTURA;
                break;

            case ESTADO_LECTURA:
                // 1. Medimos distancia con el motor detenido y actualizamos LEDs según las 3 situaciones
                sensor_leer_distancia(&mi_sensor);
                setup_actualizar_leds(mi_sensor.distancia_cm, mi_sensor.estado);

                if (mi_sensor.estado == SENSOR_OBJETO_DETECTADO) {
                    ESP_LOGI(TAG, "Reporte [Paso %d] -> Distancia: %.2f cm | Estado: OBJETO DETECTADO",
                             mi_motor.paso_actual, mi_sensor.distancia_cm);
                } else if (mi_sensor.estado == SENSOR_SIN_OBJETO) {
                    ESP_LOGI(TAG, "Reporte [Paso %d] -> Estado: ZONA DESPEJADA (Fuera de rango)",
                             mi_motor.paso_actual);
                } else {
                    ESP_LOGE(TAG, "Reporte [Paso %d] -> Estado: ERROR DE LECTURA DEL SENSOR",
                             mi_motor.paso_actual);
                }

                vTaskDelay(pdMS_TO_TICKS(TIEMPO_LECTURA_MS));

                // 2. Corrección 4: Cambio de sentido gobernado exclusivamente por los finales de carrera físicos
                if (direccion_barrido == GIRO_HORARIO) {
                    if (motor_llego_fin()) {
                        ESP_LOGI(TAG, "Tope fisico HORARIO (180°) alcanzado. Invirtiendo sentido...");
                        direccion_barrido = GIRO_ANTIHORARIO;
                        mi_motor.paso_actual = 0;
                        mi_motor.tiempo_recorrido_ms = 0; // Reiniciamos el cronómetro de protección
                        estado_actual = ESTADO_BARRIDO_ANTIHORARIO;
                    } else {
                        estado_actual = ESTADO_BARRIDO_HORARIO;
                    }
                } else {
                    if (motor_llego_inicio()) {
                        ESP_LOGI(TAG, "Tope fisico ANTIHORARIO (0°) alcanzado. Invirtiendo sentido...");
                        direccion_barrido = GIRO_HORARIO;
                        mi_motor.paso_actual = 0;
                        mi_motor.tiempo_recorrido_ms = 0; // Reiniciamos el cronómetro de protección
                        estado_actual = ESTADO_BARRIDO_HORARIO;
                    } else {
                        estado_actual = ESTADO_BARRIDO_ANTIHORARIO;
                    }
                }
                break;

            case ESTADO_BARRIDO_HORARIO:
                // Ejecuta el paso vigilando el final de carrera cada 10 ms
                motor_ejecutar_paso(&mi_motor, GIRO_HORARIO);

                // Corrección 5: Protección por tiempo máximo de recorrido
                if (mi_motor.tiempo_recorrido_ms >= TIEMPO_MAX_RECORRIDO_MS && !motor_llego_fin()) {
                    ESP_LOGE(TAG, "PROTECCION ACTIVADA: Tiempo maximo excedido (%d ms) sin detectar fin de carrera de 180°",
                             mi_motor.tiempo_recorrido_ms);
                    estado_actual = ESTADO_ERROR;
                } else {
                    estado_actual = ESTADO_LECTURA;
                }
                break;

            case ESTADO_BARRIDO_ANTIHORARIO:
                // Ejecuta el paso vigilando el final de carrera cada 10 ms
                motor_ejecutar_paso(&mi_motor, GIRO_ANTIHORARIO);

                // Corrección 5: Protección por tiempo máximo de recorrido
                if (mi_motor.tiempo_recorrido_ms >= TIEMPO_MAX_RECORRIDO_MS && !motor_llego_inicio()) {
                    ESP_LOGE(TAG, "PROTECCION ACTIVADA: Tiempo maximo excedido (%d ms) sin detectar fin de carrera de 0°",
                             mi_motor.tiempo_recorrido_ms);
                    estado_actual = ESTADO_ERROR;
                } else {
                    estado_actual = ESTADO_LECTURA;
                }
                break;

            case ESTADO_ERROR:
                // Desactiva inmediatamente el motor y deja señal lumínica de falla
                motor_detener(&mi_motor);
                setup_indicar_error_hardware();
                ESP_LOGE(TAG, "SISTEMA DETENIDO POR SEGURIDAD. Verifique finales de carrera o traba mecanica y reinicie el ESP32.");
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;

            default:
                estado_actual = ESTADO_INICIALIZACION;
                break;
        }
    }
}

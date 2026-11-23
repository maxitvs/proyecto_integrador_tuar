// Radar con ESP32 - el motor se mueve por pasos
// Idea: el motor gira unos ms. Si toca un final de carrera, gira al otro lado.
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "setup.h"
#include "motor.h"
#include "sensor.h"

static const char *TAG = "MAIN";

// ---TIEMPOS (se pueden ajustar, todos en ms)---
#define TIEMPO_PASO_MS 230            // cuánto gira el motor en cada paso
#define TIEMPO_LECTURA_MS 600         // espera con el motor detenido antes de medir
#define TIEMPO_PARADA_EXTREMO_MS 600  // espera al tocar un final de carrera, antes de girar al otro lado
#define TIEMPO_ESPERA_OBJETO_MS 200   // cada cuánto se mide de nuevo mientras hay un objeto cerca

// Muestra por el monitor serial lo que midió el sensor.
// Recibe un PUNTERO (*) a la estructura: no copia los datos, solo recibe la dirección de memoria donde están.
// Con la flecha (->) se accede a cada dato a través del puntero.
static void mostrar_reporte(const datos_sensor_t *sensor){
    ESP_LOGI(TAG, "Reporte -> Distancia: %.2f cm | Objeto: %s",
        sensor->distancia_cm,
        sensor->objeto_detectado ? "SI" : "NO");
}

void app_main(void){
    estado_sistema_t estado_actual = ESTADO_INICIALIZACION;
    sentido_giro_t direccion = GIRO_HORARIO;   // hacia dónde está girando el péndulo
    bool toco_final = false;                   // true si el paso terminó en un final de carrera
    int tiempo_ms = 0;                         // cuánto lleva girando el paso actual

    // Estructuras donde se guardan el estado del motor y la medición del sensor.
    // A las funciones se las pasamos con & (dirección de memoria) y ellas reciben un puntero para poder modificarlas.
    control_motor_t mi_motor;
    datos_sensor_t mi_sensor;

    while(1){
        switch(estado_actual){

            case ESTADO_INICIALIZACION:
                // Configura todo y empieza girando en sentido horario
                ESP_LOGI(TAG, "=== INICIANDO SISTEMA DE RADAR");
                setup_inicializar_hardware();
                motor_inicializar(&mi_motor);   // &mi_motor = puntero a la estructura del motor
                sensor_inicializar();

                direccion = GIRO_HORARIO;
                estado_actual = ESTADO_BARRIDO_HORARIO;
                break;

            case ESTADO_BARRIDO_HORARIO:
                // Gira en sentido horario hasta que pase el tiempo del paso o toque el final de carrera
                motor_avanzar_paso(&mi_motor, GIRO_HORARIO);
                tiempo_ms = 0;
                toco_final = false;

                while(tiempo_ms < TIEMPO_PASO_MS && !toco_final){
                    vTaskDelay(pdMS_TO_TICKS(10));
                    tiempo_ms += 10;
                    toco_final = motor_llego_fin();
                }

                motor_detener(&mi_motor);
                estado_actual = ESTADO_LECTURA;
                break;

            case ESTADO_BARRIDO_ANTIHORARIO:
                // Gira en sentido antihorario hasta que pase el tiempo del paso o toque el final de carrera
                motor_avanzar_paso(&mi_motor, GIRO_ANTIHORARIO);
                tiempo_ms = 0;
                toco_final = false;

                while(tiempo_ms < TIEMPO_PASO_MS && !toco_final){
                    vTaskDelay(pdMS_TO_TICKS(10));
                    tiempo_ms += 10;
                    toco_final = motor_llego_inicio();
                }

                motor_detener(&mi_motor);
                estado_actual = ESTADO_LECTURA;
                break;

            case ESTADO_LECTURA:
                // 1. Con el motor detenido, esperamos que el péndulo se estabilice y medimos
                vTaskDelay(pdMS_TO_TICKS(TIEMPO_LECTURA_MS));
                sensor_leer_distancia(&mi_sensor);   // el sensor guarda el resultado en mi_sensor a través del puntero
                setup_actualizar_leds(mi_sensor.distancia_cm, mi_sensor.objeto_detectado);
                mostrar_reporte(&mi_sensor);

                // 2. Mientras haya un objeto a menos de 40 cm, el motor sigue detenido y volvemos a medir
                while(mi_sensor.objeto_detectado && mi_sensor.distancia_cm < UMBRAL_MEDIO_CM){
                    ESP_LOGI(TAG, "Objeto a %.2f cm -> barrido en pausa", mi_sensor.distancia_cm);
                    vTaskDelay(pdMS_TO_TICKS(TIEMPO_ESPERA_OBJETO_MS));
                    sensor_leer_distancia(&mi_sensor);
                    setup_actualizar_leds(mi_sensor.distancia_cm, mi_sensor.objeto_detectado);
                }

                // 3. Si tocó un final de carrera, espera y cambia de sentido
                if(toco_final){
                    if(direccion == GIRO_HORARIO){
                        ESP_LOGI(TAG, "Final de carrera HORARIO alcanzado. Invirtiendo sentido...");
                        vTaskDelay(pdMS_TO_TICKS(TIEMPO_PARADA_EXTREMO_MS));
                        direccion = GIRO_ANTIHORARIO;
                    } else{
                        ESP_LOGI(TAG, "Final de carrera ANTIHORARIO alcanzado. Invirtiendo sentido...");
                        vTaskDelay(pdMS_TO_TICKS(TIEMPO_PARADA_EXTREMO_MS));
                        direccion = GIRO_HORARIO;
                    }
                }

                // 4. Siguiente paso en el sentido que corresponda
                if(direccion == GIRO_HORARIO){
                    estado_actual = ESTADO_BARRIDO_HORARIO;
                } else{
                    estado_actual = ESTADO_BARRIDO_ANTIHORARIO;
                }
                break;

            default:
                // Si el estado toma un valor inválido, frena el motor y reinicia
                motor_detener(&mi_motor);
                estado_actual = ESTADO_INICIALIZACION;
                break;
        }
    }
}

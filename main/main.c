// Máquina de estados principal y coordinación
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "setup.h"
#include "motor.h"
#include "sensor.h"

static const char *TAG = "MAIN";

//Tiempo que se engergiza el motor en cada paso (ajustable según velocidad de motor)
#define TIEMPO_PASO_MS 150
// Pausa con el motor detenido para estabilizar y medir
#define TIEMPO_LECTURA_MS 350

void app_main(void){
    // Variables de estado y estructuras del sistema
    estado_sistema_t estado_actual = ESTADO_INICIALIZACION; //PD: cpm estado_actual en inicialización lo primero que va a hacer es configurar los pines
    sentido_giro_t direccion_barrido = GIRO_HORARIO;//PD: la dirección para recordar en que sentido estaba girando antes de frenar

    //PD: son las 2 estructuras en donde se guardan el ángulo actual y la distancia en la RAM
    control_motor_t mi_motor;
    datos_sensor_t mi_sensor;

    while(1){ //PD: es el bucle infitio que nunca debe terminar para este sistema embebido. En cada vuelta el switch mira que valor tiene estado actual y entra en el case correspondiente
        switch(estado_actual){
            case ESTADO_INICIALIZACION://PD: configura todo, fija el sentido inicial a horario y cambia a estado lectura para que en la próxima vuelta vaya a medir directo en 0°
                ESP_LOGI(TAG, "=== INICIANDO SISTEMA DE RADAR");
                setup_inicializar_hardware();
                motor_inicializar(&mi_motor);
                sensor_inicializar();

                direccion_barrido = GIRO_HORARIO;
                estado_actual = ESTADO_LECTURA;
                break;

            case ESTADO_LECTURA://PD: PRIMERA PARTE: medir y mostrar: motor quieto le pasamos el mi sensor a leer distancia para que mida y guarde el resultado ahí. Le paso esto actualizar leds, imprimimos en consola
                // 1. Medimos distancia con el motor detenido y actualizamos LEDs
                sensor_leer_distancia(&mi_sensor);
                setup_actualizar_leds(mi_sensor.distancia_cm, mi_sensor.objeto_detectado);

                ESP_LOGI(TAG, "Reporte -> Angulo: %d° | Distancia: %.2f cm | Objeto: %s",
                    mi_motor.angulo_actual, 
                    mi_sensor.distancia_cm, 
                    mi_sensor.objeto_detectado ? "SI" : "NO");

                vTaskDelay(pdMS_TO_TICKS(TIEMPO_LECTURA_MS));//PD:espera 350 ms.
                
                //PD: SEGUNDA PARTE: decisión de movimiento: si venía en sentido HORARIO pregunta si chocó con un fin de carrera o si la cuenta matemática llegó a 180°. Si no avanza; si si recalibra el ángulo en 180° e invierte a antihorario. Si venía en sentido ANTIHORARIO hace exactamente lo opuesto preguntando si chocó un fin o si el ángulo dió 0°
                // 2. Evaluamos finales de carrera y límites angulares para decidir el próximo movimiento
                if(direccion_barrido == GIRO_HORARIO){
                    if(motor_llego_fin() || mi_motor.angulo_actual >= ANGULO_MAXIMO){
                        ESP_LOGI(TAG, "Tope HORARIO alcanzado (180°). Invirtiendo sentido...");
                        mi_motor.angulo_actual = ANGULO_MAXIMO;
                        direccion_barrido = GIRO_ANTIHORARIO;
                        estado_actual = ESTADO_BARRIDO_ANTIHORARIO;
                    } else{
                        estado_actual = ESTADO_BARRIDO_HORARIO;
                    }
                } else{
                    if(motor_llego_inicio() || mi_motor.angulo_actual <= ANGULO_MINIMO){
                        ESP_LOGI(TAG, "Tope ANTIHORARIO alcanzado (0°). Invirtiendo sentido...");
                        mi_motor.angulo_actual = ANGULO_MINIMO;
                        direccion_barrido = GIRO_HORARIO;
                        estado_actual = ESTADO_BARRIDO_HORARIO;
                    } else{
                        estado_actual = ESTADO_BARRIDO_ANTIHORARIO;
                    }
                }
                break;

                //PD: energiza el puente y suma o resta 15° llamando a motor avanzar. El motor gira 150 ms. se detiene y cambia a estado lectura para que en la siguiente vuelta el while vuelva a medir en esta nueva posición.
            case ESTADO_BARRIDO_HORARIO:
                motor_avanzar_paso(&mi_motor, GIRO_HORARIO);
                vTaskDelay(pdMS_TO_TICKS(TIEMPO_PASO_MS));
                motor_detener(&mi_motor);

                estado_actual = ESTADO_LECTURA;
                break;

            case ESTADO_BARRIDO_ANTIHORARIO:
                motor_avanzar_paso(&mi_motor, GIRO_ANTIHORARIO);
                vTaskDelay(pdMS_TO_TICKS(TIEMPO_PASO_MS));
                motor_detener(&mi_motor);

                estado_actual = ESTADO_LECTURA;
                break;
            default://PD:si hay algún ruido eléctrico o falla de memoria el estado actual toma un valor invalido el default lo reinicia
                estado_actual = ESTADO_INICIALIZACION;
                break;    
        }
    }
}
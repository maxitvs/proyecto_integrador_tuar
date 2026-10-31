// Control físico de los transistores del Puente H
#include "motor.h"
#include "setup.h"
#include "esp_log.h"

static const char *TAG = "MOTOR";

// ---INICIALIZAMOS EL MOTOR---

//PD: en esta función configuramos los pines 18 y 19 como salidas digitales, aseguramos que el puente H arranque frenado (0V en ambos pines) y ponemos el motor en su posición inicial (0° y GIRO_DETENIDO) con el puntero ->
void motor_inicializar(control_motor_t *motor){
    ESP_LOGI(TAG, "Inicializando pines del Puente H...");

    // 1. Limpiamos y configuramos los pines del motor como SALIDA
    gpio_reset_pin(PIN_MOTOR_IN1);
    gpio_reset_pin(PIN_MOTOR_IN2);
    gpio_set_direction(PIN_MOTOR_IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_MOTOR_IN2, GPIO_MODE_OUTPUT);

    // 2. Frenamos el motor
    gpio_set_level(PIN_MOTOR_IN1 0);
    gpio_set_level(PIN_MOTOR_IN2 0);

    // 3. Inicializamos los datos en la estructura original usando el puntero
    if (motor != NULL){ // PD: le ponemos el null para verificar que el puntero apunte a una dirección de memoria válida antes de hacer la modificación
        motor->angulo_actual = ANGULO_MINIMO; //Arranca en 0°
        motor->sentido = GIRO_DETENIDO;
    }
}

// ---CONTROL DEL MOVIMIENTO DEL PUENTE H---

//PD: para girar en SENTIDO HORARIO (0° a 180°) ponemos IN1 = 1 e IN2 = 0, y le sumamos el PASO_ANGULAR (15°) al ángulo actual; para girar en SENTIDO ANTIHORARIO configuramos al revés y le restamos el PASO_ANGULAR
void motor_avanzar_paso(control_motor_t *motor, sentido_giro_t direccion){
    if(motor == NULL) return; // PD: de nuevo ponemos esta barrera de seguridad para que el esp no cuelgue
    motor->sentido = direccion; // PD:vamos a la estructura en main.c y anotamos en que sentido se está moviendo el motor EN ESE MOMENTO, ANTES DE EMPEZAR CON ESTO QUE SIGUE
    if(direccion == GIRO_HORARIO){
        // Activamos el puente H en sentido horario
        //PD: ponemos el pin 1 a 3,3V y el 2 en 0V excitando los transistores de una de las diagonales del puente H haciendo que la corriente pase por el motor de izq a der, haciendolo girar en s. horario
        gpio_set_level(PIN_MOTOR_IN1, 1);
        gpio_set_level(PIN_MOTOR_IN2, 0);

        motor->angulo_actual += PASO_ANGULAR;
        if(motor->angulo_actual > ANGULO_MAXIMO){ //PD: lo clavamos en 180° como máx para que no se pase
            motor->angulo_actual = ANGULO_MAXIMO;
        }
        ESP_LOGI(TAG, "Giro HORARIO -> Ángulo actual: %d grados", motor->angulo_actual);
    }
    else if(direccion == GIRO_ANTIHORARIO){
        // Cambiamos la polaridad del puente H en sentido antihorario
        //PD: hacemos lo contrario, activando la otra diagonal girando el motor en la dirección opuesta
        gpio_set_level(PIN_MOTOR_IN1, 0);
        gpio_set_level(PIN_MOTOR_IN2, 1);

        motor->angulo_actual -= PASO_ANGULAR;//PD: le restamos 15 al ángulo actual
        if(motor->angulo_actual < ANGULO_MINIMO){//PD:ponemos el tope para que no se pase del mínimo
            motor->angulo_actual = ANGULO_MINIMO;
        }
        ESP_LOGI(TAG, "Giro ANTIHORARIO -> Ángulo actual: %d grados", motor->angulo_actual);
    }
}

//---DETENCIÓN DEL MOTOR---
//PD: necesitamos esa función tanto para frenarlo cuando no lo usamos más, como también para que el sensor pueda medir sin tener vibraciones en el motor
void motor_detener(control_motor_t *motor){
    gpio_set_level(PIN_MOTOR_IN1, 0);//PD: al no haber tensión en ningún pin el motor se detiene 
    gpio_set_level(PIN_MOTOR_IN2, 0);
    
    if(motor != NULL){
        motor->sentido = GIRO_DETENIDO;
    }
}

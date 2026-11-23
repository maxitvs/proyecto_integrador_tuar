// Control físico de los transistores del Puente H (2 BC327 PNP arriba + 2 2N2222 NPN abajo)
#include <stddef.h>
#include "motor.h"
#include "setup.h"
#include "esp_log.h"

static const char *TAG = "MOTOR";

// ---NIVELES LÓGICOS DE CADA TRANSISTOR---
//PD: los BC327 (PNP) se activan con 0V en la base y se cortan con 3,3V. Los 2N2222 (NPN) se activan con 3,3V y se cortan con 0V
#define PNP_ACTIVO   0
#define PNP_CORTE    1
#define NPN_ACTIVO   1
#define NPN_CORTE    0

// ---FUNCIONES INTERNAS---

//PD: primero se deja el nivel de apagado en el registro y recién después se pasa el pin a salida, así nunca hay un instante con un transistor activo
static void pin_salida_apagado(gpio_num_t pin, int nivel_apagado){
    gpio_reset_pin(pin);
    gpio_set_level(pin, nivel_apagado);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

//PD: motor detenido = solo los 2 BC327 con tensión en la base (Q1 = 1, Q2 = 1) y los 2N2222 en 0. Ninguna rama conduce
static void puente_apagar(void){
    gpio_set_level(PIN_Q3, NPN_CORTE);
    gpio_set_level(PIN_Q4, NPN_CORTE);
    gpio_set_level(PIN_Q1, PNP_CORTE);
    gpio_set_level(PIN_Q2, PNP_CORTE);
}

// ---INICIALIZAMOS EL MOTOR---

//PD: arranque seguro: los BC327 (Q1, Q2) quedan con tensión en la base (rama superior apagada) y los 2N2222 (Q3, Q4) sin tensión. Después se pone GIRO_DETENIDO en la estructura con el puntero ->
void motor_inicializar(control_motor_t *motor){
    ESP_LOGI(TAG, "Inicializando pines del Puente H...");

    pin_salida_apagado(PIN_Q1, PNP_CORTE);
    pin_salida_apagado(PIN_Q2, PNP_CORTE);
    pin_salida_apagado(PIN_Q3, NPN_CORTE);
    pin_salida_apagado(PIN_Q4, NPN_CORTE);

    if (motor != NULL){ // PD: verificamos que el puntero apunte a una dirección de memoria válida antes de modificar
        motor->sentido = GIRO_DETENIDO;
    }
}

// ---CONTROL DEL MOVIMIENTO DEL PUENTE H---

//PD: SENTIDO HORARIO = diagonal Q1 + Q4; SENTIDO ANTIHORARIO = diagonal Q2 + Q3.
//PD: sin pausa de tiempo muerto: antes de activar una diagonal se apaga primero la contraria, y en el flujo de main.c siempre se llama a motor_detener() entre pasos
void motor_avanzar_paso(control_motor_t *motor, sentido_giro_t direccion){
    if(motor == NULL) return; // PD: barrera de seguridad para que el esp no cuelgue
    motor->sentido = direccion; // PD: anotamos en la estructura en qué sentido se mueve el motor EN ESE MOMENTO
    if(direccion == GIRO_HORARIO){
        // 1. Apagamos la diagonal contraria (Q2 + Q3)
        gpio_set_level(PIN_Q3, NPN_CORTE);
        gpio_set_level(PIN_Q2, PNP_CORTE);
        // 2. Activamos Q1 (0V en la base) + Q4 (3,3V en la base)
        gpio_set_level(PIN_Q1, PNP_ACTIVO);
        gpio_set_level(PIN_Q4, NPN_ACTIVO);

        ESP_LOGI(TAG, "Giro HORARIO");
    }
    else if(direccion == GIRO_ANTIHORARIO){
        // 1. Apagamos la diagonal contraria (Q1 + Q4)
        gpio_set_level(PIN_Q4, NPN_CORTE);
        gpio_set_level(PIN_Q1, PNP_CORTE);
        // 2. Activamos Q2 (0V en la base) + Q3 (3,3V en la base)
        gpio_set_level(PIN_Q2, PNP_ACTIVO);
        gpio_set_level(PIN_Q3, NPN_ACTIVO);

        ESP_LOGI(TAG, "Giro ANTIHORARIO");
    }
}

//---DETENCIÓN DEL MOTOR---
//PD: la usamos tanto para frenarlo como para que el sensor mida sin vibraciones. Deja solo los 2 BC327 con tensión en la base
void motor_detener(control_motor_t *motor){
    puente_apagar();

    if(motor != NULL){
        motor->sentido = GIRO_DETENIDO;
    }
}

//---FINALES DE CARRERA---
//PD: cableados con COM y NC + pull-up interno: en reposo el contacto cierra a GND (lee 0) y al presionarse se abre y el pull-up lo sube a 1. Si se corta un cable también lee 1, por lo que falla del lado seguro (el motor frena)
bool motor_llego_inicio(void)
{
    // Devuelve true si el final de carrera ANTIHORARIO (Inicio) fue presionado (donde frena cuando viene girando en sentido antihorario)
    return (gpio_get_level(PIN_FIN_CARRERA_ANTIHORARIO) == 1);
}
bool motor_llego_fin(void)
{
    // Devuelve true si el final de carrera HORARIO (Fin) fue presionado (donde frena cuando viene girando en sentido horario)
    return (gpio_get_level(PIN_FIN_CARRERA_HORARIO) == 1);
}
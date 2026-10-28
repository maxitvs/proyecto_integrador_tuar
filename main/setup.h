// Mapa de pines del ESP32 (#define), constantes y control de LEDs

#ifndef SETUP_H
#define SETUP_H
#include <stdbool.h>
#include "driver/gpio.h"

// ---DEFINICIÓN DE PINES DE HARDWARE (GPIO)---

// PD: usamos los define porque no crean una variable en la memoria RAM. Antes de compilar, el preprocesador busca PIN_MOTOR_IN1 y lo reemplaza por GPIO_NUM_18. se usaron los pines más "comodos".

// Puente H discreto (control de giro del motor CC)
#define PIN_MOTOR_IN1       GPIO_NUM_18
#define PIN_MOTOR_IN2       GPIO_NUM_19

// Sensor ultrasónico PING (3 pines: VCC, GND y señal bidireccional SIG)
#define PIN_SENSOR_PING     GPIO_NUM_4

// LEDs (LEDs de rango de detección)
#define PIN_LED_VERDE       GPIO_NUM_21
#define PIN_LED_AMARILLO    GPIO_NUM_22
#define PIN_LED_ROJO        GPIO_NUM_23

// Barrido angular
#define ANGULO_MINIMO 0
#define ANGULO_MAXIMO 180
#define PASO_ANGULAR 15 //Grados que avanza o retrocede el radar en cada movimiento antes de frenar para medir con el sensor ultrasónico

// Umbrales de distancia
// PD: acá usamos marcamos específicamente el float porque el esp tiene un sistema para hacer cuentas instantáneas con n° de 32 bits (lo que ocupa el float), si no ponemos nada lo toma como double (65 bits) y tarda mucho más
#define UMBRAL_CERCA_CM 15.0f // Menor a 15 cms. --> Alerta (LED Rojo)
#define UMBRAL_MEDIO_CM 40.0f // Entre 15 y 40 cms. --> Precacución (LED Amarillo)


// ---ESTADOS DE LA MÁQUINA DE ESTADOS FINITOS (FSM)---
// PD: enum (enumeración) va a indicar que la variable va a poder usar sólo 1 de los 4 valores, donde definimos al primero en 0 y el siguiente es 1 etc. Con typedef creamos el tipo de dato nuevo al que le damos por nombre estado_sistema_t que esta última t lo define para que se sepa que es un tipo de dato y no una variable. Dsp lo podemos usar en el main.c y poner directamente estado_sistema_t estado_actual = ESTADO_INICIALIZACION; y queda más prolijo.
typedef enum {
    ESTADO_INICIALIZACION = 0,
    ESTADO_BARRIDO_HORARIO,
    ESTADO_LECTURA,
    ESTADO_BARRIDO_ANTIHORARIO
} estado_sistema_t;

//Prototipo de funciones de setup.c
void setup_inicializar_hardware(void);
void setup_actualizar_leds(float distancia_cm, bool objeto_detectado);

#endif
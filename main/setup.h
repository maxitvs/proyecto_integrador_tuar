// Mapa de pines del ESP32 (#define), constantes y control de LEDs

#ifndef SETUP_H
#define SETUP_H
#include <stdbool.h>
#include "driver/gpio.h"


// ---DEFINICIÓN DE PINES DE HARDWARE (GPIO)---

// PD: usamos los define porque no crean una variable en la memoria RAM. Antes de compilar, el preprocesador busca PIN_Q1 y lo reemplaza por GPIO_NUM_13. se usaron los pines más "comodos".

// Puente H discreto con 4 transistores (arriba 2 BC327 PNP, abajo 2 2N2222 NPN)
#define PIN_Q1              GPIO_NUM_13   // BC327 izquierdo (PNP, activo en 0)
#define PIN_Q2              GPIO_NUM_14   // BC327 derecho (PNP, activo en 0)
#define PIN_Q3              GPIO_NUM_25   // 2N2222 izquierdo (NPN, activo en 1)
#define PIN_Q4              GPIO_NUM_26   // 2N2222 derecho (NPN, activo en 1)

// Sensor ultrasónico PING (4 pines: VCC, GND, Trig, Echo)
#define PIN_SENSOR_TRIG     GPIO_NUM_4
#define PIN_SENSOR_ECHO     GPIO_NUM_5

// Finales de carrera, limite del pendulo (cableados COM + NC con pull-up interno: reposo = 0, presionado = 1)
#define PIN_FIN_CARRERA_HORARIO      GPIO_NUM_32
#define PIN_FIN_CARRERA_ANTIHORARIO  GPIO_NUM_33

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


//Prototipo de funciones para setup.c
void setup_inicializar_hardware(void);
void setup_actualizar_leds(float distancia_cm, bool objeto_detectado);

#endif
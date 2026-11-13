// Mapa de pines del ESP32 (#define), constantes y control de LEDs

#ifndef SETUP_H
#define SETUP_H

#include <stdbool.h>
#include "driver/gpio.h"

// --- DEFINICIÓN DE PINES DE HARDWARE (GPIO) ---

// Puente H discreto (control de giro del motor CC)
#define PIN_MOTOR_IN1               GPIO_NUM_18
#define PIN_MOTOR_IN2               GPIO_NUM_19

// Sensor ultrasónico HC-SR04 (4 pines: VCC, GND, Trig, Echo)
#define PIN_SENSOR_TRIG             GPIO_NUM_4
#define PIN_SENSOR_ECHO             GPIO_NUM_5

// Finales de carrera, utilizados como límite físico del péndulo (activos en bajo)
#define PIN_FIN_CARRERA_HORARIO     GPIO_NUM_25 // Tope de 180°
#define PIN_FIN_CARRERA_ANTIHORARIO GPIO_NUM_26 // Tope de 0° (Inicio)

// LEDs indicadores de rango de detección y fallas
#define PIN_LED_VERDE               GPIO_NUM_21
#define PIN_LED_AMARILLO            GPIO_NUM_22
#define PIN_LED_ROJO                GPIO_NUM_23

// --- TIEMPOS DE CONTROL Y SEGURIDAD (en milisegundos) ---
#define TIEMPO_PASO_MS              150  // Duración máxima de cada micropaso del motor
#define INTERVALO_MUESTREO_FC_MS    10   // Cada cuánto chequeamos los finales de carrera mientras gira
#define TIEMPO_LECTURA_MS           350  // Pausa con el motor detenido para estabilizar y medir
#define TIEMPO_MAX_RECORRIDO_MS     5000 // Protección: tiempo máximo acumulado para llegar de tope a tope
#define TIEMPO_ANTIREBOTE_MS        20   // Filtro mecánico (debounce) para los microswitches

// --- UMBRALES DE DISTANCIA ---
#define UMBRAL_CERCA_CM             15.0f // Menor a 15 cm --> Alerta (LED Rojo)
#define UMBRAL_MEDIO_CM             40.0f // Entre 15 y 40 cm --> Precaución (LED Amarillo)

// --- ESTADOS DE LA MÁQUINA DE ESTADOS FINITOS (FSM) ---
typedef enum {
    ESTADO_INICIALIZACION = 0,
    ESTADO_LECTURA,
    ESTADO_BARRIDO_HORARIO,
    ESTADO_BARRIDO_ANTIHORARIO,
    ESTADO_ERROR // Nuevo estado de protección ante falla mecánica o de sensores
} estado_sistema_t;

// Prototipos de funciones para setup.c
void setup_inicializar_hardware(void);
void setup_actualizar_leds(float distancia_cm, int estado_lectura);
void setup_indicar_error_hardware(void);

#endif

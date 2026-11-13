// Estructura de medición y prototipos del sensor ultrasónico (4 pines)
#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>

// Corrección 6: Diferenciamos las tres situaciones posibles en la lectura
typedef enum {
    SENSOR_OBJETO_DETECTADO = 0, // Rebote válido dentro del rango
    SENSOR_SIN_OBJETO,           // Camino despejado (eco fuera de rango máximo)
    SENSOR_ERROR_LECTURA         // Falla de hardware: el pin Echo nunca subió a 1
} estado_sensor_t;

// --- ESTRUCTURA PARA ALMACENAR LECTURA DEL SENSOR ---
typedef struct {
    float distancia_cm;
    estado_sensor_t estado;
} datos_sensor_t;

// Prototipos de las funciones para sensor.c
void sensor_inicializar(void);
estado_sensor_t sensor_leer_distancia(datos_sensor_t *datos);

#endif

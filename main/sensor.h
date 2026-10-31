// Estructura de medición y prototipos del sensor PING
#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>

//---ESTRUCTURA PARA ALMACENAR LECTURA DEL SENSOR---
//PD: creamos esta estructura para que nos quede la función que necesitamos, donde usamos esos dos variables que siempre van a ir juntas, en un mismo paquete
typedef struct{
    float distancia_cm;//PD:guardamos los datos en en cm (con coma) que obtiene el sensor
    bool objeto_detectado;//PD: true si el eco ultrasónico rebotó contra algo dentro del rango válido o false si el pulso se perdió en el espacio al no chocar contra nada en el rango
} datos_sensor_t;

// Prototipo de las funciones para sensor.c
void sensor_inicializar(void);//PD:el segundo void indica que no necesita recibir ningún dato para funcionar porque ya sabe que pin configurar leyéndolo desde setup.h
float sensor_leer_distancia(datos_sensor_t *datos);

#endif
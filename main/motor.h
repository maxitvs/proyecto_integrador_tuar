// Tipos de datos del motor y prototipos del Puente H
#ifndef MOTOR_H
#define MOTOR_H

#include <stdbool.h>

// --- SENTIDO DE GIRO DEL PUENTE H ---
typedef enum {
    GIRO_DETENIDO = 0,
    GIRO_HORARIO,
    GIRO_ANTIHORARIO
} sentido_giro_t;

// --- ESTRUCTURA DE ESTADO ACTUAL DEL MOTOR ---
// Correcciones 1 y 5: Eliminamos el ángulo estimado ficticio y llevamos
// el control del tiempo acumulado de recorrido para proteger el mecanismo.
typedef struct {
    sentido_giro_t sentido;
    int paso_actual;          // Contador referencial de paradas realizadas en el barrido
    int tiempo_recorrido_ms;  // Tiempo total acumulado viajando en el sentido actual
} control_motor_t;

// Prototipos de funciones para motor.c
void motor_inicializar(control_motor_t *motor);
bool motor_ejecutar_paso(control_motor_t *motor, sentido_giro_t direccion);
void motor_detener(control_motor_t *motor);
bool motor_llego_inicio(void);
bool motor_llego_fin(void);

#endif

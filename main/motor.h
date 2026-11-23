// Tipos de datos del motor (sentido) y prototipos del Puente H
#ifndef MOTOR_H
#define MOTOR_H
#include <stdbool.h>

// ---SENTIDO DE GIRO DEL PUENTE H---

//PD: el puente H usa 4 transistores (Q1/Q2 = BC327 PNP activos en 0, Q3/Q4 = 2N2222 NPN activos en 1). Estados: detenido (Q1=1 Q2=1 Q3=0 Q4=0), giro horario (Q1+Q4 activos) y giro antihorario (Q2+Q3 activos)
typedef enum{
    GIRO_DETENIDO = 0,
    GIRO_HORARIO,
    GIRO_ANTIHORARIO
} sentido_giro_t;


//---ESTRUCTURA DE ESTADO ACTUAL DEL MOTOR---

//PD: guarda hacia donde se está moviendo el motor en este momento
typedef struct{
    sentido_giro_t sentido;
} control_motor_t;


// Prototipo de funciones para motor.c
void motor_inicializar(control_motor_t *motor);
void motor_avanzar_paso(control_motor_t *motor, sentido_giro_t direccion);
void motor_detener(control_motor_t *motor);
bool motor_llego_inicio(void);
bool motor_llego_fin(void);

#endif
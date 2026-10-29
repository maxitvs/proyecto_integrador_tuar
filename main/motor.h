// Tipos de datos del motor (sentido) y prototipos del Puente H
#ifndef MOTOR_H
#define MOTOR_H

// ---SENTIDO DE GIRO DEL PUENTE H---

//PD: para manejar el puente H con los 2 pines que definimos en setup.h el motor va a tener 3 estados posibles: detenido (pines en 0V), giro horario (IN1 = 1 | IN2 = 0) y giro antihorario (IN1 = 0 | IN2= 1)
typedef enum{
    GIRO_DETENIDO = 0,
    GIRO_HORARIO,
    GIRO_ANTIHORARIO
} sentido_giro_t;


//---ESTRUCTURA DE ESTADO ACTUAL DEL MOTOR---

//PD: usamos una estructura porque para que el radar sepa donde está parado y hacia donde se mueve usamos 2 tipos distintos de datos
typedef struct{
    int angulo_actual;
    sentido_giro_t sentido;
} control_motor_t;


//Prototipo de funciones para motor.c
void motor_inicializar(control_motor_t *motor);
void motor_avanzar_paso(control_motor_t *motor, sentido_giro_t direccion);
void motor_detener(control_motor_t *motor);

#endif
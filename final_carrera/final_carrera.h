// Detección de topes y lógica de inversión de giro del péndulo
#ifndef FINAL_CARRERA_H
#define FINAL_CARRERA_H

#include <stdbool.h>
#include "motor.h"

// Prototipo de funciones para final_carrera.c
void final_carrera_inicializar(void);
bool final_carrera_horario_activado(void);
bool final_carrera_antihorario_activado(void);
sentido_giro_t final_carrera_actualizar_sentido(sentido_giro_t sentido_actual);

#endif

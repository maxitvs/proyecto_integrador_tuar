// Inicialización de GPIOs y lógica de los LEDs
#include "setup.h"
#include "esp_log.h"

static const char *TAG = "SETUP"; // PD: static para que este TAG (variable global) sea privada para este archivo setup.c ya que usamos TAG también en otros archivos.

// ---CONFIGURACIÓN DE PINES LEDS---
void setup_inicializar_hardware(void){

}

void setup_actualizar_leds(float distancia_cm, bool objeto_detectado){
    
}
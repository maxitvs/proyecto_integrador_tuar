// Inicialización de GPIOs y lógica de los LEDs
#include "setup.h"
#include "esp_log.h"

static const char *TAG = "SETUP"; // PD: static para que este TAG (variable global) sea privada para este archivo setup.c ya que usamos TAG también en otros archivos. el TAG va a ser usado por el ESP32 para armar el encabezado de una línea que nos devuelve en la terminal por ejemplo: "I (340) SETUP: Configurando pines de LEDs indicadores..." esto nos sirve ya que teniendo varios archivos .c podemos mirar esa palabra en mayúscula que nos dice al instante que arhivo .c generó el mensaje o falla.


// ---CONFIGURACIÓN DE PINES LEDS---

// PD:en esta función configuramos solo los pines de los LEDs, los del motor y sensor van en sus respectivos archivos
void setup_inicializar_hardware(void){ // Configuramos los pines como salidas digitales y para que arranquen todos apagados.
    // PD: en proyectos con ESP, en vez de usar printf() vamos a usar la librería de registros (logging), la cual tiene distintas funciones como ESP_LOGI para información
    ESP_LOGI(TAG, "Configurando pines de LEDs indicadores...");

    // 1. Limpiamos cualquier configuracion previa de los pines
    gpio_reset_pin(PIN_LED_VERDE);
    gpio_reset_pin(PIN_LED_AMARILLO);
    gpio_reset_pin(PIN_LED_ROJO);

    // 2. Los configuramos como SALIDA (OUTPUT)
    // PD: definimos el sentido de la corriente (salida en este caso) haciendo que se habiliten sus transistores internos para poder encender los LEDS
    gpio_set_direction(PIN_LED_VERDE, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_AMARILLO, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_ROJO, GPIO_MODE_OUTPUT);

    // 3. Estado inicial seguro: todos los LEDs apagados (0V)
    // PD: nos aseguramos que los 3 LEDs arranquen apagados hasta que el sensor haga su primera lectura real (cuando lee y pone un 1 el pin entrega 3.3V y el LED se prende)
    gpio_set_level(PIN_LED_VERDE, 0);
    gpio_set_level(PIN_LED_AMARILLO, 0);
    gpio_set_level(PIN_LED_ROJO, 0);
}

// ---ENCENDIDO DE LEDS SEGÚN MEDICIÓN---
// PD: en esta función recibimos desde sensor. los parametros distancia_cm que midió el sensor y el booleano objeto_detectado y según los umbrales en cms. que definimos en setup.h prende UN LED y apaga los otros DOS
void setup_actualizar_leds(float distancia_cm, bool objeto_detectado){ //PD: usamos esta estructura selectiva donde en cada llamado se ejecuta sólo uno de los bloques garantizando que nunca queden prendido dos LEDs de distinto color al mismo tiempo
    if (!objeto_detectado || distancia_cm >= UMBRAL_MEDIO_CM) {// PD: se pregunta: no se detectó ningún objeto?; y si la distancia es mayor al umbral de 40 cm (como es 'or' con que una sea true ya está) activa verde y apaga el resto
        // Zona libre o lejana -> LED Verde encendido
        gpio_set_level(PIN_LED_VERDE, 1);
        gpio_set_level(PIN_LED_AMARILLO, 0);
        gpio_set_level(PIN_LED_ROJO, 0);
    } 
    else if (distancia_cm < UMBRAL_CERCA_CM) { //PD: acá ya sabe que hay un objeto y que está a menos de 40 cm
        // Objeto muy cerca (< 15 cm) -> LED Rojo encendido
        gpio_set_level(PIN_LED_VERDE, 0);
        gpio_set_level(PIN_LED_AMARILLO, 0);
        gpio_set_level(PIN_LED_ROJO, 1);
    } 
    else { //PD: acá ya se sabe que está entre 15 y 39.99 cm
        // Objeto a distancia media (entre 15 y 40 cm) -> LED Amarillo encendido
        gpio_set_level(PIN_LED_VERDE, 0);
        gpio_set_level(PIN_LED_AMARILLO, 1);
        gpio_set_level(PIN_LED_ROJO, 0);
    }
}
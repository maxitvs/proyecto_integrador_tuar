// Disparo de pulso ultrasónico y cálculo de distancia en cm
#include "sensor.h"
#include "setup.h"
#include "esp_log.h"
#include "esp_timer.h"//PD: da acceso a un timer que cuenta los ms. desde que se encendió el ESP
#include "rom/ets_sys.h"//PD: da acceso a una pausa en ms. grabada den la memoria ROM de fábrica del ESP

static const char *TAG = "SENSOR";
#define TIMEOUT_ECO_US 30000//PD:definimos un tope de espera de 30.000 us. (30 ms.). Como el sonido recorre 5 mts. en 30 ms (más que el alcance que le pusimos al sensor), si pasa ese tiempo y no hay respuesta sabemos o que está desconectado o que el sonido se perdió en el aire sin rebotar en nada.


//---INICIALIZAMOS EL MOTOR---

// Limpiamos y configuramos los pines del motor como SALIDA y frenamos el motor
void sensor_inicializar(void){
    ESP_LOGI(TAG, "Inicializando pin del sensor PING...");
    gpio_reset_pin(PIN_SENSOR_PING);
    gpio_set_direction(PIN_SENSOR_PING, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SENSOR_PING, 0);
}

//---LECTURA DEL SENSOR---

// Leemos distancia en cada frenado en 5 etapas
//PD: ejecutamos esta función cada vez que el motor frena en un ángulo nuevo
float sensor_leer_distancia(datos_sensor_t *datos){
    if (datos == NULL) return 0.0f;
    // 1. Validación y Disparo: PIN en salida y pulso 
    gpio_set_direction(PIN_SENSOR_PING, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SENSOR_PING, 0);
    ets_delay_us(2);//PD:lo ponemos en 0V por 2 us. para estabilizar la línea
    gpio_set_level(PIN_SENSOR_PING, 1);
    ets_delay_us(5);//PD:lo ponemos a 3.3V por 5 us.
    gpio_set_level(PIN_SENSOR_PING, 0);//PD: lo volvemos a 0V
    //PD: este pulso que generamos le indica al sensor que genere un impulso de ultrasonido

    // 2. Lectura: PIN en entrada para escuchar el sensor
    gpio_set_direction(PIN_SENSOR_PING, GPIO_MODE_INPUT);//PD: en menos de 1 us. cambia a entrada para escuchar

    // 3. Espera de detección: 
    //PD: esperamos a que el pulso empiece a emitir y suba a 1 (usamos el timeout por si está desconectado)
    int64_t t_inicio_espera = esp_timer_get_time();//PD:anotamos hora actual en la variable
    while (gpio_get_level(PIN_SENSOR_PING) == 0){//PD:nos quedamos en el bucle mientras siga en 0
        if ((esp_timer_get_time() - t_inicio_espera)  > TIMEOUT_ECO_US){//PD:restamos la hora actual al inicio de espera y si pasan más de 30.000 us.(30 ms.) y el pin no sube a 1 entra el if
            ESP_LOGW(TAG, "Time out esperando inicio de pulso (Sensor desconectado o sin respuesta)");//PD:avisa por consola la advertencia
            datos->distancia_cm = 0.0f;
            datos->objeto_detectado = false;//PD: guarda que no se detectó nada con puntero
            return 0.0f;//PD: salimos de la función sin colgar el ESP
        }
    }

    // 4. Medición duración del viaje del sonido:
    //PD: cronometramos el viaje del sonido en al aire mientras el pin vale 1
    int64_t t_inicio_pulso = esp_timer_get_time();//PD:en el instante en el que el bucle anterior termina el pin sube a 1 (el sonido acaba de salir al aire), por lo que guardamos ese instante en la variable.
    //PD: el sensor mantiene el pin en 1 durante todo el tiempo en que el sonido viaje por el aire, choca contra un objeto y vuelve. Mientras esté en 1 el segundo while espera
    while (gpio_get_level(PIN_SENSOR_PING) == 1){
        if((esp_timer_get_time() - t_inicio_pulso) > TIMEOUT_ECO_US){//PD:si pasan más de 30 ms. con el pin en 1 y no se detecta nada significa que no hay objeto
            ESP_LOGI(TAG, "Camino despejado (eco fuera de rango máximo)");
            datos->distancia_cm = 999.0f;//PD: marca el máximo, no detecta nada y sale
            datos->objeto_detectado = false;
            return 999.0f;
        }//PD: apenas el eco rebota y vuelve al sensor, éste baja el pin a 0, se rompe el while y guardamos la hora de llegada abajo
    }
    int64_t t_fin_pulso = esp_timer_get_time();

    // 5. Cálculo de distancia en centímetros:
    int64_t duracion_us = t_fin_pulso - t_inicio_pulso;//PD: restamos llegada menos salida y obtenemos cuantos ms. estuvo el sonido viajando en el aire (Usamos int64_t, entero de 64 bits, porque el reloj en microsegundos del ESP32 crece muy rápido y desbordaría un int común de 32 bits en poco más de una hora)
    datos->distancia_cm = (duracion_us * 0.0343f) / 2.0f;//PD: hacemos lo anterior por la velocidad del sonido en al aire (pasada a cm por us.), lo que sería tiempo x velocidad y dividimos por 2 (ya que el tiempo medido incluye el ida y vuelta pero queremos solo un tramo)
    datos->objeto_detectado = true;//PD: guardamos en datos los valores encontramos

    ESP_LOGI(TAG, "Distancia medida: %.2f cm", datos->distancia_cm);//PD: mostramos en consola y retornamos el valor
    return datos->distancia_cm;
}
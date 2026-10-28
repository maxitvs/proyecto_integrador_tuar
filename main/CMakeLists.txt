idf_component_register(SRCS "main.c" "motor.c" "sensor.c" "setup.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_driver_gpio)
/*
 * Author: Christogarm
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/* Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "temperature_sensor_if.h"

    /* Configuracion del Sensor */
typedef struct 
{
    float highTemperature;
    float lowTemperature;
	float difTemperature; // Diferencia de temperatura para marcar el sensor como fallido

    float errorTemperature;

	float failedReadingTime_s;

    temperature_device_t device; // Sensor de Temperatura
}temperature_config_t;

    /* Sensor de Temperatura */
typedef struct
{
    temperature_config_t config;                              // Configuraciones del sensor de Temperatura

    float temperature;                                              // Temperatura del Sensor
    float tempSamples[TEMPERATURE_NUM_SAMPLES];                              // Muestras
    uint8_t countSamples;
    float accumulatedFailedTime_s;
	bool firstFullBuffer;                                            // Bandera para saber si el buffer esta lleno por primera vez

    temperature_states_t state;                                // Estado del sensor
    temperature_warnings_t warnings;                          // Advertencias del sensor
    bool init;
}temperatureSensor_t;

/* Funciones Publicas */
temperature_status_t temperature_init(temperatureSensor_t * sensor_, const temperature_config_t * config_);
temperature_status_t temperature_updateSample(temperatureSensor_t * sensor_, float timeSeconds_);
float temperature_getTemperature(temperatureSensor_t * sensor_);
temperature_states_t temperature_getState(temperatureSensor_t * sensor_);
temperature_warnings_t temperature_getWarnings(temperatureSensor_t * sensor_);
temperature_status_t temperature_reset(temperatureSensor_t * sensor_);


#endif /* TEMPERATURE_H */
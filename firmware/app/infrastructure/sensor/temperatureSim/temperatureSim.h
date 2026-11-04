/*
 * Author: Christogarm
 */

#ifndef TEMPERATURE_SIM_H
#define TEMPERATURE_SIM_H

/* Includes */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "temperature_sensor_if.h"
#include "interpolation.h"
#include "timeSim.h"

#define TEMP_SIM_READ_MAX_LINE_CHAR        100
#define TEMP_SIM_MAX_FIELD_LEN              100


    /* Configuracion del Sensor */
typedef struct 
{
    const char* filename;      // Nombre del Archivo

}temperatureSim_config_t;

    /* Sensor de Temperatura */
typedef struct
{
    temperatureSim_config_t config;                              // Configuraciones del sensor de Temperatura

    time_sim_t time;                                                // Tiempo
    timeData_t time_init;
    timeData_t timeData;
    float temperature;                                              // Temperatura del Sensor
    
    float tabTemperature[24];
    float tabTimeUnix[24];
    interpolation_t interpolation;

    FILE* file;
    
    bool init;
}temperatureSim_t;

temperature_status_t temperatureSim_init(void* instance, void* config);
temperature_states_t temperatureSim_updateSample(void* instance, float* temperature);
temperature_status_t temperatureSim_reset(void* instance);



#endif /* TEMPERATURE_SIM_H */
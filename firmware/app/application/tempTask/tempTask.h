/*
 * Author: Christogarm
 */

#ifndef TEMP_TASK_H
#define TEMP_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "rtos_resources.h"
#include "temperature.h"
#include "config.h"



typedef struct
{
	uint32_t timeDelayTask_ms;										// Retraso de la tarea en milisegundos
	queueID_t queueID;													// Cola para enviar los datos de temperatura
	
	transductor_bit_t bitSensor;											// Bandera para indicar que la data esta lista
	config_device_flag_t flagEnable;								// Posicion de la configuracion en la memoria RAM
}tempTask_config_t;

typedef struct
{
	tempTask_config_t config;									// Configuracion de la tarea
	
	temperatureSensor_t sensor;										// Sensor de Temperatura
	
	configTempSensor_t configTemp;												// Configuracion del sensor de temperatura

	bool configFailed;													// Bandera para indicar si la configuracion de la tarea fallo
	float temperature;													// Valor de temperatura
	temperature_warnings_t warnings;											// Advertencias de temperatura
	temperature_states_t state;											// Estado del sensor de temperatura
}tempTask_sensor_t;

void vTempTask(void *pvParameters);

#endif /* TEMP_TASK_H */
/*
 * Author: Christogarm
 */

#ifndef TEMP_TASK_H
#define TEMP_TASK_H

#include <stdint.h>
#include "rtos_resources.h"
#include "temperature.h"
#include "config.h"



typedef struct
{
	uint32_t timeDelayTask_ms;										// Retraso de la tarea en milisegundos
	uint32_t numTimesToSendData;									// Numero de veces que se enviara la data de temperatura
	
	temperatureSensor_t sensor;										// Sensor de Temperatura
	queueID_t queueID;												// Cola para enviar los datos de temperatura
	readyData_t readyData;											// Bandera para indicar que la data esta lista

	config_device_flag_t flagEnable;								// Posicion de la configuracion en la memoria RAM
}tempTask_config_t;

typedef struct
{
	tempTask_config_t config;									// Configuracion de la tarea
	uint32_t countSamples;										// Contador de muestras
	queueData_t dataSensor;											// Estructura de datos para enviar a la cola
}tempTask_sensor_t;

void vTempTask(void *pvParameters);

#endif /* TEMP_TASK_H */
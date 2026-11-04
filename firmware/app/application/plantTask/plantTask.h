/*
 * Author: Christogarm
 */

#ifndef PLANT_TASK_H
#define PLANT_TASK_H

#include <stdint.h>
#include "rtos_resources.h"

#include "plant.h"



typedef struct
{
	configPlant* configPlant;

	uint32_t timeDelayTask_ms;										// Retraso de la tarea en milisegundos
	
	queueID_t queueID;												// Cola para enviar los datos de temperatura
	readyData_t readyData;											// Bandera para indicar que la data esta lista

}plantTask_config_t;

typedef struct
{
	plantTask_config_t config;										// Configuracion de la tarea
	queueData_t dataSensor;											// Estructura de datos para enviar a la cola
	Plant_t plant;
	float PIDout;
}plantTask_t;

void vPlantTask(void *pvParameters);

#endif /* PLANT_TASK_H */
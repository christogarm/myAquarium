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
	uint32_t timeDelayTask_ms;										// Retraso de la tarea en milisegundos

}plantTask_config_t;

typedef struct
{
	plantTask_config_t config;										// Configuracion de la tarea
	queueData_t dataSensor;											// Estructura de datos para enviar a la cola
	Plant_t plant;
	float heaterPower;
}plantTask_t;

void vPlantTask(void *pvParameters);

#endif /* PLANT_TASK_H */
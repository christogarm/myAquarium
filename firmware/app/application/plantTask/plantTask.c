/*
 * Author: Christogarm
 */

#include "plantTask.h"
#include "FreeRTOS.h"
#include "config.h"
#include "task.h"

#include "rtos_resources.h"
/* Macros */


/* Variable Privadas */


/* Prototipos de las Funciones Privadas*/
static void send_dataLogger(plantTask_t* tempTaskSensor_);

/* Funciones Publicas */
void vPlantTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    plantTask_t* taskPlant = (plantTask_t*)pvParameters;

    if(taskPlant->config.configPlant == NULL) {
        vTaskDelete(NULL);
        return;
    }

    if (plant_Init(&taskPlant->plant, taskPlant->config.configPlant) != PLANT_STATUS_OK) {
        vTaskDelete(NULL);
        return;
    }   

	float startTime, endTime = 0, timeSeconds;
    taskPlant->PIDout = taskPlant->plant.config.initOutput;
    
    TickType_t xLastWakeTime = xTaskGetTickCount(); 

    for (;;)
    {   
        startTime = getTimeSeconds();
        timeSeconds = startTime - endTime;

        mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
        taskPlant->PIDout = dataLogging.heaterPower;
        mutexGive(MUTEX_ID_LOGGER);

        plant_Update(&taskPlant->plant, taskPlant->PIDout, taskPlant->plant.config.initInput, timeSeconds);

        taskPlant->dataSensor.value = taskPlant->plant.outMeasured;
        taskPlant->dataSensor.failure = 0;
        taskPlant->dataSensor.enable = 1;
        taskPlant->dataSensor.timeStamp = startTime;

        
        setDataQueue_overWrite(&taskPlant->dataSensor, taskPlant->config.queueID);
        send_dataLogger(taskPlant);
        endTime = getTimeSeconds();
        rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPlant->config.timeDelayTask_ms));

    }
    
}

/* Funciones Privadas */
static void send_dataLogger(plantTask_t* plantTask_) {
    mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
	dataLogging.readyData |= plantTask_->config.readyData;
    if (plantTask_->config.readyData & READY_DATA_TEMP_WATER)
        dataLogging.tempWater = plantTask_->dataSensor.value;
    else if (plantTask_->config.readyData & READY_DATA_TEMP_AMB)
        dataLogging.tempAmb = plantTask_->dataSensor.value;

    plantTask_->PIDout = dataLogging.heaterPower;
    mutexGive(MUTEX_ID_LOGGER);
}
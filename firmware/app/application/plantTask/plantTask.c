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
static void update_sysData(plantTask_t* tempTaskSensor_);

/* Funciones Publicas */
void vPlantTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    plantTask_t* taskPlant = (plantTask_t*)pvParameters;

    if (plant_Init(&taskPlant->plant, &taskPlant->plant.config) != PLANT_STATUS_OK) {
        vTaskDelete(NULL);
        return;
    }   

	float startTime, endTime = 0, timeSeconds;
    taskPlant->heaterPower = taskPlant->plant.config.initOutput;
    
    TickType_t xLastWakeTime = xTaskGetTickCount(); 

    for (;;)
    {   
        startTime = getTimeSeconds();
        timeSeconds = startTime - endTime;

        plant_Update(&taskPlant->plant, taskPlant->heaterPower, taskPlant->plant.config.initInput, timeSeconds);

        update_sysData(taskPlant);

        endTime = getTimeSeconds();
        rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(taskPlant->config.timeDelayTask_ms));

    }
    
}

/* Funciones Privadas */
static void update_sysData(plantTask_t* plantTask_) {
    mutexTake(MUTEX_ID_SYS_DATA, RTOS_TIMEOUT_MAX);
	sysData.readyData |= TRANS_BIT_TEMP_WATER;    
    sysData.tempWater = plantTask_->plant.outMeasured;

    plantTask_->heaterPower = sysData.heaterPower;
    mutexGive(MUTEX_ID_SYS_DATA);
}
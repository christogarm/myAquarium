/*
 * Author: Christogarm
 */

#include "tempTask.h"
#include "FreeRTOS.h"
#include "config.h"
#include "task.h"

/* Macros */


/* Variable Privadas */


/* Prototipos de las Funciones Privadas*/
static temperature_status_t vTempTaskInit(tempTask_sensor_t * tempTaskSensor_);
static void send_dataLogger(tempTask_sensor_t* tempTaskSensor_);

/* Funciones Publicas */
void vTempTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
	tempTask_sensor_t* tempTaskSensor = (tempTask_sensor_t*)pvParameters;
    if (vTempTaskInit(tempTaskSensor) != TEMPERATURE_STATUS_OK) {
        vTaskDelete(NULL);
        return;
    }   

    temperatureSensor_t* tempSensor = &tempTaskSensor->config.sensor;
	float startTime, endTime, timeSeconds;
    
    TickType_t xLastWakeTime = xTaskGetTickCount(); 

    for (;;)
    {   
        startTime = getTimeSeconds();

		rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(tempTaskSensor->config.timeDelayTask_ms));

        endTime = getTimeSeconds();
        timeSeconds = endTime - startTime;
        temperature_updateSample(tempSensor, timeSeconds);
        tempTaskSensor->dataSensor.value = temperature_getTemperature(tempSensor);
        tempTaskSensor->dataSensor.failure = temperature_getState(tempSensor) == TEMPERATURE_STATE_FAIL;

        tempTaskSensor->countSamples++;
        if(tempTaskSensor->countSamples >= tempTaskSensor->config.numTimesToSendData){
            tempTaskSensor->countSamples = 0;

			tempTaskSensor->dataSensor.timeStamp = endTime;
            
            mutexTake(MUTEX_ID_CONFIGURATION, RTOS_TIMEOUT_MAX);
            tempTaskSensor->dataSensor.enable = (configData_RAM.flags & tempTaskSensor->config.flagEnable) != 0;
            mutexGive(MUTEX_ID_CONFIGURATION);

            setDataQueue_overWrite(&tempTaskSensor->dataSensor, tempTaskSensor->config.queueID);
        }
        
        send_dataLogger(tempTaskSensor);
    }
    
}

/* Funciones Privadas */
static temperature_status_t vTempTaskInit(tempTask_sensor_t* tempTaskSensor_){

    if(tempTaskSensor_->config.timeDelayTask_ms == 0 ||
        tempTaskSensor_->config.numTimesToSendData == 0){
        return TEMPERATURE_STATUS_ERROR;
    }

    temperatureSensor_t* tempSensor = &tempTaskSensor_->config.sensor;
    if (temperature_init(tempSensor, &tempSensor->config) != TEMPERATURE_STATUS_OK) {
        return TEMPERATURE_STATUS_ERROR;
    }

    tempTaskSensor_->countSamples = 0;

    return TEMPERATURE_STATUS_OK;
}

static void send_dataLogger(tempTask_sensor_t* tempTaskSensor_) {
    mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
	dataLogging.readyData |= tempTaskSensor_->config.readyData;
    if (tempTaskSensor_->config.readyData & READY_DATA_TEMP_WATER)
        dataLogging.tempWater = tempTaskSensor_->dataSensor.value;
    else if (tempTaskSensor_->config.readyData & READY_DATA_TEMP_AMB)
        dataLogging.tempAmb = tempTaskSensor_->dataSensor.value;
    mutexGive(MUTEX_ID_LOGGER);
}
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
static void send_dataSysData(tempTask_sensor_t* tempTaskSensor_);
static void checkWarnings(tempTask_sensor_t* tempTaskSensor_);
static void checkReconfig(tempTask_sensor_t* tempTaskSensor_);
static void setReconfiguration(tempTask_sensor_t* tempTaskSensor_);

/* Funciones Publicas */
void vTempTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
	tempTask_sensor_t* tempTaskSensor = (tempTask_sensor_t*)pvParameters;
    tempTaskSensor->configFailed = vTempTaskInit(tempTaskSensor) != TEMPERATURE_STATUS_OK;

    temperatureSensor_t* tempSensor = &tempTaskSensor->sensor;
	float startTime, endTime = 0, timeSeconds;
    
    TickType_t xLastWakeTime = xTaskGetTickCount(); 

    for (;;)
    {   
        /* Tiempos de la Tarea */
        startTime = getTimeSeconds();
        timeSeconds = startTime - endTime;

        /* Toma de Lecturas del Sensor de Temperatura */
        temperature_updateSample(tempSensor, timeSeconds);
        tempTaskSensor->temperature = temperature_getTemperature(tempSensor);
        tempTaskSensor->warnings = temperature_getWarnings(tempSensor);
        tempTaskSensor->state = temperature_getState(tempSensor);

        /* Revisamos si hay una reconfiguracion */
        checkReconfig(tempTaskSensor);

        /* Compartimos los datos al sistema */
        send_dataSysData(tempTaskSensor);

        /* Toma de Tiempo y Suspension de la Tarea */
        if (tempTaskSensor->configFailed || (!tempTaskSensor->config.flagEnable)) {
            rxDataQueue(&tempTaskSensor->configTemp, tempTaskSensor->config.queueID, portMAX_DELAY);
            setReconfiguration(tempTaskSensor);
        }

        endTime = getTimeSeconds();
        rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(tempTaskSensor->config.timeDelayTask_ms));
    }
    
}

/* Funciones Privadas */
static temperature_status_t vTempTaskInit(tempTask_sensor_t* tempTaskSensor_){

    if(tempTaskSensor_->config.timeDelayTask_ms == 0)
        return TEMPERATURE_STATUS_ERROR;

    temperatureSensor_t* tempSensor = &tempTaskSensor_->sensor;
    if (temperature_init(tempSensor, &tempSensor->config) != TEMPERATURE_STATUS_OK)
        return TEMPERATURE_STATUS_ERROR;

    return TEMPERATURE_STATUS_OK;
}

static void send_dataSysData(tempTask_sensor_t* tempTaskSensor_) {
    mutexTake(MUTEX_ID_SYS_DATA, RTOS_TIMEOUT_MAX);
	sysData.readyData |= tempTaskSensor_->config.bitSensor;
    sysData.enable = tempTaskSensor_->config.flagEnable? 
        (tempTaskSensor_->config.bitSensor | sysData.enable):
        ((~tempTaskSensor_->config.bitSensor) & sysData.enable);
    sysData.failures = tempTaskSensor_->state == TEMPERATURE_STATE_FAIL ?
        (tempTaskSensor_->config.bitSensor | sysData.failures) :
        ((~tempTaskSensor_->config.bitSensor) & sysData.failures);
    checkWarnings(tempTaskSensor_);

    if (tempTaskSensor_->config.bitSensor & TRANS_BIT_TEMP_WATER) {
        sysData.tempWater = tempTaskSensor_->temperature;
        sysData.configTaskInit = tempTaskSensor_->configFailed ?
            (sysData.configTaskInit & (~CONFIG_TASK_INIT_BIT_TEMP_WATER)):
            (sysData.configTaskInit | (CONFIG_TASK_INIT_BIT_TEMP_WATER));
    }
    else if (tempTaskSensor_->config.bitSensor & TRANS_BIT_TEMP_AMB) {
        sysData.tempAmb = tempTaskSensor_->temperature;
        sysData.configTaskInit = tempTaskSensor_->configFailed ?
            (sysData.configTaskInit & (~CONFIG_TASK_INIT_BIT_TEMP_AMB)) :
            (sysData.configTaskInit | (CONFIG_TASK_INIT_BIT_TEMP_AMB));
    }
    mutexGive(MUTEX_ID_SYS_DATA);
}

static void checkWarnings(tempTask_sensor_t* tempTaskSensor_) {

	if (tempTaskSensor_->warnings & TEMPERATURE_WARNING_HIGH) {
		sysData.warnings |= (tempTaskSensor_->config.bitSensor == TRANS_BIT_TEMP_WATER) ? WARNING_BIT_TEMP_WATER_HIGH : WARNING_BIT_TEMP_AMB_HIGH;
	}
	else if (tempTaskSensor_->warnings & TEMPERATURE_WARNING_LOW) {
		sysData.warnings |= (tempTaskSensor_->config.bitSensor == TRANS_BIT_TEMP_WATER) ? WARNING_BIT_TEMP_WATER_LOW : WARNING_BIT_TEMP_AMB_LOW;
	}
	else {
		sysData.warnings &= ~(tempTaskSensor_->config.bitSensor == TRANS_BIT_TEMP_WATER ? (WARNING_BIT_TEMP_WATER_HIGH | WARNING_BIT_TEMP_WATER_LOW) : (WARNING_BIT_TEMP_AMB_HIGH | WARNING_BIT_TEMP_AMB_LOW));
	}
}   

static void checkReconfig(tempTask_sensor_t* tempTaskSensor_) {
	if (rxDataQueue(&tempTaskSensor_->configTemp, tempTaskSensor_->config.queueID, 0) == RTOS_OK) {
        setReconfiguration(tempTaskSensor_);
	}
}

static void setReconfiguration(tempTask_sensor_t* tempTaskSensor_) {
    temperatureSensor_t* tempSensor = &tempTaskSensor_->sensor;

	tempTaskSensor_->config.flagEnable = tempTaskSensor_->configTemp.flagEnable;

    tempSensor->config.highTemperature = tempTaskSensor_->configTemp.highTemperature; // Temperatura mínima simulada
    tempSensor->config.lowTemperature = tempTaskSensor_->configTemp.lowTemperature; // Temperatura mínima simulada
    tempSensor->config.difTemperature = tempTaskSensor_->configTemp.difTemperature; // Diferencia de temperatura para marcar el sensor como fallido
    tempSensor->config.failedReadingTime_s = tempTaskSensor_->configTemp.failedReadingTime_s;

    tempTaskSensor_->configFailed = vTempTaskInit(tempTaskSensor_) != TEMPERATURE_STATUS_OK;
}
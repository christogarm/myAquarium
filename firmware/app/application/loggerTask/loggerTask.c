/*
 * Author: Christogarm
 */

#include "loggerTask.h"

/* Macros */


/* Variable Privadas */
static void detectEvents(loggerTask_log_t* eventLogTask_Logger_,
    sysGroupEvents_t eventsBits);
static void saveDataEventInit(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc);
static void saveDataEventEnd(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc);
static eventLogger_t* selectBufferEventLogger(loggerTask_log_t* eventLogTask_Logger_,
    sysGroupEvents_t eventType);

/* Prototipos de las Funciones Privadas*/

/* Funciones Publicas */
void vTimeLoggerTask(void *pvParameters){

    if(pvParameters == NULL){
		vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    loggerTask_log_t* timeLogTask_Logger = (loggerTask_log_t*)pvParameters;
	logger_t* Logger = (logger_t*)&timeLogTask_Logger->logger;

    if (!logger_isInit(Logger)) {
        vTaskDelete(NULL);
        return;  
    }
     
    /* Incializacion del Tiempo */
	if (timeService_init(&timeLogTask_Logger->timeLog_time, &timeLogTask_Logger->timeLog_time.config) != TIME_STATUS_OK) {
		vTaskDelete(NULL);
		return;
	}

    TickType_t xLastWakeTime = xTaskGetTickCount();
	bool readyAllDataFlag = false;

    for (;;)
    {   
		timeService_getTime(&timeLogTask_Logger->timeLog_time);
        Logger->timeLog.timeUNIX = timeLogTask_Logger->timeLog_time.timeData.timeUNIX;

        mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
        Logger->timeLog.tempAmb = dataLogging.tempAmb;
        Logger->timeLog.tempWater = dataLogging.tempWater;
        Logger->timeLog.heaterPower = dataLogging.heaterPower;
        readyAllDataFlag = (dataLogging.readyData & READY_DATA_ALL) == READY_DATA_ALL;
        mutexGive(MUTEX_ID_LOGGER);

        if (readyAllDataFlag) {
            logger_addTimeRegister(Logger);
        }
            

		rtos_delayUntil(&xLastWakeTime, pdMS_TO_TICKS(timeLogTask_Logger->config.timeAddRegisterTime_ms));
    }
    
}


void vEventLoggerTask(void* pvParameters) {
    if (pvParameters == NULL) {
        vTaskDelete(NULL);
        return;
    }

    /* Inicializacion de Variables */
    loggerTask_log_t* eventLogTask_Logger = (loggerTask_log_t*)pvParameters;
    logger_t* Logger = (logger_t*)&eventLogTask_Logger->logger;

    if (!logger_isInit(Logger)) {
        vTaskDelete(NULL);
        return;
    }

    /* Incializacion del Tiempo */
    if (timeService_init(&eventLogTask_Logger->eventLog_time, &eventLogTask_Logger->eventLog_time.config) != TIME_STATUS_OK) {
        vTaskDelete(NULL);
        return;
    }

    sysGroupEvents_t eventsBits;
    
    for (;;)
    {
        eventsBits = rtos_waitEvent(eventLogTask_Logger->config.typesEvents_enable);
        timeService_getTime(&eventLogTask_Logger->eventLog_time);
        Logger->eventLog.timeUNIX_init = eventLogTask_Logger->eventLog_time.timeData.timeUNIX;

        /* Leemos los datos */
        mutexTake(MUTEX_ID_LOGGER, RTOS_TIMEOUT_MAX);
        Logger->eventLog.tempAmb_init = dataLogging.tempAmb;
        Logger->eventLog.tempWater_init = dataLogging.tempWater;
        Logger->eventLog.heaterPower_init = dataLogging.heaterPower;
        mutexGive(MUTEX_ID_LOGGER);

        /* Detectamos el tipo de eventos*/
        detectEvents(eventLogTask_Logger, eventsBits);

    }
}

/* Funciones Privadas */
static void detectEvents(loggerTask_log_t* eventLogTask_Logger_,
    sysGroupEvents_t eventsBits){

    logger_t* logger_ = &eventLogTask_Logger_->logger;
    eventLogger_t data = logger_->eventLog;
    eventLogger_t* pointBuffLogger;
    sysGroupEvents_t* eventsInit = &eventLogTask_Logger_->initEvents;

    /* Detectamos tipo de evento */
    for (sysGroupEvents_t eventType = SYS_EVENT_GROUP_POWER_ON; eventType < LOGGER_EVENT_TYPE_NUM_EVENTS; eventType*=2)
    {
        pointBuffLogger = selectBufferEventLogger(eventLogTask_Logger_, eventType);
        if (pointBuffLogger == NULL) {
            return;
        }
        else if (eventsBits & eventType != 0) {
            if (eventType == SYS_EVENT_GROUP_POWER_ON ||
                eventType == SYS_EVENT_GROUP_RECONFIG_CONTROL) {
                saveDataEventInit(pointBuffLogger, &data);
                saveDataEventEnd(pointBuffLogger, &data);
                logger_->eventLog = *pointBuffLogger;
                logger_addEventRegister(logger_);
            }
            else if (*eventsInit & eventType != 0) {
                *eventsInit &= ~eventType;
                saveDataEventEnd(pointBuffLogger, &data);
                logger_->eventLog = *pointBuffLogger;
                logger_addEventRegister(logger_);
            }
            else {
                *eventsInit |= eventType;
                saveDataEventInit (pointBuffLogger, &data);
            }

        }
    }

}

static void saveDataEventInit(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc) {
    eventBuffDst->timeUNIX_init = eventBuffSrc->timeUNIX_init;
    eventBuffDst->tempAmb_init = eventBuffSrc->tempAmb_init;
    eventBuffDst->tempWater_init = eventBuffSrc->tempWater_init;
    eventBuffDst->heaterPower_init = eventBuffSrc->heaterPower_init;
}

static void saveDataEventEnd(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc) {
    eventBuffDst->timeUNIX_end = eventBuffSrc->timeUNIX_init;
    eventBuffDst->tempAmb_end = eventBuffSrc->tempAmb_init;
    eventBuffDst->tempWater_end = eventBuffSrc->tempWater_init;
    eventBuffDst->heaterPower_end = eventBuffSrc->heaterPower_init;
}

static eventLogger_t* selectBufferEventLogger(loggerTask_log_t* eventLogTask_Logger_, sysGroupEvents_t eventType) {
    eventLogger_t* pointEventLog = NULL;

    switch (eventType){
        case SYS_EVENT_GROUP_POWER_ON:
            pointEventLog = &eventLogTask_Logger_->config.buffEvents[LOGGER_EVENT_TYPE_FAIL_POWER_ON];
            break;
        case SYS_EVENT_GROUP_RECONFIG_CONTROL:
            pointEventLog = &eventLogTask_Logger_->config.buffEvents[LOGGER_EVENT_TYPE_FAIL_RECONFIGURATION];
            break;
        case SYS_EVENT_GROUP_WATER_TEMP_CHANGE_STATE:
            pointEventLog = &eventLogTask_Logger_->config.buffEvents[LOGGER_EVENT_TYPE_FAIL_WATER_TEMP];
            break;
        case SYS_EVENT_GROUP_AMB_TEMP_CHANGE_STATE:
            pointEventLog = &eventLogTask_Logger_->config.buffEvents[LOGGER_EVENT_TYPE_FAIL_AMB_TEMP];
            break;
        case SYS_EVENT_GROUP_HEATER_CHANGE_STATE:
            pointEventLog = &eventLogTask_Logger_->config.buffEvents[LOGGER_EVENT_TYPE_FAIL_HEATER];
            break;
    }
    return pointEventLog;
}
/*
 * Author: Christogarm
 */

#include "loggerTask.h"

/* Macros */


/* Variable Privadas */
static void detectEvents(loggerTask_t* eventLogTask_Logger_,
    sysGroupEvents_t eventsBits);
static void saveDataEventInit(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc);
static void saveDataEventEnd(eventLogger_t* eventBuffDst, eventLogger_t* eventBuffSrc);
static eventLogger_t* selectBufferEventLogger(loggerTask_t* eventLogTask_Logger_,
    sysGroupEvents_t eventType);

/* Prototipos de las Funciones Privadas*/

/* Funciones Publicas */
void vLoggerTask(void* pvParameters) {
    if (pvParameters == NULL) {
        vTaskDelete(NULL);
        return;
    }

    loggerTask_t* loggerTask = (loggerTask_t*)pvParameters;
    logger_t* logger = (logger_t*)&loggerTask->logger;

    if (logger_init(logger, &logger->config) != LOGGER_STATUS_OK) {
        vTaskDelete(NULL);
        return;
    }

    for (;;)
    {
		if (rxDataQueue(logger->log, loggerTask->config.queueID, RTOS_TIMEOUT_MAX) != RTOS_OK) {
			vTaskDelete(NULL);
			return;
		}

        logger_addRegister(logger);
    }
}


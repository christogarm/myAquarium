/*
 * Author: Christogarm
 */

#ifndef RTOS_RESOURCES_H
#define RTOS_RESOURCES_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"              /* Para Queues (Colas) */
#include "semphr.h"             /* Para Mutexes y Semáforos */
#include "event_groups.h"       /* Para Event Groups */
#include "task.h"               /* Para Task Notifications */

/* Macros */
#define RTOS_TIMEOUT_MAX        portMAX_DELAY
#define RTOS_OK                 pdPASS
#define RTOS_ERROR              pdFALSE

/* Tipos de Datos */
typedef struct
{
    float value;
    float timeStamp;
    bool failure;
    bool enable;
}queueData_t;

typedef enum {
    READY_DATA_TEMP_WATER =     (1UL << 0),
    READY_DATA_TEMP_AMB =       (1UL << 1),
    READY_DATA_HEATER_POWER =   (1UL << 2),
    READY_DATA_ALL = (READY_DATA_TEMP_WATER | READY_DATA_TEMP_AMB | READY_DATA_HEATER_POWER ),
}readyData_t;

typedef struct
{
	float tempAmb;
	float tempWater;
	float heaterPower;
    readyData_t readyData;
}dataLogger_t;

typedef enum{
    QUEUE_ID_TEMP_WATER,
    QUEUE_ID_TEMP_AMB,
    QUEUE_ID_HEATER_POWER,
}queueID_t;

typedef enum{
    MUTEX_ID_CONFIGURATION,
    MUTEX_ID_LOGGER,
}mutexID_t;

/* Variables Publicas */
    /* Tareas */
extern TaskHandle_t xControlTask;

    /* Grupo de Eventos */
extern EventGroupHandle_t sysEventGroup;

    /* Quues */
extern QueueHandle_t waterTempQueue;
extern QueueHandle_t ambTempQueue;
extern QueueHandle_t heaterPowerQueue;

    /* Semaforos o mutexes */
extern SemaphoreHandle_t mutexConfig;
extern SemaphoreHandle_t mutexLogger;

    /* Datos Publicos */
extern dataLogger_t dataLogging;

    /* Bits de Sitema para el Grupo de Eventos */
typedef enum{
    SYS_EVENT_GROUP_POWER_ON                    = (1UL << 0),           // Eventos de Power On
    SYS_EVENT_GROUP_RECONFIG_CONTROL            = (1UL << 1),           // Eventos de Reconfiguración del control
    SYS_EVENT_GROUP_WATER_TEMP_CHANGE_STATE     = (1UL << 2),           // Evento de Falla del Sensor de Temperatura del Agua
    SYS_EVENT_GROUP_AMB_TEMP_CHANGE_STATE       = (1UL << 3),           // Evento de Falla del Sensor de Temperatura Ambiente
    SYS_EVENT_GROUP_HEATER_CHANGE_STATE         = (1UL << 4),           // Evento de Falla del Calentador (No hay cambios de Temperatura)
    SYS_EVENT_GROUP_ALL                         = (0b11111UL),
} sysGroupEvents_t;

/* Prototipos de las Funciones Publicas */
void rtos_resources_init(void);

    /* Manejo de Grupo de Eventos */
void rtos_setEvent(sysGroupEvents_t event_);
_Bool rtos_pollEvent(sysGroupEvents_t event_);
sysGroupEvents_t rtos_waitEvent(sysGroupEvents_t event_);
void rtos_clearEvent(sysGroupEvents_t event_);

    /* Manejo para compartir/enviar datos (Colas) */
void queueData_reset(queueData_t* dataSensor_);
uint32_t getDataQueue(queueData_t * dataSensor_, queueID_t id_, uint32_t waitTime_);
void setDataQueue(queueData_t * dataSensor_, queueID_t id_, uint32_t waitTime_);
void setDataQueue_overWrite(queueData_t * dataSensor_, queueID_t id_);

    /* Mutex */
uint32_t mutexTake(mutexID_t id_,  uint32_t waitTime_);
uint32_t mutexGive(mutexID_t id_);

    /* Time with Ticks */
float getTimeSeconds(void);
float getTimeMiliSeconds(void);

    /* Delay */
void rtos_delayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);
void rtos_delay(TickType_t delay_);


#endif /* RTOS_RESOURCES_H */

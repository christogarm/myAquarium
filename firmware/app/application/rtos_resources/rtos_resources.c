/*
 * Author: Christogarm
 */

#include "rtos_resources.h"


/* Tareas */
TaskHandle_t xControlTask = NULL;

/* Grupo de Eventos */
EventGroupHandle_t sysEventGroup;

/* Colas */
QueueHandle_t waterTempQueue;
QueueHandle_t ambTempQueue;
QueueHandle_t heaterPowerQueue;

/* Semaforos o mutexes */
SemaphoreHandle_t mutexConfig;
SemaphoreHandle_t mutexLogger;

/* Datos Publicos */
dataLogger_t dataLogging = {0};

/* Prototipos de las Funciones Privadas */
static QueueHandle_t selectQueue(queueID_t id_);
static SemaphoreHandle_t selectMutex(mutexID_t id_);


/* Funciones Publicas */
    /* Init Resources */
void rtos_resources_init(void){
    /* init Grupo de Eventos */
    sysEventGroup = xEventGroupCreate();
    if (sysEventGroup == NULL) 
        while (1);
    
    /* init Queues */
    waterTempQueue = xQueueCreate(1, sizeof(queueData_t));
    if (waterTempQueue == NULL) 
        while (1);
    
    ambTempQueue = xQueueCreate(1, sizeof(queueData_t));
    if (ambTempQueue == NULL) 
        while (1);

    heaterPowerQueue = xQueueCreate(1, sizeof(queueData_t));
    if (heaterPowerQueue == NULL) 
        while (1);
   
    /* init Semaforos o mutexes */
    mutexConfig = xSemaphoreCreateMutex();
    if (mutexConfig == NULL) 
        while (1);

    mutexLogger = xSemaphoreCreateMutex();
    if (mutexLogger == NULL)
        while (1);

}

    /* Event Drivers */
void rtos_setEvent(sysGroupEvents_t event_){
    xEventGroupSetBits(sysEventGroup, event_);
}

_Bool rtos_pollEvent(sysGroupEvents_t event_){
    /* Revisamos si hay un evento */
    EventBits_t bits_ev = xEventGroupWaitBits(
        sysEventGroup,
        event_,                         // Evento a Revisar
        pdFALSE,                        // No Borra el bit al salir.
        pdFALSE,                        // Espera cualquiera de los bits.
        0                               // No realiza la espera
    );
    /* Entregamos el bit */
    return ((bits_ev & event_) != 0);
}

sysGroupEvents_t rtos_waitEvent(sysGroupEvents_t event_){
    /* Esperamos el Evento */
    return  (sysGroupEvents_t) xEventGroupWaitBits(
        sysEventGroup,                          // Manejador del grupo de eventos
        event_,                                 // Máscara con los bits a esperar (ej. 0x05)
        pdFALSE,                                // pdTRUE para borrar los bits al salir, pdFALSE para dejarlos
        pdFALSE,                                 // pdTRUE = AND (todos los bits), pdFALSE = OR (cualquiera)
        portMAX_DELAY                           // Tiempo máximo de espera en ticks (o portMAX_DELAY)
    );
}

void rtos_clearEvent(sysGroupEvents_t event_){
    xEventGroupClearBits(sysEventGroup, event_);
}

    /* Queue Drivers */
void queueData_reset(queueData_t* dataSensor_){
    if(dataSensor_ == NULL)
        return;

    dataSensor_->value = 0.0f;
    dataSensor_->timeStamp = 0.0f;
    dataSensor_->failure = false;
    dataSensor_->enable = false;
}

uint32_t getDataQueue(queueData_t * dataSensor_, queueID_t id_, uint32_t waitTime_){
    if(dataSensor_ == NULL)
        return RTOS_ERROR;

    QueueHandle_t queue_ = selectQueue(id_);
    if(queue_ == NULL)
        return RTOS_ERROR   ;

    return xQueueReceive(queue_, dataSensor_, waitTime_);
}

void setDataQueue(queueData_t * dataSensor_, queueID_t id_, uint32_t waitTime_){
    if(dataSensor_ == NULL)
        return;

    QueueHandle_t queue_ = selectQueue(id_);
    if(queue_ == NULL)
        return;
    
    xQueueSend(queue_, dataSensor_, waitTime_);
}

void setDataQueue_overWrite(queueData_t * dataSensor_, queueID_t id_){
    if(dataSensor_ == NULL)
        return;

    QueueHandle_t queue_ = selectQueue(id_);
    if(queue_ == NULL)
        return;
    
    xQueueOverwrite(queue_, dataSensor_);
}

    /* Mutex Drivers */ 
uint32_t mutexTake(mutexID_t id_,  uint32_t waitTime_){
    SemaphoreHandle_t mutex_ = selectMutex(id_);
    if(mutex_ == NULL)
        return RTOS_ERROR;

    return xSemaphoreTake(mutex_, waitTime_);
}

uint32_t mutexGive(mutexID_t id_){
    SemaphoreHandle_t mutex_ = selectMutex(id_);
    if(mutex_ == NULL)
        return RTOS_ERROR;

    return xSemaphoreGive(mutex_);
}

    /* Time with Ticks */
float getTimeSeconds(void){
    return getTimeMiliSeconds() / 1000;
}

float getTimeMiliSeconds(void){
    TickType_t xInicio = xTaskGetTickCount();
    return (float) (xInicio * portTICK_PERIOD_MS);
}

/* Delay */
void rtos_delayUntil(TickType_t* pxPreviousWakeTime, TickType_t xTimeIncrement){
    vTaskDelayUntil(pxPreviousWakeTime, xTimeIncrement);
}

void rtos_delay(TickType_t delay_){
    vTaskDelay(delay_);
}

/* Funciones Privadas */
static QueueHandle_t selectQueue(queueID_t id_){
    QueueHandle_t queue_;
    switch (id_){
    case QUEUE_ID_TEMP_WATER:
        queue_ = waterTempQueue;
        break;

    case QUEUE_ID_TEMP_AMB:
        queue_ = ambTempQueue;
        break;

    case QUEUE_ID_HEATER_POWER:
        queue_ = heaterPowerQueue;
        break;    

    default:
        return NULL;
        break;
    }
    return queue_;
} 

static SemaphoreHandle_t selectMutex(mutexID_t id_){
    SemaphoreHandle_t mutex_;
    switch (id_){
    case MUTEX_ID_CONFIGURATION:
        mutex_ = mutexConfig;
        break;
    case MUTEX_ID_LOGGER:
        mutex_ = mutexLogger;
        break;
    default:
        return NULL;
        break;
    }
    return mutex_;
}


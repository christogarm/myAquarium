/*
 * Author: Christogarm
 */

#ifndef LOGGER_IF_H
#define LOGGER_IF_H

#define	LOGGER_READ_MAX_LINEA_CHAR 256
#define LOGGER_SIZE_COLUMNS_EVENT	9
#define LOGGER_SIZE_COLUMNS_TIME	4

 /* Enum para los tipos de eventos a loggear */
typedef enum {
	LOGGER_STATUS_OK,
	LOGGER_STATUS_NULL_POINTER,
	LOGGER_STATUS_ERROR,
}logger_status_t;

typedef enum {
	LOGGER_TYPE_EVENT,
	LOGGER_TYPE_TIME,
}logger_type_t;

typedef enum {
	LOGGER_EVENT_TYPE_FAIL_AMB_TEMP = 0,
	LOGGER_EVENT_TYPE_FAIL_WATER_TEMP,
	LOGGER_EVENT_TYPE_FAIL_HEATER,
	LOGGER_EVENT_TYPE_FAIL_RECONFIGURATION,
	LOGGER_EVENT_TYPE_FAIL_POWER_ON,
	LOGGER_EVENT_TYPE_NUM_EVENTS,
} logger_eventType_t;

/* Tipos de Datos */
	/* Estructura de Configuracion para todo el Sistema */
typedef struct
{
	uint32_t timeUNIX;
	float tempAmb;
	float tempWater;
	float heaterPower;
} timeLogger_t;

typedef struct
{
	uint32_t timeUNIX_init;
	uint32_t timeUNIX_end;
	logger_eventType_t type;
	float tempAmb_init;
	float tempAmb_end;
	float tempWater_init;
	float tempWater_end;
	float heaterPower_init;
	float heaterPower_end;
} eventLogger_t;

 /* Callbacks */
typedef logger_status_t(*logger_init_f)(void* instance, const void* config);
typedef logger_status_t(*logger_addRegister_f)(void* instance, void* logRegister);
typedef logger_status_t(*logger_readRegister_f)(void* instance, void* logRegister, uint32_t numRegister);

#endif /* LOGGER_IF_H */
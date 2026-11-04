/*
 * Author: Christogarm
 */

#ifndef TIME_IF_H
#define TIME_IF_H

#include <stdint.h>

 /* Enum para los tipos de eventos a loggear */
typedef enum {
	TIME_STATUS_OK,
	TIME_STATUS_NULL_POINTER,
	TIME_STATUS_ERROR,
}time_status_t;


/* Tipos de Datos */
	/* Estructura de Tiempo para todo el Sistema */
typedef struct
{
	uint32_t timeUNIX;
	uint8_t year;				// Año (0-99)
	uint8_t month;				// Mes del año (1-12)
	uint8_t week;				// Semana del año (1-52)
	uint8_t weekDay;			// Dia de la semana (0-6)
	uint8_t day;				// Dia del mes (1-31)
	uint8_t hour;				// Hora del dia (0-23)
	uint8_t minute;				// Minuto de la hora (0-59)
	uint8_t second;				// Segundo del minuto (0-59)
} timeData_t;

 /* Callbacks */
typedef time_status_t(*time_init_f)(void* instance_, const void* config_);
typedef time_status_t(*time_getTime_f)(void* instance, timeData_t* timeData);

#endif /* TIME_IF_H */
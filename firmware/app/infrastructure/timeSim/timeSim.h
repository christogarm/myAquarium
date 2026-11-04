/*
 * Author: Christogarm
 *
 */

#ifndef TIMESIM_H
#define TIMESIM_H

#include <stdint.h>
#include <stdbool.h>

#include "time_if.h"

/* Limites del offset UTC aceptado (segundos): UTC-12:00 .. UTC+14:00 */
#define TIME_SIM_UTC_OFFSET_MIN_S   (-43200)
#define TIME_SIM_UTC_OFFSET_MAX_S   (50400)

/* Configuracion (inmutable tras la inicializacion) */
typedef struct
{
	int32_t  utc_offset_seconds;	/* Offset de zona horaria respecto a UTC */
	bool     use_start_time;		/* true: el reloj simulado arranca en start_unix */
	uint32_t start_unix;			/* Tiempo UNIX (UTC) inicial si use_start_time == true */
} time_sim_config_t;

/* Estado interno (no modificar desde fuera del modulo) */
typedef struct
{
	bool    initialized;
	int64_t start_delta_seconds;	/* Diferencia entre tiempo simulado y tiempo del sistema */
} time_sim_state_t;

/* Contexto de instancia: este es el 'instance' que reciben los callbacks */
typedef struct
{
	time_sim_config_t config;
	time_sim_state_t  state;
} time_sim_t;

/*
 * Compatible con time_init_f.
 * instance_ : time_sim_t* (memoria provista por el llamador)
 * config_   : const time_sim_config_t*
 */
time_status_t time_sim_init(void* instance_, const void* config_);

/*
 * Compatible con time_getTime_f.
 * instance  : time_sim_t* previamente inicializada
 * timeData  : estructura de salida
 *
 * timeUNIX contiene el tiempo UNIX en UTC; el resto de los campos
 * corresponden a la hora local (UTC + utc_offset_seconds).
 */
time_status_t time_sim_get_time(void* instance, timeData_t* timeData);

#endif /* TIMESIM_H */

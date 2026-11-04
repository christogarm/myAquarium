/*
 * Logger tiene la funcion de Grabar Eventos y Datos en dos archivos distintos
 * DataLog.csv      ->  Se graban datos de interes
 * EventosLog.csv   ->  Se graban Eventos como errores, advertencias o informacion
 */

#ifndef LOGGER_FILE_H_
#define LOGGER_FILE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "logger_if.h"

typedef struct
{
    const char* filename;      // Nombre del Archivo
    char** pEncabezado;        // Apuntador para escribir el encabezado de la tabla de Datos
} logger_file_config_t;

typedef struct
{
	logger_file_config_t configEvent;
	logger_file_config_t configTime;
	uint32_t countEventRegisters;
	uint32_t countTimeRegisters;
    FILE* fileLoggerEvent;          // Archivo
    FILE* fileLoggerTime;           // Archivo

	timeLogger_t timeLog;
	eventLogger_t eventLog;
	bool init;
} logger_file_t;


logger_status_t logger_file_init(void* logger_, const void* configEvent_, const void* configTime_);
logger_status_t logger_file_addEventRegister(void* logger_, eventLogger_t* eventLog);
logger_status_t logger_file_addTimeRegister(void* logger_, timeLogger_t* timeLog);
void* logger_file_readTimeLogger(void* logger_, uint32_t numRegister);
void* logger_file_readEventLogger(void* logger_, uint32_t numRegister);

#endif
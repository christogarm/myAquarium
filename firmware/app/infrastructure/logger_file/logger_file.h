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
	logger_type_t type;
	uint8_t numColumns;
} logger_file_config_t;

typedef struct
{
	logger_file_config_t config;
	uint32_t countRegisters;

	bool init;
} logger_file_t;


logger_status_t logger_file_init(void* instance, const void* config_);
logger_status_t logger_file_addRegister(void* instance, void* log);
logger_status_t logger_file_readRegister(void* instance, void* logRegister, uint32_t numRegister);

#endif
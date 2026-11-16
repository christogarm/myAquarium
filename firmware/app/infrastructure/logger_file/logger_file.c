/*
 * Author: Christogarm
 */

#include "logger_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prototipos de las funciones Privadas */
static logger_status_t logger_file_printEncabezado(logger_file_config_t* config_);         // Imprime el encabezado
static logger_status_t logger_file_addEventRegister(logger_file_t* loggerFile, eventLogger_t* eventLog);
static logger_status_t logger_file_addTimeRegister(void* instance, timeLogger_t* timeLog);
static logger_status_t logger_file_readTimeRegister(logger_file_t* loggerFile, timeLogger_t* logRegister, uint32_t numRegister);
static logger_status_t logger_file_readEventRegister(logger_file_t* loggerFile, eventLogger_t* logRegister, uint32_t numRegister);


/* Fucniones Publicas */
logger_status_t logger_file_init(void* instance, const void* config_) {
	if(instance == NULL || config_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;

	logger_file_t * loggerFile = (logger_file_t*)instance;
	logger_file_config_t* loggerFileConfig = (logger_file_config_t*)config_;
    
	loggerFile->init = false;
	if (loggerFileConfig->numColumns == 0)
		return LOGGER_STATUS_ERROR;
    if (loggerFile->config.type != LOGGER_TYPE_TIME &&
        loggerFile->config.type != LOGGER_TYPE_EVENT)
		return LOGGER_STATUS_ERROR;
	if (loggerFileConfig->pEncabezado == NULL || loggerFileConfig->filename == NULL )
		return LOGGER_STATUS_NULL_POINTER;

    if(logger_file_printEncabezado(loggerFileConfig) != LOGGER_STATUS_OK)
        return LOGGER_STATUS_ERROR;

    loggerFile->config = *loggerFileConfig;
	loggerFile->init = true;

	return LOGGER_STATUS_OK;
}

logger_status_t logger_file_addRegister(void* instance, void* log) {
    if (instance == NULL || log == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    logger_file_t* loggerFile = (logger_file_t*)instance;
    if(loggerFile->init == false)
        return LOGGER_STATUS_ERROR;

    if (loggerFile->config.type == LOGGER_TYPE_TIME)
        logger_file_addTimeRegister(loggerFile, (timeLogger_t*)log);
    else if (loggerFile->config.type == LOGGER_TYPE_EVENT) 
        logger_file_addEventRegister(loggerFile, (eventLogger_t*)log);
    else 
        return LOGGER_STATUS_ERROR;

    return LOGGER_STATUS_OK;
}

logger_status_t logger_file_readRegister(void* instance, void* logRegister, uint32_t numRegister) {
    if (instance == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    logger_file_t* loggerFile = (logger_file_t*)instance;
    if (loggerFile->init == false)
        return LOGGER_STATUS_ERROR;

    if (numRegister >= loggerFile->countRegisters)
        return LOGGER_STATUS_ERROR;

    logger_status_t status = LOGGER_STATUS_ERROR;
    if (loggerFile->config.type == LOGGER_TYPE_TIME)
        status = logger_file_readTimeRegister(loggerFile, (timeLogger_t*)logRegister, numRegister);
    else if (loggerFile->config.type == LOGGER_TYPE_EVENT)
        status = logger_file_readEventRegister(loggerFile, (eventLogger_t*)logRegister, numRegister);

    return status;
}


/* Funciones Privadas */
static logger_status_t logger_file_printEncabezado(logger_file_config_t* config_)
{
    FILE* fileLogger = fopen(config_->filename, "w"); // Abrimos el verificar que todo esta correcto
    if (fileLogger == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    size_t sizeEncabezados = 0;

    /* Calculamos la cantidad de caracteres que son en total */
    for (size_t i = 0; i < config_->numColumns; i++)
        sizeEncabezados += strlen(config_->pEncabezado[i]);

    char* str = (char*)malloc(sizeEncabezados + (config_->numColumns - 1 + 1) + 1 + 1); // Sumamos el tamano, debido a que colocaremos comas entre palabra, ademas sumamos un elemento extra para que tenggamos un caracter \0 y un salto de linea

    if (str == NULL) // Si tenemmos espacio de memoria?
        return LOGGER_STATUS_NULL_POINTER;

    str[0] = '\0';          // inicializar string

    // concatenamos el resto de datos con comas
    for (int i = 0; i < config_->numColumns; i++)
    {
        strcat(str, config_->pEncabezado[i]);
        strcat(str, ",");
    }
    strcat(str, "\n");

    fprintf(fileLogger, "%s", str);
    free(str);
    fclose(fileLogger);

    return LOGGER_STATUS_OK;
}

static logger_status_t logger_file_addEventRegister(logger_file_t* loggerFile, eventLogger_t* eventLog) {
    FILE* fileLoggerEvent = fopen(loggerFile->config.filename, "a");
    if (fileLoggerEvent == NULL)
        return LOGGER_STATUS_ERROR;

    fprintf(fileLoggerEvent, "%d", eventLog->timeUNIX_init);
    fprintf(fileLoggerEvent, ",%d", eventLog->timeUNIX_end);
    fprintf(fileLoggerEvent, ",%d", eventLog->type);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->tempAmb_init);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->tempAmb_end);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->tempWater_init);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->tempWater_end);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->heaterPower_init);
    fprintf(fileLoggerEvent, ",%.3f", eventLog->heaterPower_end);
    fprintf(fileLoggerEvent, "\n");

    fclose(fileLoggerEvent);

    loggerFile->countRegisters++;

    return LOGGER_STATUS_OK;
}

static logger_status_t logger_file_addTimeRegister(logger_file_t* loggerFile, timeLogger_t* timeLog) {
    FILE* fileLoggerTime = fopen(loggerFile->config.filename, "a");
    if (fileLoggerTime == NULL)
        return LOGGER_STATUS_ERROR;

    fprintf(fileLoggerTime, "%d", timeLog->timeUNIX);
    fprintf(fileLoggerTime, ",%.3f", timeLog->tempAmb);
    fprintf(fileLoggerTime, ",%.3f", timeLog->tempWater);
    fprintf(fileLoggerTime, ",%.3f", timeLog->heaterPower);

    fprintf(fileLoggerTime, "\n");
    fclose(fileLoggerTime);

    loggerFile->countRegisters++;

    return LOGGER_STATUS_OK;
}

static logger_status_t logger_file_readTimeRegister(logger_file_t* loggerFile, timeLogger_t* logRegister, uint32_t numRegister) {
    char linea[LOGGER_READ_MAX_LINEA_CHAR];

    FILE* fileLoggerTime = fopen(loggerFile->config.filename, "r");
    if (fileLoggerTime == NULL)
        return LOGGER_STATUS_ERROR;

    /* Leer una linea del archivo CSV */
	fgets(linea, sizeof(linea), fileLoggerTime);    // Leemos el encabezado
    for(uint32_t i = 0; i < numRegister; i++)
    {
        if (fgets(linea, sizeof(linea), fileLoggerTime) == NULL)
        {
            fclose(fileLoggerTime);
            return LOGGER_STATUS_ERROR;
        }
    }
    fclose(fileLoggerTime);

    char token = strtok(linea, ",");
    char fin;

    logRegister->timeUNIX = strtoul(token, &fin, 10);
    logRegister->tempAmb = strtof(token, &fin);
    logRegister->tempWater = strtof(token, &fin);
    logRegister->heaterPower = strtof(token, &fin);

    return LOGGER_STATUS_OK;
}

static logger_status_t logger_file_readEventRegister(logger_file_t* loggerFile, eventLogger_t* logRegister, uint32_t numRegister) {
    char linea[LOGGER_READ_MAX_LINEA_CHAR];

    FILE* fileLoggerEvent = fopen(loggerFile->config.filename, "r");
    if (fileLoggerEvent == NULL)
        return LOGGER_STATUS_ERROR;

    /* Leer una linea del archivo CSV */
    fgets(linea, sizeof(linea), fileLoggerEvent);    // Leemos el encabezado
    for (uint32_t i = 0; i < numRegister; i++)
    {
        if (fgets(linea, sizeof(linea), fileLoggerEvent) == NULL)
        {
            fclose(fileLoggerEvent);
            return LOGGER_STATUS_ERROR;
        }
    }

    char token = strtok(linea, ",");
    char fin;

    logRegister->timeUNIX_init = strtoul(token, &fin, 10);
    logRegister->timeUNIX_end = strtoul(token, &fin, 10);
    logRegister->type = strtoul(token, &fin, 10);
    logRegister->tempAmb_init = strtof(token, &fin);
    logRegister->tempAmb_end = strtof(token, &fin);
    logRegister->tempWater_init = strtof(token, &fin);
    logRegister->tempWater_end = strtof(token, &fin);
    logRegister->heaterPower_init = strtof(token, &fin);
    logRegister->heaterPower_end = strtof(token, &fin);

    return LOGGER_STATUS_OK;
}


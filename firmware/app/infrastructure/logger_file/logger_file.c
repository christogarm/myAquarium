/*
 * Author: Christogarm
 */

#include "logger_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prototipos de las funciones Privadas */
static logger_status_t printEncabezado(FILE* fileLogger, logger_file_config_t* configLogger_, uint8_t numColumnas);         // Imprime el encabezado
static logger_status_t openFileLogger(FILE** fileLogger, logger_file_config_t* configLogger_);          // Abre el archivo para escribir
static logger_status_t closeFileLogger(FILE** fileLogger);         // Cierra el archivo


/* Fucniones Publicas */
logger_status_t logger_file_init(void* logger_, const void* configEvent_, const void* configTime_) {
	if(logger_ == NULL || configEvent_ == NULL || configTime_ == NULL)
		return LOGGER_STATUS_NULL_POINTER;

	logger_file_t * loggerFile = (logger_file_t*)logger_;
	logger_file_config_t* loggerFileConfigEvent = (logger_file_config_t*)configEvent_;
	logger_file_config_t* loggerFileConfigTime = (logger_file_config_t*)configTime_;

	if (loggerFileConfigEvent->pEncabezado == NULL || loggerFileConfigTime->pEncabezado == NULL)
		return LOGGER_STATUS_NULL_POINTER;

    if(printEncabezado(loggerFile->fileLoggerEvent,loggerFileConfigEvent, LOGGER_SIZE_COLUMNS_EVENT) != LOGGER_STATUS_OK ||
        printEncabezado(loggerFile->fileLoggerTime, loggerFileConfigTime, LOGGER_SIZE_COLUMNS_TIME ) != LOGGER_STATUS_OK)
        return LOGGER_STATUS_ERROR;

	loggerFile->configEvent = *loggerFileConfigEvent;
	loggerFile->configTime = *loggerFileConfigTime;
	loggerFile->init = true;

	return LOGGER_STATUS_OK;
}

logger_status_t logger_file_addEventRegister(void* logger_, eventLogger_t* eventLog) {
    if (logger_ == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    logger_file_t* loggerFile = (logger_file_t*)logger_;

    if(openFileLogger(&loggerFile->fileLoggerEvent, &loggerFile->configEvent) != LOGGER_STATUS_OK)
        return LOGGER_STATUS_ERROR;
    fprintf(loggerFile->fileLoggerEvent, "%d", eventLog->timeUNIX_init);
    fprintf(loggerFile->fileLoggerEvent, ",%d", eventLog->timeUNIX_end);
    fprintf(loggerFile->fileLoggerEvent, ",%d", eventLog->type);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->tempAmb_init);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->tempAmb_end);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->tempWater_init);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->tempWater_end);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->heaterPower_init);
    fprintf(loggerFile->fileLoggerEvent, ",%.3f", eventLog->heaterPower_end);

    fprintf(loggerFile->fileLoggerEvent, "\n");
    closeFileLogger(&loggerFile->fileLoggerEvent);

    loggerFile->countEventRegisters++;
    return LOGGER_STATUS_OK;
}

logger_status_t logger_file_addTimeRegister(void* logger_, timeLogger_t* timeLog){
    if (logger_ == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    logger_file_t* loggerFile = (logger_file_t*)logger_;

    if (openFileLogger(&loggerFile->fileLoggerTime, &loggerFile->configTime) != LOGGER_STATUS_OK)
        return LOGGER_STATUS_ERROR;
    fprintf(loggerFile->fileLoggerTime, "%d", timeLog->timeUNIX);
    fprintf(loggerFile->fileLoggerTime, ",%.3f", timeLog->tempAmb);
    fprintf(loggerFile->fileLoggerTime, ",%.3f", timeLog->tempWater);
    fprintf(loggerFile->fileLoggerTime, ",%.3f", timeLog->heaterPower);

    fprintf(loggerFile->fileLoggerTime, "\n");
    closeFileLogger(&loggerFile->fileLoggerTime);

    loggerFile->countTimeRegisters++;

    return LOGGER_STATUS_OK;
}

void* logger_file_readTimeLogger(void* logger_, uint32_t numRegister) {
    if (logger_ == NULL)
        return NULL;

    logger_file_t* loggerFile = (logger_file_t*)logger_;
    char linea[LOGGER_READ_MAX_LINEA_CHAR];

    if (numRegister >= loggerFile->countTimeRegisters)
        return NULL;

    loggerFile->fileLoggerTime = fopen(loggerFile->configTime.filename, "r");

    if (loggerFile->fileLoggerTime == NULL)
        return NULL;

    /* Leer una linea del archivo CSV */
    if (fgets(linea, sizeof(linea), loggerFile->fileLoggerTime) == NULL)
    {
        fclose(loggerFile->fileLoggerTime);
        return NULL;
    }

    char token = strtok(linea, ",");
    char fin;

    loggerFile->timeLog.timeUNIX = strtoul(token, &fin, 10);
    loggerFile->timeLog.tempAmb = strtof(token, &fin);
    loggerFile->timeLog.tempWater = strtof(token, &fin);
    loggerFile->timeLog.heaterPower = strtof(token, &fin);

    return &loggerFile->timeLog;
}

void* logger_file_readEventLogger(void* logger_, uint32_t numRegister) {
    if (logger_ == NULL)
        return NULL;

    logger_file_t* loggerFile = (logger_file_t*)logger_;
    char linea[LOGGER_READ_MAX_LINEA_CHAR];

    if (numRegister >= loggerFile->countEventRegisters)
        return NULL;

    loggerFile->fileLoggerEvent = fopen(loggerFile->configEvent.filename, "r");

    if (loggerFile->fileLoggerEvent == NULL)
        return NULL;

    /* Leer una linea del archivo CSV */
    if (fgets(linea, sizeof(linea), loggerFile->fileLoggerEvent) == NULL)
    {
        fclose(loggerFile->fileLoggerEvent);
        return NULL;
    }

    char token = strtok(linea, ",");
    char fin;

    loggerFile->eventLog.timeUNIX_init = strtoul(token, &fin, 10);
    loggerFile->eventLog.timeUNIX_end = strtoul(token, &fin, 10);
    loggerFile->eventLog.type = strtoul(token, &fin, 10);
    loggerFile->eventLog.tempAmb_init = strtof(token, &fin);
    loggerFile->eventLog.tempAmb_end = strtof(token, &fin);
    loggerFile->eventLog.tempWater_init = strtof(token, &fin);
    loggerFile->eventLog.tempWater_end = strtof(token, &fin);
    loggerFile->eventLog.heaterPower_init = strtof(token, &fin);
    loggerFile->eventLog.heaterPower_end = strtof(token, &fin);

    return &loggerFile->eventLog;
}

/* Funciones Privadas */
static logger_status_t printEncabezado(FILE* fileLogger, logger_file_config_t* configLogger_, uint8_t numColumnas)
{
    fileLogger = fopen(configLogger_->filename, "w"); // Abrimos el verificar que todo esta correcto
    if (fileLogger == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    int sizeEncabezados = 0;

    /* Calculamos la cantidad de caracteres que son en total */
    for (size_t i = 0; i < numColumnas; i++)
        sizeEncabezados += strlen(configLogger_->pEncabezado[i]);

    char* str = (char*)malloc(sizeEncabezados + (numColumnas - 1 + 1) + 1 + 1); // Sumamos el tamano, debido a que colocaremos comas entre palabra, ademas sumamos un elemento extra para que tenggamos un caracter \0 y un salto de linea

    if (str == NULL) // Si tenemmos espacio de memoria?
        return LOGGER_STATUS_NULL_POINTER;

    str[0] = '\0';          // inicializar string

    // concatenamos el resto de datos con comas
    for (int i = 0; i < numColumnas; i++)
    {
        strcat(str, configLogger_->pEncabezado[i]);
        strcat(str, ",");
    }
    strcat(str, "\n");

    fprintf(fileLogger, "%s", str);
    free(str);
    fclose(fileLogger);

    return LOGGER_STATUS_OK;
}

static logger_status_t openFileLogger(FILE** fileLogger, logger_file_config_t* configLogger_) {
    *fileLogger = fopen(configLogger_->filename, "a");
    if (*fileLogger == NULL)
        return LOGGER_STATUS_NULL_POINTER;

    return LOGGER_STATUS_OK;
}
static logger_status_t closeFileLogger(FILE** fileLogger) {
    fclose(*fileLogger);
	return LOGGER_STATUS_OK;
}


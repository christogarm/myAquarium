/*
 * Author: Christogarm
 */

#include "config_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

/* Macros */


 /* Valores predeterminados */
static const dataConfig_t CONFIG_BK_DEFAULT = {
    /* Configuracion del Control PID */
    .setpoint = CONFIG_BK_SETPOINT,
    .deadband = CONFIG_BK_DEADBAND,
    .rateLimit = CONFIG_BK_RATELIMIT,
    .initOutput = CONFIG_BK_INITOUTPUT,
    .kp = CONFIG_BK_KP,
    .ki = CONFIG_BK_KI,
    .kd = CONFIG_BK_KD,

    /* Configuracion del Calentador */
    .minPowerHeater = CONFIG_BK_MIN_POWER_HEATER,
    .maxPowerHeater = CONFIG_BK_MAX_POWER_HEATER,

    /* Configuracion del Sensor de Temperatura del Agua */
    .maxWaterTemperature = CONFIG_BK_MAX_WATER_TEMP,
    .minWaterTemperature = CONFIG_BK_MIN_WATER_TEMP,
    .difWaterTemperature = CONFIG_BK_DIF_WATER_TEMP,
    .timeShowFailWaterTemp = CONFIG_BK_TIME_SHOW_FAIL_WATER_TEMP,

    /* Configuracion del Sensor de Temperatura Ambiente */
    .maxAmbientTemperature = CONFIG_BK_MAX_AMBIENT_TEMP,
    .minAmbientTemperature = CONFIG_BK_MIN_AMBIENT_TEMP,
    .difAmbientTemperature = CONFIG_BK_DIF_AMBIENT_TEMP,
    .timeShowFailAmbientTemp = CONFIG_BK_TIME_SHOW_FAIL_AMBIENT_TEMP,

    /* Configuracion del Logger */
    .timeAddNewRegisterLogger = CONFIG_BK_TIME_ADD_NEW_REG_LOG,
    .enabled_events_mask = CONFIG_BK_ENABLE_EVENT_MASK,

    /* Banderas de Configuracion */
    .flags = CONFIG_BK_FLAGS
};


/* Prototipos de las funciones Privadas */
static char* trim(char* str);
static int parseCSV(char* line, char fields[][MAX_FIELD_LEN], size_t maxFields);
static bool readHeader(config_file_t* configFile_);
static bool validateHeader(config_file_t* configFile_);
static bool writeDefaultConfig(config_file_t* configFile_);
static bool parseFloat(const char* text, float* value);
static bool parseUint(const char* text, uint32_t* value);
static bool readValues(config_file_t* configFile_);

/* Fucniones Publicas */
config_status_t config_file_init(void* instance_, const void* config_){
    if(instance_ == NULL || config_ == NULL)
		return CONFIG_STATUS_NULL_POINTER;

    config_file_t* configFile = (config_file_t*)instance_;
    config_file_config_t* configFile_config = (config_file_config_t*)config_;
    configFile->config = *configFile_config;

    if(configFile_config->filename == NULL)
        return CONFIG_STATUS_NULL_POINTER;
	
    configFile->file = fopen(configFile_config->filename, "r");
    if (configFile->file == NULL) {
        if (!writeDefaultConfig(configFile))
            return CONFIG_STATUS_ERROR;
        
        configFile->file = fopen(configFile_config->filename, "r");

        if (configFile->file == NULL)
            return CONFIG_STATUS_ERROR;
    }
    
    bool headerOK = readHeader(configFile) &&
        validateHeader(configFile);

    if (!headerOK) {
        fclose(configFile->file);

        /* Reescribir con nombres y valores correctos */
        if (!writeDefaultConfig(configFile))
            return false;

        /* Cargar la configuración recién creada */
        configFile->file = fopen(configFile_config->filename, "r");

        if (configFile->file == NULL)
            return CONFIG_STATUS_ERROR;

        headerOK = readHeader(configFile) &&
            validateHeader(configFile);

        if (!headerOK) {
            fclose(configFile->file);
            return CONFIG_STATUS_ERROR;
        }
    }

    fclose(configFile->file);

    if(!readValues(configFile))
        return CONFIG_STATUS_ERROR;

    configFile->init = true;

	return CONFIG_STATUS_OK;
}

config_status_t config_file_write(void* instance_, dataConfig_t* configData_) {
    if (instance_ == NULL || configData_ == NULL)
        return CONFIG_STATUS_NULL_POINTER;

    config_file_t* configFile_ = (config_file_t*)instance_;

    if(!configFile_->init)
        return CONFIG_STATUS_ERROR;

    configFile_->file = fopen(configFile_->config.filename, "w");

    if (configFile_->file == NULL)
        return CONFIG_STATUS_ERROR;

    /* Impresion de Encabezado */
    for (size_t i = 0; i < CONFIG_NUM_PARAMETERS; i++) {
        fprintf(configFile_->file, "%s%s",
            configFile_->config.pEncabezado[i],
            (i + 1 < CONFIG_NUM_PARAMETERS) ? "," : "\n");
    }

    /* Impresion de Datos numericos */
    fprintf(configFile_->file, "%.3f", configData_->setpoint);
    fprintf(configFile_->file, ",%.3f", configData_->deadband);
    fprintf(configFile_->file, ",%.3f", configData_->rateLimit);
    fprintf(configFile_->file, ",%.3f", configData_->initOutput);
    fprintf(configFile_->file, ",%.3f", configData_->kp);
    fprintf(configFile_->file, ",%.3f", configData_->ki);
    fprintf(configFile_->file, ",%.3f", configData_->kd);

    fprintf(configFile_->file, ",%.3f", configData_->minPowerHeater);
    fprintf(configFile_->file, ",%.3f", configData_->maxPowerHeater);

    fprintf(configFile_->file, ",%.3f", configData_->maxWaterTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->minWaterTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->difWaterTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->timeShowFailWaterTemp);

    fprintf(configFile_->file, ",%.3f", configData_->maxAmbientTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->minAmbientTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->difAmbientTemperature);
    fprintf(configFile_->file, ",%.3f", configData_->timeShowFailAmbientTemp);

    fprintf(configFile_->file, ",%.3f", configData_->timeAddNewRegisterLogger);
    fprintf(configFile_->file, ",%d", configData_->enabled_events_mask);

    fprintf(configFile_->file, ",%d", configData_->flags);

    fprintf(configFile_->file, "\n");
    fclose(configFile_->file);

    return CONFIG_STATUS_OK;
}

config_status_t config_file_read(void* instance_, dataConfig_t* configData_) {
    if (instance_ == NULL || configData_ == NULL)
        return CONFIG_STATUS_NULL_POINTER;

    config_file_t* configFile_ = (config_file_t*)instance_;

    if (!configFile_->init)
        return CONFIG_STATUS_ERROR;

    configFile_->file = fopen(configFile_->config.filename, "r");
    /* Leemos el Header */
    char line[CONFIG_READ_MAX_LINEA_CHAR];
    if (fgets(line, sizeof(line), configFile_->file) == NULL)
        return CONFIG_STATUS_ERROR;

    /* Leemos los Valores */
    if (fgets(line, sizeof(line), configFile_->file) == NULL)
        return CONFIG_STATUS_ERROR;

    if (strchr(line, '\n') == NULL && !feof(configFile_->file))
        return CONFIG_STATUS_ERROR;

    int count = parseCSV(line, configFile_->values, CONFIG_NUM_PARAMETERS);

    if (count != CONFIG_NUM_PARAMETERS)
        return CONFIG_STATUS_ERROR;

    fclose(configFile_->file);

    bool verifyFlag = 1;

    verifyFlag = parseFloat(configFile_->values[0], &configData_->setpoint);
    parseFloat(configFile_->values[1], &configData_->deadband);
    parseFloat(configFile_->values[2], &configData_->rateLimit);
    parseFloat(configFile_->values[3], &configData_->initOutput);
    parseFloat(configFile_->values[4], &configData_->kp);
    parseFloat(configFile_->values[5], &configData_->ki);
    parseFloat(configFile_->values[6], &configData_->kd);

    parseFloat(configFile_->values[7], &configData_->minPowerHeater);
    parseFloat(configFile_->values[8], &configData_->maxPowerHeater);

    parseFloat(configFile_->values[9], &configData_->maxWaterTemperature);
    parseFloat(configFile_->values[10], &configData_->minWaterTemperature);
    parseFloat(configFile_->values[11], &configData_->difWaterTemperature);
    parseFloat(configFile_->values[12], &configData_->timeShowFailWaterTemp);

    parseFloat(configFile_->values[13], &configData_->maxAmbientTemperature);
    parseFloat(configFile_->values[14], &configData_->minAmbientTemperature);
    parseFloat(configFile_->values[15], &configData_->difAmbientTemperature);
    parseFloat(configFile_->values[16], &configData_->timeShowFailAmbientTemp);

    parseFloat(configFile_->values[17], &configData_->timeAddNewRegisterLogger);
    parseUint(configFile_->values[18], &configData_->enabled_events_mask);

    parseUint(configFile_->values[19], &configData_->flags);

    return CONFIG_STATUS_OK;
}

/* Funciones Privadas */

/* Elimina espacios al inicio y al final */
static char* trim(char* str)
{
    while (*str == ' ' || *str == '\t')
        str++;

    size_t len = strlen(str);

    while (len > 0 &&
        (str[len - 1] == ' ' ||
            str[len - 1] == '\t' ||
            str[len - 1] == '\r' ||
            str[len - 1] == '\n')) {
        str[--len] = '\0';
    }

    return str;
}

/* Divide una línea CSV en campos. */
static int parseCSV(char* line, char fields[][MAX_FIELD_LEN], size_t maxFields)
{
    size_t count = 0;
    char* start = line;

    while (true) {
        char* comma = strchr(start, ',');

        if (count >= maxFields)
            return -1;

        if (comma != NULL)
            *comma = '\0';

        char* field = trim(start);
        size_t len = strlen(field);

        if (len >= MAX_FIELD_LEN)
            return -1;

        memcpy(fields[count], field, len + 1);
        count++;

        if (comma == NULL)
            break;

        start = comma + 1;
    }

    return (int)count;
}

/* Lee y almacena los nombres de las columnas */
static bool readHeader(config_file_t * configFile_)
{
    char line[CONFIG_READ_MAX_LINEA_CHAR];

    if (fgets(line, sizeof(line), configFile_->file) == NULL)
        return false;

    /* Detectar líneas que exceden el buffer */
    if (strchr(line, '\n') == NULL && !feof(configFile_->file))
        return false;

    char fields[CONFIG_NUM_PARAMETERS][MAX_FIELD_LEN];

    int count = parseCSV(line, fields, CONFIG_NUM_PARAMETERS);

    if (count != CONFIG_NUM_PARAMETERS)
        return false;

    for (size_t i = 0; i < CONFIG_NUM_PARAMETERS; i++) {
        strcpy(configFile_->header[i], fields[i]);
    }

    return true;
}

/* Compara los nombres y su orden */
static bool validateHeader(config_file_t* configFile_)
{
    for (size_t i = 0; i < CONFIG_NUM_PARAMETERS; i++) {
        if (strcmp(configFile_->header[i], configFile_->config.pEncabezado[i]) != 0)
            return false;
    }

    return true;
}

/* Escribe el encabezado y los valores predeterminados */
static bool writeDefaultConfig(config_file_t* configFile_)
{
    configFile_->init = true;
    bool ok = config_file_write(configFile_, &CONFIG_BK_DEFAULT) == CONFIG_STATUS_OK;
    configFile_->init = false;

    return ok;
}

/* Convierte un campo en float y comprueba que sea válido */
static bool parseFloat(const char* text, float* value)
{
    char* end;
    errno = 0;

    float result = strtof(text, &end);

    if (text == end || *end != '\0' ||
        errno == ERANGE || !isfinite(result)) {
        return false;
    }

    *value = result;
    return true;
}

static bool parseUint(const char* text, uint32_t* value)
{
    char* end;
    errno = 0;

    float result = strtoul(text, &end,10);

    if (text == end || *end != '\0' ||
        errno == ERANGE || !isfinite(result)) {
        return false;
    }

    *value = result;
    return true;
}

static bool readValues(config_file_t* configFile_) {
    configFile_->init = 1;
    config_status_t status = config_file_read(configFile_, &configFile_->data);
    configFile_->init = 0;
    return status == CONFIG_STATUS_OK;
}

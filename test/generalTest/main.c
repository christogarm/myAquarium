
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "rtos_resources.h"

/* Includes Temperature Libraries */
#include "tempTask.h"
#include "temperature.h"
#include "temperatureSim.h"

/* Includes Heater Libraries */
#include "heaterTask.h"

/* Includes Control Temperatures */
#include "tempCtrlTask.h"

/* Includes Plant */
#include "plantTask.h"

/* Includes Logger */
#include "loggerTask.h"
#include "logger.h"
#include "logger_file.h"

/* Includes Time */
#include "timeSim.h"

/* Include Configuration */
#include "config.h"
#include "config_file.h"

/* Variables */
tempTask_sensor_t ambientTempSensor;
temperatureSim_t ambientTempSim;

    /* Logger */
loggerTask_log_t logging;
logger_file_t loggerFile;
const char* encabezadoLogEvent[] = {
    "timeUNIX_init", 
    "timeUNIX_end", 
    "type", 
    "tempAmb_init", 
    "tempAmb_end", 
    "tempWater_init", 
    "tempWater_end", 
    "heaterPower_init", 
    "heaterPower_end"
};
const char* encabezadoLogTime[] = {
    "timeUNIX",
    "tempAmb",
    "tempWater",
    "heaterPower"
};
time_sim_t timeLogging;
time_sim_t eventLogging;
eventLogger_t buffEvents[LOGGER_EVENT_TYPE_NUM_EVENTS];

    /* Heater */
heaterTask_log_t heater;
char* HeaterSimText = "Salida Calentador1";
char** HeaterSim;
static heater_status_t heaterSim_init(void* instance_, const void* config_);
static heater_status_t heaterSim_setOutput(void* instance_, float output_);

    /* Plant */
plantTask_t planta = { 0 };

    /* Configuraciones */
const char* dataConfig_names[] = {
    "Setpoint",
    "Deadband",
    "Rate Limit",
    "Init Output",
    "Kp", "Ki", "Kd",

    "Mininimun Power Heater",
    "Maximun Power Heater",

    "Maximun Water Temperature",
    "Minimun Water Temperature",
    "Diferential Water Temperature",
    "Time Show Fail Water Temperature",

    "Maximun Ambient Temperature",
    "Minimun Ambient Temperature",
    "Diferential Ambient Temperature",
    "Time Show Fail Ambient Temp",

    "Time Add New Register Logger",
    "Enabled Events Mask",

    "Configuration Flags",
};
config_t config_BK;
config_file_t configFile_BK;
config_t config_SYS;
config_file_t configFile_SYS;
dataConfig_t configData_RAM;

/* prototipos de las Funciones Privadas */
static void initConfig(void);
static void initAmbientTempSensor(void);
static void initLogger(void);
static void initHeater(void);
static void initPlant(void);


int main(void)
{
    /* init Configuration */
    initConfig();

    /* init RTOS Resources */
    rtos_resources_init();

    /* init Temperature Sensor */
    initAmbientTempSensor();
    initLogger();
    initHeater();
    initPlant();

    /* init Tasks */
    xTaskCreate(vTempControlTask, "TempCtrl", configMINIMAL_STACK_SIZE, NULL,
        tskIDLE_PRIORITY + 4, NULL);
    //xTaskCreate(vTempTask, "waterTempTask", configMINIMAL_STACK_SIZE, &waterTempSensor,
        //tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vTempTask, "ambientTempTask", configMINIMAL_STACK_SIZE, &ambientTempSensor,
        tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vHeaterTask, "heaterTask", configMINIMAL_STACK_SIZE, &heater,
        tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vTimeLoggerTask, "timeLoggerTask", configMINIMAL_STACK_SIZE, &logging,
        tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vPlantTask, "waterTempTask", configMINIMAL_STACK_SIZE, &planta,
        tskIDLE_PRIORITY + 4, NULL);
    //xTaskCreate(vTempControlTask, "CtrlTemp", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    /* No debería llegar aquí a menos que falle el heap. */
    for (;; );
    return 0;
}

/* Funciones Privadas */
static void initConfig(void){
    /* Configuracion de las configuraciones Backup */
    configFile_BK.config.filename = "config_BK.csv";
    configFile_BK.config.pEncabezado = dataConfig_names;

    config_BK.config.init_f = config_file_init;
    config_BK.config.write_f = config_file_write;
    config_BK.config.read_f = config_file_read;

    config_BK.config.instance = &configFile_BK;
    config_BK.config.instanceConfig = &configFile_BK.config;

    config_init(&config_BK, &config_BK.config);

    /* Configuracion de las Configuracion del Sistema */
    configFile_SYS.config.filename = "config_SYS.csv";
    configFile_SYS.config.pEncabezado = dataConfig_names;

    config_SYS.config.init_f = config_file_init;
    config_SYS.config.write_f = config_file_write;
    config_SYS.config.read_f = config_file_read;

    config_SYS.config.instance = &configFile_SYS;
    config_SYS.config.instanceConfig = &configFile_SYS.config;

    config_init(&config_SYS, &config_SYS.config);

    /* Guardamos las configuraciones del sistema en RAM */
    configData_RAM = config_SYS.dataConfig;
}

static void initAmbientTempSensor(void) {
    /* Configuracion del Dispositvio */
    ambientTempSim.config.filename = "ambTempSim.csv";

	/* Configuracion de la Tarea */
    ambientTempSensor.config.timeDelayTask_ms = 1000;          // Retraso de la tarea en milisegundos
    ambientTempSensor.config.numTimesToSendData = 1;         // Numero de veces que se enviara la data de temperatura
    ambientTempSensor.config.queueID = QUEUE_ID_TEMP_AMB;   // Cola para enviar los datos de temperatura
    ambientTempSensor.config.readyData = READY_DATA_TEMP_AMB;
    ambientTempSensor.config.flagEnable = CONFIG_DEVICE_FLAG_AMB_TEMP; // Posicion de la configuracion en la memoria RAM

    ambientTempSensor.config.sensor.config.highTemperature = configData_RAM.maxAmbientTemperature; // Temperatura mínima simulada
    ambientTempSensor.config.sensor.config.lowTemperature = configData_RAM.minAmbientTemperature; // Temperatura mínima simulada
    ambientTempSensor.config.sensor.config.difTemperature = configData_RAM.difAmbientTemperature; // Diferencia de temperatura para marcar el sensor como fallido
    ambientTempSensor.config.sensor.config.errorTemperature = TEMPERATURE_ERROR_MEASUREMENT; // Temperatura de error  
    ambientTempSensor.config.sensor.config.failedReadingTime_s = configData_RAM.timeShowFailAmbientTemp;

    ambientTempSensor.config.sensor.config.device.init_f = temperatureSim_init;
    ambientTempSensor.config.sensor.config.device.updateSample_f = temperatureSim_updateSample;
    ambientTempSensor.config.sensor.config.device.reset_f = temperatureSim_reset;
    ambientTempSensor.config.sensor.config.device.instance = &ambientTempSim;
    ambientTempSensor.config.sensor.config.device.instanceConfig = &ambientTempSim.config;
}

static void initLogger(void) {
    /* Inicialización del logger para almacenamienti en archivos */
	loggerFile.configEvent.filename = "EventosLog.csv";
	loggerFile.configEvent.pEncabezado = &encabezadoLogEvent;
	loggerFile.configTime.filename = "TiempoLog.csv";
	loggerFile.configTime.pEncabezado = &encabezadoLogTime;

    /* Incializacion del tiempo*/
    timeLogging.config.utc_offset_seconds = 0;
    timeLogging.config.use_start_time = false;
    timeLogging.config.start_unix = 0;

    eventLogging.config.utc_offset_seconds = 0;
    eventLogging.config.use_start_time = false;
    eventLogging.config.start_unix = 0;

    /*Configuracion de la Tarea del logging*/
	logging.config.timeAddRegisterTime_ms = (uint32_t)(configData_RAM.timeAddNewRegisterLogger*1000); // Retraso de la tarea en segundos
    logging.config.typesEvents_enable = SYS_EVENT_GROUP_ALL;
    logging.config.buffEvents = buffEvents;

	logging.logger.config.init_f = logger_file_init;
    logging.logger.config.addRegisterEvent_f = logger_file_addEventRegister;
    logging.logger.config.addRegisterTime_f = logger_file_addTimeRegister;
    logging.logger.config.readTimeLogger_f = logger_file_readTimeLogger;
    logging.logger.config.readEventLogger_f = logger_file_readEventLogger;

    logging.logger.config.instance = &loggerFile;
    logging.logger.config.configInstanceEvent = &loggerFile.configEvent;
    logging.logger.config.configInstanceTime = &loggerFile.configTime;

	logging.timeLog_time.config.init_f = time_sim_init;
    logging.timeLog_time.config.getTime_f = time_sim_get_time;
    logging.timeLog_time.config.instance = &timeLogging;
    logging.timeLog_time.config.configInstance = &timeLogging.config;

    logger_init(&logging.logger, &logging.logger.config);
}

static void initHeater(void) {
	heater.config.timeUpdateHeater_ms = 100;            // Retraso de la tarea en milisegundos
	heater.config.queueID = QUEUE_ID_HEATER_POWER;      // Cola para enviar los datos de temperatura
	heater.config.readyData = READY_DATA_HEATER_POWER;  // Bandera para indicar que la data esta lista

	heater.heater.config.init_f = heaterSim_init;
    heater.heater.config.setOutput_f = heaterSim_setOutput;
    heater.heater.config.instance = &HeaterSim;
    heater.heater.config.configInstance = HeaterSimText;
	heater.heater.config.rateLimit = configData_RAM.rateLimit; // Limite de cambio de potencia por segundo
	heater.heater.config.minOutput = configData_RAM.minPowerHeater; // Potencia mínima del calefactor
	heater.heater.config.maxOutput = configData_RAM.maxPowerHeater; // Potencia máxima del calefactor

}

static heater_status_t heaterSim_init(void* instance_, const void* config_) {
    return HEATER_STATUS_OK;
}

static heater_status_t heaterSim_setOutput(void* instance_, float output_) {
    printf("Calentador: %.2f\n", output_);
    return HEATER_STATUS_OK;
}

static void initPlant(void) {
    planta.plant.config.tau = 600;
    planta.plant.config.K = 5;
    planta.plant.config.initOutput = 10.0f;
    planta.plant.config.initInput = 0.0f;
    planta.plant.config.maxOut = 45.0f;
    planta.plant.config.minOut = -45.0f;
    planta.plant.config.minIn = 0;
    planta.plant.config.maxIn = 100;
    planta.plant.config.noiseAmpIN = 0;
    planta.plant.config.noiseAmpOUT = 0;
    planta.plant.config.delaySamplesIN = 1;
    planta.plant.config.delaySamplesOUT = 1;

    planta.config.configPlant = &planta.plant.config;
    planta.config.timeDelayTask_ms = 1000;
    planta.config.queueID = QUEUE_ID_TEMP_WATER;
    planta.config.readyData = READY_DATA_TEMP_WATER;

}


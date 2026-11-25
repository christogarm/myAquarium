
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
#include "PID.h"

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

/* Include Configuration */
#include "diagTask.h"


/* Variables */
tempTask_sensor_t ambientTempSensor;
temperatureSim_t ambientTempSim;

    /* Logger */
loggerTask_t timeLogger;
logger_file_t timeLogger_file;
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

loggerTask_t eventLogger;
logger_file_t eventLogger_file;
const char* encabezadoLogTime[] = {
    "timeUNIX",
    "tempAmb",
    "tempWater",
    "heaterPower"
};

    /* Heater */
heaterTask_t heater;
char* HeaterSimText = "Salida Calentador1";
char** HeaterSim;
static heater_status_t heaterSim_init(void* instance_, const void* config_);
static heater_status_t heaterSim_setOutput(void* instance_, float output_);

    /* Control */
tempCtrlTask_t tempCtrlTask = { 0 };

    /* Plant */
plantTask_t plantTask = { 0 };

    /* Configuraciones */
const char* dataConfig_names[] = {
    "Setpoint",
    "Deadband",
    "Rate Limit",
    "Init Output",
    "Kp", "Ki", "Kd",

    "Mininimun Power Heater",
    "Maximun Power Heater",
    "Diferential Power Heater",
    "Maximun Power Heater Alarm Delay",

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

    /* Diagnostico */
diagTask_t diagTask;
time_sim_t timeSim;

/* prototipos de las Funciones Privadas */
static void initConfig(void);
static void initAmbientTempSensor(void);
static void initLogger(void);
static void initHeater(void);
static void initControl(void);
static void initPlant(void);
static void initDaignostic(void);


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
    initControl();
    initPlant();
    initDaignostic();

    /* init Tasks */
    xTaskCreate(vTempControlTask, "TempCtrl", configMINIMAL_STACK_SIZE, &tempCtrlTask,
        tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(vTempTask, "ambientTempTask", configMINIMAL_STACK_SIZE, &ambientTempSensor,
        tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vHeaterTask, "heaterTask", configMINIMAL_STACK_SIZE, &heater,
        tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vLoggerTask, "timeLoggerTask", configMINIMAL_STACK_SIZE, &timeLogger,
        tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vPlantTask, "waterTempTask", configMINIMAL_STACK_SIZE, &plantTask,
        tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(vDiagTask, "diagTask", configMINIMAL_STACK_SIZE, &diagTask,
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
    ambientTempSensor.config.bitSensor = TRANS_BIT_TEMP_AMB;
    ambientTempSensor.config.flagEnable = CONFIG_DEVICE_FLAG_AMB_TEMP; // Posicion de la configuracion en la memoria RAM

    ambientTempSensor.sensor.config.highTemperature = configData_RAM.maxAmbientTemperature; // Temperatura mínima simulada
    ambientTempSensor.sensor.config.lowTemperature = configData_RAM.minAmbientTemperature; // Temperatura mínima simulada
    ambientTempSensor.sensor.config.difTemperature = configData_RAM.difAmbientTemperature; // Diferencia de temperatura para marcar el sensor como fallido
    ambientTempSensor.sensor.config.errorTemperature = TEMPERATURE_ERROR_MEASUREMENT; // Temperatura de error  
    ambientTempSensor.sensor.config.failedReadingTime_s = configData_RAM.timeShowFailAmbientTemp;

    ambientTempSensor.sensor.config.device.init_f = temperatureSim_init;
    ambientTempSensor.sensor.config.device.updateSample_f = temperatureSim_updateSample;
    ambientTempSensor.sensor.config.device.reset_f = temperatureSim_reset;
    ambientTempSensor.sensor.config.device.instance = &ambientTempSim;
    ambientTempSensor.sensor.config.device.instanceConfig = &ambientTempSim.config;
}

static void initLogger(void) {
    /* Inicialización del logger para almacenamiento en archivos */
	timeLogger_file.config.filename = "TiempoLog.csv";
	timeLogger_file.config.pEncabezado = encabezadoLogTime;
	timeLogger_file.config.type = LOGGER_TYPE_TIME;
    timeLogger_file.config.numColumns = LOGGER_SIZE_COLUMNS_TIME;

    eventLogger_file.config.filename = "EventosLog.csv";
    eventLogger_file.config.pEncabezado = encabezadoLogEvent;
    eventLogger_file.config.type = LOGGER_TYPE_EVENT;
    eventLogger_file.config.numColumns = LOGGER_SIZE_COLUMNS_EVENT ;

    /* Configuracion del Logger*/
	timeLogger.logger.config.init_f = logger_file_init;
    timeLogger.logger.config.addRegister_f = logger_file_addRegister;
    timeLogger.logger.config.readRegister_f = logger_file_readRegister;
	timeLogger.logger.config.instance = &timeLogger_file;
    timeLogger.logger.config.instanceConfig = &timeLogger_file;
	timeLogger.logger.config.type = LOGGER_TYPE_TIME;

    eventLogger.logger.config.init_f = logger_file_init;
    eventLogger.logger.config.addRegister_f = logger_file_addRegister;
    eventLogger.logger.config.readRegister_f = logger_file_readRegister;
    eventLogger.logger.config.instance = &eventLogger_file;
    eventLogger.logger.config.instanceConfig = &eventLogger_file;
    eventLogger.logger.config.type = LOGGER_TYPE_EVENT;

    /*Configuracion de la Tarea del Logger */
	timeLogger.config.queueID = QUEUE_ID_LOGGER_TIME;
    
    eventLogger.config.queueID = QUEUE_ID_LOGGER_EVENT;
}

static void initHeater(void) {
	heater.config.timeUpdateHeater_ms = 100;            // Retraso de la tarea en milisegundos
	heater.config.heaterBit = TRANS_BIT_HEATER_POWER;  // Bandera para indicar que la data esta lista
	heater.config.flagEnable = (configData_RAM.flags & CONFIG_DEVICE_FLAG_HEATER) != 0; // Posicion de la configuracion en la memoria RAM

	heater.heater.config.init_f = heaterSim_init;
    heater.heater.config.setOutput_f = heaterSim_setOutput;
    heater.heater.config.instance = &HeaterSim;
    heater.heater.config.configInstance = HeaterSimText;
    heater.heater.config.maxPowerAlarm_delay_s = configData_RAM.maxPowerAlarm_delay_s;
    heater.heater.config.rateLimit = configData_RAM.rateLimit; // Limite de cambio de potencia por segundo
    heater.heater.config.difOutput = configData_RAM.difPowerHeater;
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

static void initControl(void) {
    tempCtrlTask.temperPID.param.setPoint = configData_RAM.setpoint;
    tempCtrlTask.temperPID.param.deadband = configData_RAM.deadband;
    tempCtrlTask.temperPID.param.Kp = configData_RAM.kp;
    tempCtrlTask.temperPID.param.Ki = configData_RAM.ki;
    tempCtrlTask.temperPID.param.Kd = configData_RAM.kd;
    tempCtrlTask.temperPID.param.alpha = 0.8;
    tempCtrlTask.temperPID.param.minOutput = 0;
    tempCtrlTask.temperPID.param.maxOutput = 100;
    tempCtrlTask.temperPID.param.initOutput = configData_RAM.initOutput;
    tempCtrlTask.temperPID.param.initInput = 0;
    tempCtrlTask.temperPID.param.clampIntMin = configData_RAM.ki != 0 ? -100 / configData_RAM.ki : -1;
    tempCtrlTask.temperPID.param.clampIntMax = configData_RAM.ki != 0 ? 100 / configData_RAM.ki : 1;

    tempCtrlTask.ctrlTemper.config.instance = &tempCtrlTask.temperPID;
    tempCtrlTask.ctrlTemper.config.init_f = PID_Init;
    tempCtrlTask.ctrlTemper.config.update_f = PID_Update;
    tempCtrlTask.ctrlTemper.config.isInit_f = PID_IsInit;
    tempCtrlTask.ctrlTemper.config.reset_f = PID_Reset;
    tempCtrlTask.ctrlTemper.config.getOutput_f = PID_GetOutput;
    tempCtrlTask.ctrlTemper.config.maxOutput = 100;
    tempCtrlTask.ctrlTemper.config.minOutput = 0;
    tempCtrlTask.ctrlTemper.config.configInstance = &tempCtrlTask.temperPID.param;
}

static void initPlant(void) {
    plantTask.plant.config.tau = 600;
    plantTask.plant.config.K = 5;
    plantTask.plant.config.initOutput = 10.0f;
    plantTask.plant.config.initInput = 0.0f;
    plantTask.plant.config.maxOut = 45.0f;
    plantTask.plant.config.minOut = -45.0f;
    plantTask.plant.config.minIn = 0;
    plantTask.plant.config.maxIn = 100;
    plantTask.plant.config.noiseAmpIN = 0;
    plantTask.plant.config.noiseAmpOUT = 0;
    plantTask.plant.config.delaySamplesIN = 1;
    plantTask.plant.config.delaySamplesOUT = 1;

    plantTask.config.timeDelayTask_ms = 1000;
}

static void initDaignostic(void) {
    /* Configuracion del Tiempo Simulado*/
    timeSim.config.utc_offset_seconds = 0;
    timeSim.config.use_start_time = 0;
    timeSim.config.start_unix = 0;

    /* Configuracion de la tarea */
	diagTask.config.timeDelayTask_ms = 200; // Retraso de la tarea en milisegundos
	diagTask.config.timeLogger_addNewReg_s = configData_RAM.timeAddNewRegisterLogger; // Tiempo para agregar un nuevo registro al logger
    
    /* Configuracion del Servicio de Tiempo */
	diagTask.timeService.config.init_f = time_sim_init;
    diagTask.timeService.config.getTime_f = time_sim_get_time;

    diagTask.timeService.config.instance = &timeSim;
    diagTask.timeService.config.configInstance = &timeSim.config;
}
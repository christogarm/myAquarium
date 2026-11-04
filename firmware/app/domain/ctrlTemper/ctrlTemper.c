/*
 * Author: Christogarm
 * 
 */


 #include "ctrlTemper.h"

 /* Prototipos de las Funciones privadas */
static void ctrlTemper_stateReset(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_);
static void ctrlTemper_stateWait(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_);
static void ctrlTemper_stateControl(ctrlTemper_t * ctrlTemper_);
static void ctrlTemper_stateFault(ctrlTemper_t * ctrlTemper_,  _Bool * eventFlags_);
static void ctrlTemper_stateDisable(ctrlTemper_t * ctrlTemper_,  _Bool * eventFlags_);

 /* Funciones Publicas */
ctrlTemper_status_t ctrlTemper_Init(ctrlTemper_t * ctrlTemper_, ctrlTemper_config_t * config_){
    if(ctrlTemper_ == NULL || config_ == NULL || config_->instance == NULL ||
        config_->configInstance == NULL || config_->init_f == NULL || 
        config_->isInit_f == NULL || config_->update_f == NULL)
        return CTRL_TEMPER_STATUS_NULL_POINTER;
    
	config_->init_f(config_->instance, config_->configInstance);   // Inicializamos el Controlador de Temperatura
	if (config_->isInit_f(config_->instance) == 0)
		return CTRL_TEMPER_STATUS_ERROR;

    ctrlTemper_->config = *config_;
    ctrlTemper_ ->state = CTRL_TEMPER_STATE_RESET;
    ctrlTemper_->init = 1;
    ctrlTemper_->heaterPower = 0;
    ctrlTemper_->ambTemp = 0;
    ctrlTemper_->waterTemp = 0;
    ctrlTemper_->dt = 0;
    ctrlTemper_->newDataPower = 0;

    return CTRL_TEMPER_STATUS_OK;
}    

ctrlTemper_status_t ctrlTemper_MachineState(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_){
    if(ctrlTemper_ == NULL || eventFlags_ == NULL)
        return CTRL_TEMPER_STATUS_NULL_POINTER;

    if(!ctrlTemper_->init)
        return CTRL_TEMPER_STATUS_ERROR;

    if(eventFlags_[CTRL_TEMPER_EVENT_POWER_ON] || eventFlags_[CTRL_TEMPER_EVENT_RECONFIGURATION])
        ctrlTemper_->state = CTRL_TEMPER_STATE_RESET;
    else if(!eventFlags_[CTRL_TEMPER_EVENT_ENABLE_CONTROL])
        ctrlTemper_->state = CTRL_TEMPER_STATE_DISABLED;
    else if(eventFlags_[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] || eventFlags_[CTRL_TEMPER_EVENT_HEATER_FAIL] ||
        eventFlags_[CTRL_TEMPER_EVENT_CONFIG_FAIL] )
        ctrlTemper_->state = CTRL_TEMPER_STATE_FAULT;
    
    /* Determina el nuevo evento */
    switch (ctrlTemper_->state)
    {
    case CTRL_TEMPER_STATE_RESET:
        ctrlTemper_stateReset(ctrlTemper_, eventFlags_);
        break;
    case CTRL_TEMPER_STATE_WAIT:
        ctrlTemper_stateWait(ctrlTemper_, eventFlags_);
        break;
    case CTRL_TEMPER_STATE_CONTROL:
        ctrlTemper_stateControl(ctrlTemper_);
        break;
    case CTRL_TEMPER_STATE_FAULT:
        ctrlTemper_stateFault(ctrlTemper_, eventFlags_);
        break;
    case CTRL_TEMPER_STATE_DISABLED:
        ctrlTemper_stateDisable(ctrlTemper_, eventFlags_);
        break;

    default:
        ctrlTemper_->state = CTRL_TEMPER_STATE_RESET;
        return CTRL_TEMPER_STATUS_ERROR;
        break;
    }
    
    return CTRL_TEMPER_STATUS_OK;
}

ctrlTemper_status_t ctrlTemper_setTemperatures(ctrlTemper_t * ctrlTemper_, float ambTemp_, float waterTemp_){
    if(ctrlTemper_ == NULL)
        return CTRL_TEMPER_STATUS_NULL_POINTER;

    ctrlTemper_->ambTemp = ambTemp_;
    ctrlTemper_->waterTemp = waterTemp_;
    
    return CTRL_TEMPER_STATUS_OK;
}

ctrlTemper_status_t ctrlTemper_setDtime(ctrlTemper_t * ctrlTemper_, float dt_){
    if(ctrlTemper_ == NULL)
        return CTRL_TEMPER_STATUS_NULL_POINTER;

    ctrlTemper_->dt = dt_;
    
    return CTRL_TEMPER_STATUS_OK;
}

ctrlTemper_status_t ctrlTemper_setState(ctrlTemper_t * ctrlTemper_, ctrlTemper_state_t state_){
    if(ctrlTemper_ == NULL)
        return CTRL_TEMPER_STATUS_NULL_POINTER;

    ctrlTemper_->state = state_;
    
    return CTRL_TEMPER_STATUS_OK;
}

ctrlTemper_state_t ctrlTemper_getState(ctrlTemper_t * ctrlTemper_){
    if(ctrlTemper_ == NULL)
        return CTRL_TEMPER_STATE_FAULT;
    
    return ctrlTemper_->state;
}

float ctrlTemper_getHeaterPower(ctrlTemper_t * ctrlTemper_){
    if(ctrlTemper_ == NULL)
        return 0;

    /* Limpiamos la bandera de que ya consumimos un nuevo valor de Potencia */
    ctrlTemper_->newDataPower = 0;

    /* Entregamos la nueva Potencia */
    return ctrlTemper_->heaterPower;
}

_Bool ctrlTemper_isAvailablePowerHeater(ctrlTemper_t * ctrlTemper_){
    if(ctrlTemper_ == NULL)
        return 0;
    return ctrlTemper_->newDataPower;
}

/* Funciones Privadas */
static void ctrlTemper_stateReset(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_){
    ctrlTemper_->heaterPower = 0;
    ctrlTemper_->newDataPower = 0;
    ctrlTemper_->dt = 0;

    if(eventFlags_[CTRL_TEMPER_EVENT_FIRST_SAMPLE]){
        ctrlTemper_->state = CTRL_TEMPER_STATE_WAIT;
		ctrlTemper_->config.reset_f(ctrlTemper_->config.instance); // Reiniciamos el Controlador de Temperatura
    }
}

static void ctrlTemper_stateWait(ctrlTemper_t * ctrlTemper_, _Bool * eventFlags_){
    if(eventFlags_[CTRL_TEMPER_EVENT_SAMPLE_UPDATE]) 
        ctrlTemper_->state = CTRL_TEMPER_STATE_CONTROL;
 }

static void ctrlTemper_stateControl(ctrlTemper_t * ctrlTemper_){
    float PIDout = 0;
    float FFout = 0;

    ctrlTemper_->state = CTRL_TEMPER_STATE_WAIT;

    /* Ejecutamos el control PID */
    ctrlTemper_->config.update_f(ctrlTemper_->config.instance, ctrlTemper_->waterTemp, ctrlTemper_->dt);
	PIDout = ctrlTemper_->config.getOutput_f(ctrlTemper_->config.instance); 

    /* Falta Implementar Feedfoward */
    
    /* Aviso de que tenemos una nueva salida una nueva Salida */
    ctrlTemper_->newDataPower = 1;
    
    /*Saturacion de la Salida */
    float ctrlOut = PIDout + FFout;
    if(ctrlOut > ctrlTemper_->config.maxOutput)
        ctrlOut = ctrlTemper_->config.maxOutput;
    else if(ctrlOut < ctrlTemper_->config.minOutput)
        ctrlOut = ctrlTemper_->config.minOutput;

    /* Suma de las dos salidas */
    ctrlTemper_->heaterPower = ctrlOut;
}

static void ctrlTemper_stateFault(ctrlTemper_t * ctrlTemper_,  _Bool * eventFlags_){
    if (!eventFlags_[CTRL_TEMPER_EVENT_WATER_TEMP_FAIL] && !eventFlags_[CTRL_TEMPER_EVENT_HEATER_FAIL] && 
        !eventFlags_[CTRL_TEMPER_EVENT_CONFIG_FAIL])
        ctrlTemper_->state = CTRL_TEMPER_STATE_RESET;    
}

static void ctrlTemper_stateDisable(ctrlTemper_t * ctrlTemper_,  _Bool * eventFlags_){
    if (eventFlags_[CTRL_TEMPER_EVENT_ENABLE_CONTROL])
        ctrlTemper_->state = CTRL_TEMPER_STATE_RESET;  
}
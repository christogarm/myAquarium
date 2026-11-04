/*
 * Author: Christogarm
 */

 #include "config.h"



/* Funciones Publicas */
config_status_t config_init(config_t* configInst_, config_config_t * config_){
    if (configInst_ == NULL || config_ == NULL ||
        config_->instance == NULL || config_->instanceConfig == NULL ||
        config_->init_f == NULL || config_->write_f == NULL ||
        config_->read_f == NULL)
        return CONFIG_STATUS_NULL_POINTER;

    if(config_->init_f(config_->instance, config_->instanceConfig) != CONFIG_STATUS_OK)
        return CONFIG_STATUS_ERROR;

    configInst_->config = *config_;
    configInst_->init = true;
    config_read(configInst_);

    return CONFIG_STATUS_OK;
}

config_status_t config_write(config_t* configInst_) {
    if (configInst_ == NULL)
        return CONFIG_STATUS_NULL_POINTER;
    if (!configInst_->init)
        return CONFIG_STATUS_ERROR;

    return configInst_->config.write_f(configInst_->config.instance, &configInst_->dataConfig);
}


config_status_t config_read(config_t* configInst_){
    if (configInst_ == NULL)
        return CONFIG_STATUS_NULL_POINTER;
    if (!configInst_->init)
        return CONFIG_STATUS_ERROR;

    return configInst_->config.read_f(configInst_->config.instance, &configInst_->dataConfig);
}


/* */


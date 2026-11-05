/*
 * Author: Christogarm
 */

 /* includes */
#include "interpolation.h"


/* Prototipos de las funciones Privadas */
static int indexFind(interpolation_t* instance_, const float input_);
static void indexFind_withMemory(interpolation_t* instance_, const float input_);

/* Funciones Públicas */
interpolation_status_t interpolation_init(interpolation_t* instance_, const interpolation_config_t* config_) {
    if (instance_ == NULL || config_ == NULL ||
        config_->tabX == NULL || config_->tabY == NULL)
        return INTERPOLATION_STATUS_NULL_POINTER;

    if (config_->sizeTab < 2 || config_->attempts == 0)
        return INTERPOLATION_STATUS_ERROR;

    instance_->config = *config_;
    instance_->init = true;

    return INTERPOLATION_STATUS_OK;
}

float interpolation_calculate(interpolation_t* instance_, const float input_) {
    if (instance_ == NULL)
        return 0;

    if (!instance_->init)
        return 0;

    indexFind_withMemory(instance_, input_);

    if (instance_->config.tabX[instance_->indexCache + 1] == instance_->config.tabX[instance_->indexCache])
        return 0;

    return instance_->config.tabY[instance_->indexCache] +
        (input_ - instance_->config.tabX[instance_->indexCache]) *
        (instance_->config.tabY[instance_->indexCache + 1] - instance_->config.tabY[instance_->indexCache]) /
        (instance_->config.tabX[instance_->indexCache + 1] - instance_->config.tabX[instance_->indexCache]);
}


/* Funciones Privadas */
static int indexFind(interpolation_t* instance_, const float input_)
{
    uint32_t low = 0;
    uint32_t high = instance_->config.sizeTab - 1;

    if (input_ <= instance_->config.tabX[0])
        return 0;
    if (input_ >= instance_->config.tabX[instance_->config.sizeTab - 1])
        return instance_->config.sizeTab - 2;

    while (low < high - 1) {
        uint32_t middle = (low + high) / 2;
        if (instance_->config.tabX[middle] <= input_)
            low = middle;
        else
            high = middle;
    }
    return low;
}

static void indexFind_withMemory(interpolation_t* instance_, const float input_)
{
    uint32_t index = instance_->indexCache;

    if (index < 0 || index > instance_->config.sizeTab - 2) {
        index = indexFind(instance_, input_);
        instance_->indexCache = index;
        return;
    }

    if (input_ >= instance_->config.tabX[index] && input_ <= instance_->config.tabX[index + 1])
        return;

    if (input_ > instance_->config.tabX[index + 1]) {
        uint32_t pasos = 0;
        while (index < instance_->config.sizeTab - 2 && input_ > instance_->config.tabX[index + 1] && pasos < instance_->config.attempts) {
            index++;
            pasos++;
        }
        if (input_ > instance_->config.tabX[index + 1])
            index = indexFind(instance_, input_);
        instance_->indexCache = index;
        return;
    }
    else {
        int pasos = 0;
        while (index > 0 && input_ < instance_->config.tabX[index] && pasos < instance_->config.attempts) {
            index--;
            pasos++;
        }
        if (input_ < instance_->config.tabX[index])
            index = indexFind(instance_, input_);
        instance_->indexCache = index;
        return;
    }
}

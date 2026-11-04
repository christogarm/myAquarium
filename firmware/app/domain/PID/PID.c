/*
 * PID.c
 *
 *  Created on: 11 mar 2026
 *      Author: Christogarm
 */

#include "PID.h"
#include <math.h> /* fabsf() */

/* -----------------------------------------------------------------------
 * Prototipos de funciones privadas
 * ----------------------------------------------------------------------- */


/* -----------------------------------------------------------------------
 * Inicializacion
 * ----------------------------------------------------------------------- */

/**
 * @brief Inicializa el controlador PID.
 */
PID_Status PID_Init(PID_Control *Ppid_, PID_Parameters *Pparam_)
{
	if (Ppid_ == NULL || Pparam_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (Pparam_->minOutput >= Pparam_->maxOutput)
		return PID_STATUS_ERROR;

	if (Pparam_->clampIntMin >= Pparam_->clampIntMax)
		return PID_STATUS_ERROR;

	if (Pparam_->alpha < 0.0f || Pparam_->alpha > 1.0f)
		return PID_STATUS_ERROR;

	if (Pparam_->deadband < 0.0f)
		return PID_STATUS_ERROR;

	if (Pparam_->initOutput > Pparam_->maxOutput)
		Pparam_->initOutput = Pparam_->maxOutput;

	if (Pparam_->initOutput < Pparam_->minOutput)
		Pparam_->initOutput = Pparam_->minOutput;

	Ppid_->param = *Pparam_;
	Ppid_->init = 1;

	PID_Reset(Ppid_);

	return PID_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * Configuracion en tiempo de ejecucion
 * ----------------------------------------------------------------------- */

PID_Status PID_setSetPoint(PID_Control *Ppid_, float setPoint_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	Ppid_->param.setPoint = setPoint_;

	return PID_STATUS_OK;
}

PID_Status PID_setGain(PID_Control *Ppid_, float Kp_, float Ki_, float Kd_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	Ppid_->param.Kp = Kp_;
	Ppid_->param.Ki = Ki_;
	Ppid_->param.Kd = Kd_;

	return PID_STATUS_OK;
}

PID_Status PID_setLimitOut(PID_Control *Ppid_, float minOut_, float maxOut_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (minOut_ >= maxOut_)
		return PID_STATUS_ERROR;

	Ppid_->param.minOutput = minOut_;
	Ppid_->param.maxOutput = maxOut_;

	return PID_STATUS_OK;
}

PID_Status PID_setClampInt(PID_Control *Ppid_, float minInt_, float maxInt_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (minInt_ >= maxInt_)
		return PID_STATUS_ERROR;

	Ppid_->param.clampIntMin = minInt_;
	Ppid_->param.clampIntMax = maxInt_;

	return PID_STATUS_OK;
}

PID_Status PID_setAlpha(PID_Control *Ppid_, float alpha_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (alpha_ < 0.0f || alpha_ > 1.0f)
		return PID_STATUS_ERROR;

	Ppid_->param.alpha = alpha_;

	return PID_STATUS_OK;
}

PID_Status PID_setDeadband(PID_Control *Ppid_, float deadband_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (deadband_ < 0.0f)
		return PID_STATUS_ERROR;

	Ppid_->param.deadband = deadband_;

	return PID_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * Operacion
 * ----------------------------------------------------------------------- */

/**
 * @brief  Ejecuta un ciclo del controlador PID.
 *
 * Flujo de calculo:
 *   1. Calculo del error.
 *   2. Banda muerta.
 *   3. Acumulador integral con anti-windup condicional.
 *   4. Termino derivativo sobre la medicion (evita derivative kick).
 *   5. Filtro EMA sobre el derivativo.
 *   6. Clamp del acumulador integral.
 *   7. Salida PID.
 *   8. Rate limiter.
 *   9. Saturacion de salida.
 *  10. Actualizacion de valores anteriores.
 *

 */
PID_Status PID_Update(PID_Control *Ppid_, float input_, float dt_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	if (dt_ <= 0.0f)
		return PID_STATUS_ERROR;

	if(!Ppid_->init)
		return PID_STATUS_ERROR;

	Ppid_->input = input_; /* Medicion actual */

	/* --- Error de seguimiento --- */
	Ppid_->error = Ppid_->param.setPoint - Ppid_->input;

	/* --- Banda muerta ---
	 * Si el error es menor a la banda muerta se trata como cero,
	 * deteniendo el acumulador integral y el termino proporcional. */
	if (fabsf(Ppid_->error) < Ppid_->param.deadband)
		Ppid_->error = 0.0f;

	/* --- Anti-windup condicional (clamping) ---
	 * Solo acumula integral si la salida NO esta saturada en la
	 * direccion que incrementaria el error. */
	if (!((Ppid_->output >= Ppid_->param.maxOutput && Ppid_->error > 0.0f) ||
		  (Ppid_->output <= Ppid_->param.minOutput && Ppid_->error < 0.0f)))
	{
		Ppid_->integralError += Ppid_->error * dt_;
	}

	/* --- Termino derivativo sobre la medicion ---
	 * Usar (lastInput - input) en lugar del error evita el "derivative
	 * kick" cuando el set point cambia abruptamente. */
	Ppid_->derivativeError = -(Ppid_->input - Ppid_->lastInput) / dt_;

	/* --- Filtro EMA sobre el derivativo ---
	 * alpha = 1: solo muestra actual (sin filtro).
	 * alpha = 0: solo muestra anterior (congelado). */
	Ppid_->derivativeError = (Ppid_->param.alpha * Ppid_->derivativeError) +
							 ((1.0f - Ppid_->param.alpha) * Ppid_->lastDerivativeError);

	/* --- Clamp del acumulador integral ---
	 * Segunda linea de defensa ante windup. */
	if (Ppid_->integralError > Ppid_->param.clampIntMax)
		Ppid_->integralError = Ppid_->param.clampIntMax;

	if (Ppid_->integralError < Ppid_->param.clampIntMin)
		Ppid_->integralError = Ppid_->param.clampIntMin;

	/* --- Salida PID --- */
	Ppid_->output = (Ppid_->param.Kp * Ppid_->error) +
					(Ppid_->param.Ki * Ppid_->integralError) +
					(Ppid_->param.Kd * Ppid_->derivativeError);

	/* --- Saturacion de salida --- */
	if (Ppid_->output > Ppid_->param.maxOutput)
		Ppid_->output = Ppid_->param.maxOutput;

	if (Ppid_->output < Ppid_->param.minOutput)
		Ppid_->output = Ppid_->param.minOutput;

	/* --- Actualizacion de valores anteriores --- */
	Ppid_->lastError = Ppid_->error;
	Ppid_->lastOutput = Ppid_->output;
	Ppid_->lastInput = Ppid_->input;
	Ppid_->lastDerivativeError = Ppid_->derivativeError;

	return PID_STATUS_OK;
}

/* -----------------------------------------------------------------------
 * Getters
 * ----------------------------------------------------------------------- */

/**
 * @brief  [FIX-4] Retorna la ultima salida calculada.
 *         Usar este getter en lugar de acceder directamente a Ppid_->output.
 */
float PID_GetOutput(const PID_Control *Ppid_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	return Ppid_->output;
}

/* -----------------------------------------------------------------------
 * Mantenimiento
 * ----------------------------------------------------------------------- */

/**
 * @brief  Reinicia el estado interno del controlador.
 *         Los parametros de configuracion no se modifican.
 */
void PID_Reset(PID_Control *Ppid_)
{
	if (Ppid_ == NULL)
		return PID_STATUS_NULL_POINTER;

	Ppid_->output = Ppid_->param.initOutput;
	Ppid_->lastOutput = Ppid_->param.initOutput;
	Ppid_->input = Ppid_->param.initInput;
	Ppid_->lastInput = Ppid_->param.initInput;

	Ppid_->error = 0.0f;
	Ppid_->lastError = 0.0f;
	Ppid_->integralError = 0.0f;
	Ppid_->derivativeError = 0.0f;
	Ppid_->lastDerivativeError = 0.0f;

	return PID_STATUS_OK;
}

_Bool PID_IsInit(PID_Control* Ppid_){
	if (Ppid_ == NULL)
		return 0;
	
	return Ppid_->init; /* Controlador inicializado correctamente */
}

/* -----------------------------------------------------------------------
 * Funciones privadas
 * ----------------------------------------------------------------------- */


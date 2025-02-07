/*
 * INA3221.h
 *
 *  Created on: Jan 11, 2025
 *      Author: gpt + Jakub ale tylko troszkę
 */

#ifndef INC_INA3221_H_
#define INC_INA3221_H_

#include "INA3221_reg.h"
#if defined(STM32L1)
#include "stm32l1xx_hal.h"
#endif
//#include "I2C.h"
#include "i2c.h"

#define INA3221_CHANNEL_1 0
#define INA3221_CHANNEL_2 1
#define INA3221_CHANNEL_3 2

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    float resistances[3];
    float filters[3];
} INA3221_HandleTypeDef;

void INA3221_Init(INA3221_HandleTypeDef *ina, I2C_HandleTypeDef *hi2c, uint8_t address);
void INA3221_SetShuntResistor(INA3221_HandleTypeDef *ina, uint8_t channel, float resistance);
void INA3221_SetFilterResistor(INA3221_HandleTypeDef *ina, uint8_t channel, float resistance);
float INA3221_GetBusVoltage(INA3221_HandleTypeDef *ina, uint8_t channel);
float INA3221_GetShuntVoltageRaw(INA3221_HandleTypeDef *ina, uint8_t channel);
float INA3221_GetShuntVoltageCorrected(INA3221_HandleTypeDef *ina, uint8_t channel);
float INA3221_GetCurrentRaw(INA3221_HandleTypeDef *ina, uint8_t channel);
float INA3221_GetCurrentCorrected(INA3221_HandleTypeDef *ina, uint8_t channel);
uint16_t INA3221_GetManufacturerID(INA3221_HandleTypeDef *ina);
uint16_t INA3221_GetDieID(INA3221_HandleTypeDef *ina);
uint16_t INA3221_ReadReg(INA3221_HandleTypeDef *ina, uint16_t reg);




#endif /* INC_INA3221_H_ */

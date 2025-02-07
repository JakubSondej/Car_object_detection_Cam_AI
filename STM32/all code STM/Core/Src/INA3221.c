/*
 * INA3221.c
 *
 *  Created on: Jan 11, 2025
 *      Author: jakub
 */

/* INA3221.c */

#include "INA3221.h"
#include <stdio.h>


extern UART_HandleTypeDef huart2;  // UART do debugowania
extern INA3221_HandleTypeDef ina3221;


void INA3221_Init(INA3221_HandleTypeDef *ina, I2C_HandleTypeDef *hi2c, uint8_t address) {
    ina->hi2c = hi2c;
    ina->address = address;
    ina->resistances[0] = 0.01f;
    ina->resistances[1] = 0.01f;
    ina->resistances[2] = 0.01f;
    ina->filters[0] = 0;
    ina->filters[1] = 0;
    ina->filters[2] = 0;
}

void INA3221_SetShuntResistor(INA3221_HandleTypeDef *ina, uint8_t channel, float resistance) {
    if (channel < 3 && resistance > 1e-9) {
        ina->resistances[channel] = resistance;
    }
}

void INA3221_SetFilterResistor(INA3221_HandleTypeDef *ina, uint8_t channel, float resistance)
{
    if (channel < 3 && resistance >= 0)
    {
        ina->filters[channel] = resistance;
    }
}

float INA3221_GetBusVoltage(INA3221_HandleTypeDef *ina, uint8_t channel)
{
    uint16_t reg;
    switch (channel)
    {
        case INA3221_CHANNEL_1: reg = INA3221_REGISTER_C1_BUS_VOLTAGE; break;
        case INA3221_CHANNEL_2: reg = INA3221_REGISTER_C2_BUS_VOLTAGE; break;
        case INA3221_CHANNEL_3: reg = INA3221_REGISTER_C3_BUS_VOLTAGE; break;
        default: return 0;
    }
    return INA3221_ReadReg(ina, reg) * 0.001f;
}

float INA3221_GetShuntVoltageRaw(INA3221_HandleTypeDef *ina, uint8_t channel)
{
    uint16_t reg;
    switch (channel)
    {
        case INA3221_CHANNEL_1: reg = INA3221_REGISTER_C1_SHUNT_VOLTAGE; break;
        case INA3221_CHANNEL_2: reg = INA3221_REGISTER_C2_SHUNT_VOLTAGE; break;
        case INA3221_CHANNEL_3: reg = INA3221_REGISTER_C3_SHUNT_VOLTAGE; break;
        default: return 0;
    }
    uint16_t raw = INA3221_ReadReg(ina, reg);
    return (raw & 0x8000) ? -((~raw + 1) * 5e-6f) : (raw * 5e-6f);
}

float INA3221_GetCurrentRaw(INA3221_HandleTypeDef *ina, uint8_t channel)
{
    return INA3221_GetShuntVoltageRaw(ina, channel) / ina->resistances[channel];
}

uint16_t INA3221_ReadReg(INA3221_HandleTypeDef *ina, uint16_t reg)
{
    uint16_t data;
    HAL_I2C_Mem_Read(ina->hi2c, ina->address << 1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&data, 2, HAL_MAX_DELAY);
    return (data >> 8) | (data << 8);
}

uint16_t INA3221_GetManufacturerID(INA3221_HandleTypeDef *ina)
{
    return INA3221_ReadReg(ina, INA3221_REGISTER_MANUFACTURER_ID);
}

uint16_t INA3221_GetDieID(INA3221_HandleTypeDef *ina)
{
    return INA3221_ReadReg(ina, INA3221_REGISTER_DIE_ID);
}



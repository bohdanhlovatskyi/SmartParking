/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

#define HMC5883L 0x1E
#define MAXLEN 32
#define SENSORS_GAUSS_TO_MICROTESLA (100)

enum {
  HMC5883_REGISTER_MAG_CRA_REG_M = 0x00,
  HMC5883_REGISTER_MAG_CRB_REG_M = 0x01,
  HMC5883_REGISTER_MAG_MR_REG_M = 0x02,
  HMC5883_REGISTER_MAG_OUT_X_H_M = 0x03,
  HMC5883_REGISTER_MAG_OUT_X_L_M = 0x04,
  HMC5883_REGISTER_MAG_OUT_Z_H_M = 0x05,
  HMC5883_REGISTER_MAG_OUT_Z_L_M = 0x06,
  HMC5883_REGISTER_MAG_OUT_Y_H_M = 0x07,
  HMC5883_REGISTER_MAG_OUT_Y_L_M = 0x08,
  HMC5883_REGISTER_MAG_SR_REG_Mg = 0x09,
  HMC5883_REGISTER_MAG_IRA_REG_M = 0x0A,
  HMC5883_REGISTER_MAG_IRB_REG_M = 0x0B,
  HMC5883_REGISTER_MAG_IRC_REG_M = 0x0C,
  HMC5883_REGISTER_MAG_TEMP_OUT_H_M = 0x31,
  HMC5883_REGISTER_MAG_TEMP_OUT_L_M = 0x32
};

static float _hmc5883_Gauss_LSB_XY = 1100.0F; // Varies with gain
static float _hmc5883_Gauss_LSB_Z = 980.0F;   // Varies with gain

typedef struct Vector3 { int x; int y; int z; } MagVector;

void Read_Mag(MagVector *magData);

int main(void)
{
    // uint8 i2cbuf[MAXLEN];
    char msg[MAXLEN];
    static MagVector mag;

    CyGlobalIntEnable; /* Enable global interrupts. */

    I2C_Start();
    UART_Start();   
    
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_WRITE_XFER_MODE, 100);
    I2C_I2CMasterWriteByte(HMC5883_REGISTER_MAG_MR_REG_M, 100);
    // required bits in the register
    I2C_I2CMasterWriteByte(0x00u, 100);
    I2C_I2CMasterSendStop(50);
    
    Pin_Red_Write(1);
    CyDelay( 1000 );
    Pin_Red_Write(0);
 
    for(;;)
    {
        // I2C_MasterRead(HMC5883L, MAXLEN, i2cbuf);
        // sprintf(msg, "%s\r\n", i2cbuf);
        // memset(msg, 0, sizeof(msg)); 
        Pin_Red_Write(1);
        Read_Mag(&mag);

        sprintf(msg, "%i %i %i\r\r", (int)mag.x, (int)mag.y, (int)mag.z);
        UART_UartPutString(msg);
        memset(msg, 0, sizeof(msg));
        CyDelay( 1000 );
        Pin_Red_Write(0);
    }
}

void Read_Mag(MagVector *magData) {
    uint8 data[6];
    
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_WRITE_XFER_MODE, 100);
    I2C_I2CMasterWriteByte(HMC5883_REGISTER_MAG_OUT_X_H_M, 100);
    
    I2C_I2CMasterSendStop(50);
    
    I2C_I2CMasterSendStart(HMC5883L, I2C_I2C_READ_XFER_MODE, 50);
    
    int i = 0;
    for (; i < 5; i++) {
        I2C_I2CMasterReadByte(I2C_I2C_ACK_DATA, &data[i], 50); 
    }
    I2C_I2CMasterReadByte(I2C_I2C_NAK_DATA, &data[++i], 50);
    I2C_I2CMasterSendStop(50);
    
    int x = (data[1] | (data[0] << 8));
    int y = (data[5] | (data[4] << 8));
    int z = (data[3] | (data[2] << 8));
    
    magData->x = x / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
    magData->y = y / _hmc5883_Gauss_LSB_XY * SENSORS_GAUSS_TO_MICROTESLA;
    magData->z = z / _hmc5883_Gauss_LSB_Z * SENSORS_GAUSS_TO_MICROTESLA;
    
}
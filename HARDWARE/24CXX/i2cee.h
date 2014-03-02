#ifndef __I2C_EE_H
#define	__I2C_EE_H

#include "stm32f10x.h"

/* EEPROM Addresses defines */
#define EEPROM_Block0_ADDRESS 0xA0   /* E2 = 0 */
//#define EEPROM_Block1_ADDRESS 0xA2 /* E2 = 0 */
//#define EEPROM_Block2_ADDRESS 0xA4 /* E2 = 0 */
//#define EEPROM_Block3_ADDRESS 0xA6 /* E2 = 0 */

void I2C_EE_Init(void);
void I2C_EE_BufferWrite(u8* pBuffer, u8 WriteAddr, u16 NumByteToWrite);
void I2C_EE_ByteWrite(u8* pBuffer, u8 WriteAddr);
void I2C_EE_PageWrite(u8* pBuffer, u8 WriteAddr, u8 NumByteToWrite);
void I2C_EE_BufferRead(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead);
void I2C_EE_WaitEepromStandbyState(void);
//外部调用
void I2C1_WriteByte(u8 write_address,u8 byte);
u8 I2C_EE_ReadByte(u8 read_address);
void I2C_EE_WriteNLenByte(u8 WriteAddr,u32 DataToWrite,u8 Len);
u32 I2C_EE_ReadLenByte(u8 ReadAddr,u8 Len);



#endif /* __I2C_EE_H */

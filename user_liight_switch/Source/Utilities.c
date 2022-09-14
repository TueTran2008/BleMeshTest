/******************************************************************************
 * @file    	Utilities.c
 * @author  	
 * @version 	V1.0.0
 * @date    	15/01/2014
 * @brief   	
 ******************************************************************************/


/******************************************************************************
                                   INCLUDES					    			 
 ******************************************************************************/
#include "utilities.h"

/******************************************************************************
                                   GLOBAL VARIABLES					    			 
 ******************************************************************************/


/******************************************************************************
                                   GLOBAL FUNCTIONS					    			 
 ******************************************************************************/


/******************************************************************************
                                   DATA TYPE DEFINE					    			 
 ******************************************************************************/
//#define ON		0
//#define OFF		1
//#define BeepCountLength 	7

/******************************************************************************
                                   PRIVATE VARIABLES					    			 
 ******************************************************************************/
//static uint8_t BeepCount = 0;
//static uint8_t tmpBeepCount = 0;
//static uint16_t BeepLength = 0;

/******************************************************************************
                                   LOCAL FUNCTIONS					    			 
 ******************************************************************************/


uint8_t CopyParameter(char* BufferSource, char* BufferDes, char FindCharBegin, char FindCharEnd)
{
    int16_t ViTriBatDau = FindIndexOfChar(FindCharBegin, BufferSource);
    int16_t ViTriKetThuc = FindIndexOfChar(FindCharEnd, BufferSource);
    int16_t tmpCount, i = 0;

    /* Kiem tra dinh dang du lieu */
    if(ViTriBatDau == -1 || ViTriKetThuc == -1) return 0;
    if(ViTriKetThuc <= 1 + ViTriBatDau) return 0;
	if(ViTriKetThuc >= 199 + ViTriBatDau) return 0;

    for(tmpCount = ViTriBatDau + 1; tmpCount < ViTriKetThuc; tmpCount++)
    {
        BufferDes[i++] = BufferSource[tmpCount];
    }

    BufferDes[i] = 0;

    return 1;
}

int16_t FindIndexOfChar(char CharToFind, char *BufferToFind)
{
    uint8_t tmpCount = 0;
    uint16_t DoDai = 0;

    /* Do dai du lieu */
    DoDai = strlen(BufferToFind);

    for(tmpCount = 0; tmpCount < DoDai; tmpCount++)
    {
        if(BufferToFind[tmpCount] == CharToFind) return tmpCount;
    }
    return -1;
}
/******************************************************************************************************************/
uint32_t GetHexNumberFromString(uint16_t BeginAddress, char* Buffer)
{
    uint32_t Value = 0;
    uint16_t tmpCount = 0;

    tmpCount = BeginAddress;
    Value = 0;
    while(Buffer[tmpCount] && tmpCount < 1024)
    {
        if((Buffer[tmpCount] >= '0' && Buffer[tmpCount] <= '9') || (Buffer[tmpCount] >= 'A' && Buffer[tmpCount] <= 'F') || (Buffer[tmpCount] >= 'a' && Buffer[tmpCount] <= 'f'))
        {
            Value *= 16;

            if(Buffer[tmpCount] == '1') Value += 1;
            if(Buffer[tmpCount] == '2') Value += 2;
            if(Buffer[tmpCount] == '3') Value += 3;
            if(Buffer[tmpCount] == '4') Value += 4;
            if(Buffer[tmpCount] == '5') Value += 5;
            if(Buffer[tmpCount] == '6') Value += 6;
            if(Buffer[tmpCount] == '7') Value += 7;
            if(Buffer[tmpCount] == '8') Value += 8;
            if(Buffer[tmpCount] == '9') Value += 9;

            if(Buffer[tmpCount] == 'A' || Buffer[tmpCount] == 'a') Value += 10;
            if(Buffer[tmpCount] == 'B' || Buffer[tmpCount] == 'b') Value += 11;
            if(Buffer[tmpCount] == 'C' || Buffer[tmpCount] == 'c') Value += 12;
            if(Buffer[tmpCount] == 'D' || Buffer[tmpCount] == 'd') Value += 13;
            if(Buffer[tmpCount] == 'E' || Buffer[tmpCount] == 'e') Value += 14;
            if(Buffer[tmpCount] == 'F' || Buffer[tmpCount] == 'f') Value += 15;
        }
        else break;

        tmpCount++;
    }

    return Value;
}
/******************************************************************************************************************/

/*
 * 	Ham doc mot so trong chuoi bat dau tu dia chi nao do.
 *	Buffer = abc124mff thi GetNumberFromString(3,Buffer) = 123
 *
 */
uint32_t GetNumberFromString(uint16_t BeginAddress, char* Buffer)
{
    uint32_t Value = 0;
    uint16_t tmpCount = 0;

    tmpCount = BeginAddress;
    Value = 0;
    while(Buffer[tmpCount] && tmpCount < 1024)
    {
        if(Buffer[tmpCount] >= '0' && Buffer[tmpCount] <= '9')
        {
            Value *= 10;
            Value += Buffer[tmpCount] - 48;
        }
        else break;

        tmpCount++;
    }

    return Value;
}

bool IsASCIIString(char* str)
{
	int leng = strnlen(str, 200);
	if(leng == 0) return false;
	
	for(int i = 0; i < leng; i++) {
		if(str[i] < 32 || str[i] > 126) {
			return false;
		}
	}
	return true;
}

bool IsDigitString(char* str)
{
	int leng = strnlen(str, 200);
	if(leng == 0) return false;
	
	for(int i = 0; i < leng; i++) {
		if(str[i] < '0' || str[i] > '9') {
			return false;
		}
	}
	return true;
}

#define ISO15693_PRELOADCRC16	0xFFFF 
#define ISO15693_POLYCRC16      0x8408 
#define ISO15693_MASKCRC16      0x0001
#define ISO15693_RESIDUECRC16   0xF0B8
 
uint16_t CalculateCRC16(uint8_t *DataIn, uint8_t NbByte)
{
	int16_t   i,j; 
	int32_t ResCrc = ISO15693_PRELOADCRC16;
            
	for (i = 0; i < NbByte; i++) 
	{ 
		ResCrc = ResCrc ^ DataIn[i];
		for (j = 8; j > 0; j--) 
		{
			ResCrc = (ResCrc & ISO15693_MASKCRC16) ? (ResCrc>>1) ^ ISO15693_POLYCRC16 : (ResCrc>>1); 
		}
	} 
 
	return ((~ResCrc) & 0xFFFF);
}
/********************************* END OF FILE *******************************/

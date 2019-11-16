#pragma once

#define DISPLAY_SPI
//LCD Hardware routines

void HwLcdInit();
//Подождать несколько милисекунд
void DelaySome();

//Управление пинами 0 - low, 1 - high
void HwLcdPinCE(uint8_t on);
void HwLcdPinDC(uint8_t on);
void HwLcdPinRst(uint8_t on);
void HwLcdPinLed(uint8_t on);
void HwLcdSend(uint8_t data); //send spi data
uint8_t HwLcdSendReceive(uint8_t data); //send and receive spi data

void HwLcdSend16NoWait(uint16_t data);
void HwLcdWait();//Wait send complete

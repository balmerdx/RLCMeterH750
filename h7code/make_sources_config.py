#
# "base" - base directory
# "dirs" - directiry with sources, all source files in this directory added (without recursion)
# "files" - raw list of source files
# "output" - output path prefix

import sys
sys.path.insert(0, '/home/balmer/radio/stm32/projects/BalmerVNA/make_py')
import make_sources

STM_LIB_DIR = "/home/balmer/radio/stm32/new_lib/l0/STM32Cube_FW_L0_V1.10.0/Drivers/STM32L0xx_HAL_Driver"
QT_PROJECT_NAME = "RLCMeterH750"

sources = [
{
	"base" : ".",
	"output" : "base",
	"dirs" : [
		"Src",
		],
	"files" : [
		"startup_stm32h743xx.s"
		]
},
{
	"base" : "Middlewares/ST/STM32_USB_Device_Library/Core/Src",
	"output" : "usb",
	"files" : [
		"usbd_core.c",
		"usbd_ctlreq.c",
		"usbd_ioreq.c",
		]

},
{
	"base" : "Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src",
	"output" : "usb_cdc",
	"files" : [
		"usbd_cdc.c",
		]

},
{
	"base" : "Drivers/STM32H7xx_HAL_Driver/Src",
	"output" : "hal",
	"files" : [
		"stm32h7xx_hal_pcd.c",
		"stm32h7xx_hal_pcd_ex.c",
		"stm32h7xx_ll_usb.c",
		"stm32h7xx_hal_rcc.c",
		"stm32h7xx_hal_rcc_ex.c",
		"stm32h7xx_hal_flash.c",
		"stm32h7xx_hal_flash_ex.c",
		"stm32h7xx_hal_gpio.c",
		"stm32h7xx_hal_hsem.c",
		"stm32h7xx_hal_dma.c",
		"stm32h7xx_hal_dma_ex.c",
		"stm32h7xx_hal_mdma.c",
		"stm32h7xx_hal_pwr.c",
		"stm32h7xx_hal_pwr_ex.c",
		"stm32h7xx_hal_cortex.c",
		"stm32h7xx_hal.c",
		"stm32h7xx_hal_i2c.c",
		"stm32h7xx_hal_i2c_ex.c",
		"stm32h7xx_hal_exti.c",
		"stm32h7xx_hal_eth.c",
		"stm32h7xx_hal_eth_ex.c",
		"stm32h7xx_hal_tim.c",
		"stm32h7xx_hal_tim_ex.c",
		"stm32h7xx_hal_uart.c",
		"stm32h7xx_hal_uart_ex.c",
		]

},
]


if __name__ == "__main__":
	make_sources.makeProject(sources, QT_PROJECT_NAME)

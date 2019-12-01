#!/usr/bin/env python3

import time
import serial
import sys
import struct

ser = None

def connect():
	global ser
	ser = serial.Serial(
		port='/dev/ttyACM0',#UART
		#port='/dev/ttyACM1',#USB
		baudrate=115200,
		#baudrate=500000,
		parity=serial.PARITY_NONE,
		stopbits=serial.STOPBITS_ONE,
		bytesize=serial.EIGHTBITS,
		timeout = 0.2
	)

	return ser.isOpen()

def close():
	ser.close()

def ReceiveData():
	ser.write([1])
	print("Data query.")
	f = open("data.bin", "wb")
	sum_size = 0
	is_empty = False
	while True:
		data = ser.read_all()
		if len(data)==0:
			print(".", end='')
			sys.stdout.flush()
			time.sleep(1)
			continue

		if is_empty:
			print("")
		is_empty = False
		#print("value=", hex(data[0]))
		#print("value=", data.decode("utf-8"))
		print("len(data)=", len(data))
		f.write(data)
		sum_size += len(data)
		if sum_size>=40000:
			break
		time.sleep(0.01)
	f.close()
	print("Data received complete. size=", sum_size)
	return


if __name__ == "__main__":
	if not connect():
		print("Cannot connect to serial port")
		exit(1)

	if 1:
		ReceiveData()
		#ser.write([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
		#ser.write([i+0x10 for i in range(16)])
		is_empty = False
		while True:
			data = ser.read_all()
			if len(data)==0:
				print(".", end='')
				sys.stdout.flush()
				time.sleep(1)
				continue

			if is_empty:
				print("")
			is_empty = False
			#print("value=", hex(data[0]))
			#print("value=", data.decode("utf-8"))
			if(len(data)>256):
				print("len(data)=", len(data))
			else:
				print("value=", data)
			time.sleep(0.1)
			 
		exit(1)

	time.sleep(0.01)
	pass

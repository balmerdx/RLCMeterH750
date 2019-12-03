#!/usr/bin/env python3

import time
import serial
import sys
import struct
import xml.etree.ElementTree as ET

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

def ReceiveData(freq=2123):
	ser.write(struct.pack("II",0xABDE0002, freq))
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
		print("len(data)=", len(data))
		f.write(data)
		sum_size += len(data)
		if sum_size>=40000:
			break
		time.sleep(0.01)
	f.close()
	print("Data received complete. size=", sum_size)
	return

def ReceiveConvolutionInternal(freq_list):
	assert(len(freq_list)<=16)
	print("freq_list=", freq_list)
	data = []
	for freq in freq_list:
		data += struct.pack("II",0xABDE0001, freq)
	ser.write(data)
	print("len_sended=", len(data))
	request_received = len(freq_list)*16
	is_empty = False
	data_sum = bytearray()
	while True:
		data = ser.read_all()
		if len(data)==0:
			print(".", end='')
			sys.stdout.flush()
			time.sleep(0.1)
			continue
		is_empty = False
		data_sum += data
		if len(data_sum)>=request_received:
			break
		time.sleep(0.01)

	freq_offset = 0
	out = []
	for (x,y) in struct.iter_unpack("dd", data_sum):
		s = {}
		s["f"] = freq_list[freq_offset]
		s["re"] = x
		s["im"] = y
		print("f=", s["f"], "Zx=", s["re"], s["im"])
		freq_offset += 1
		out.append(s)
	print("ReceiveConvolution complete. size=", len(data_sum))
	return out


def ReceiveConvolution(freq_list):
	max_per_bath = 16

	data = []
	for i in range(0, len(freq_list), max_per_bath):
		iend = min(i+max_per_bath, len(freq_list))
		data += ReceiveConvolutionInternal(freq_list[i:iend])

	et_data = ET.Element('data')
	for d in data:
		et_c = ET.SubElement(et_data, 'c')
		for key,value in d.items():
			et_c.set(key, str(value))

	mydata = ET.tostring(et_data)
	myfile = open("convolution.xml", "wb")
	myfile.write(mydata)


if __name__ == "__main__":
	if not connect():
		print("Cannot connect to serial port")
		exit(1)

	if 1:
		ReceiveData(freq=5000)
		#ReceiveConvolution(list(range(5000, 505000, 5000)))
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

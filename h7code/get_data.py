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

def ReceiveData(freq, filename):
	ser.write(struct.pack("II",0xABDE0002, freq))
	print("Data query.")
	f = open(filename, "wb")
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

AD9833_MASTER_CLOCK = 25000000

def AD9833_CalcFreqWorld(freq):
    freq_reg = (freq*(1<<28)+AD9833_MASTER_CLOCK/2)/AD9833_MASTER_CLOCK
    return int(freq_reg)

def AD9833_CalcFreq(freq_word):
    return (AD9833_MASTER_CLOCK*float(freq_word))/(1<<28)

def fixFreq(f):
	freq_word = AD9833_CalcFreqWorld(f)
	return AD9833_CalcFreq(freq_word)


def ReceiveConvolutionInternal(freq_list):
	data_size = 7*4
	assert(len(freq_list)<=data_size)
	print("freq_list=", freq_list)
	data = []
	for freq in freq_list:
		data += struct.pack("II",0xABDE0001, freq)
	ser.write(data)
	print("len_sended=", len(data))
	request_received = len(freq_list)*data_size
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
	print(" ")
	for (real_freq, re, im, a_re, a_im, b_re, b_im) in struct.iter_unpack("fffffff", data_sum):
		s = {}
		s["f"] = real_freq
		s["re"] = re
		s["im"] = im
		s["a_re"] = a_re
		s["a_im"] = a_im
		s["b_re"] = b_re
		s["b_im"] = b_im
		print("f=", s["f"], "Zx=", s["re"], s["im"])
		freq_offset += 1
		out.append(s)
	print("ReceiveConvolution complete. size=", len(data_sum))
	return out


def ReceiveConvolution(freq_list, filename):
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
	myfile = open(filename, "wb")
	myfile.write(mydata)

def sendReciveInfinite():
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
	return

g_standart_freq = [
    10, 12, 15, 18, 22, 27, 33, 39, 47, 56, 68, 82,
    100, 120, 150, 180, 220, 270, 330, 390, 470, 560, 680, 820,
    1000, 1200, 1500, 1800, 2200, 2700, 3300, 3900, 4700, 5600, 6800, 8200,
    10000, 12000, 15000, 18000, 22000, 27000, 33000, 39000, 47000, 56000, 68000, 82000,
    100000, 120000, 150000, 180000, 220000,
    270000, 330000, 390000, 470000, 500000
];

def FreqListX(mul_count=3):
	out = []
	for i in range(len(g_standart_freq)-1):
		fmin = g_standart_freq[i]
		fmax = g_standart_freq[i+1]
		fprev = fmin-1
		for j in range(mul_count):
			a = j/mul_count
			fcur = int(fmin*(1-a)+fmax*a)
			if fcur==165: #почемуто именно на этой частоте косяк с калибровкой (на частотах 164 и 166 все нормально)
				continue
			if fcur>fprev:
				out.append(fcur)
				fprev = fcur
	out.append(g_standart_freq[-1])
	return out			


def main():
	if not connect():
		print("Cannot connect to serial port")
		exit(1)

	if len(sys.argv)>1:
		a1 = sys.argv[1]
		if a1[0]=='s':
			freq_list = g_standart_freq
			if a1[0:2]=='s=':
				mul = int(a1[2:])
				print("mul=", mul)
				freq_list = FreqListX(mul)
			filename = "convolution.xml"
			if len(sys.argv)>2:
				filename = sys.argv[2]
			print("filename=", filename)
			#print("freq_list=", freq_list)
			#freq_list = [x for x in range(210, 600, 3)]
			#freq_list = [x for x in range(100, 200, 3)] + [x for x in range(200, 1000, 10)]
			#freq_list = [x for x in range(2500, 5000, 20)]
			ReceiveConvolution(freq_list, filename)
			return

		if a1[0:2]=='f=':
			freq = int(a1[2:])
			print("freq=", freq)
			filename = "data.bin"
			if len(sys.argv)>2:
				filename = sys.argv[2]
			print("filename=",filename)
			ReceiveData(freq=freq, filename=filename)
	else:
		sendReciveInfinite()
	pass

def help():
	print("Receive data from RLCMeterH750")
	print("get_data.py s out_file.xml - receive Zx from standart frequency list")
	print("get_data.py s=4 out_file.xml - receive Zx from x4 count frequencies")
	print("get_data.py f=1000 out_file.bin - receive sin data from I/V channel of frequency 1000 Hz")

if __name__ == "__main__":
	if len(sys.argv)==1:
		help()
	#print(sys.argv)
	#print(g_standart_freq)
	#print(FreqListX(4))
	main()

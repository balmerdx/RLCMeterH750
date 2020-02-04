#!/usr/bin/env python3
import array
import math
import cmath
import sys
import matplotlib.pyplot as plt
from struct import iter_unpack
from scipy.fftpack import fft
import scipy as np
from sys import argv
import xml.etree.ElementTree as ET

TIME_STEP = 1/2e6

def CFR(F, R):
	'''
	F - Herz
	C - Farad
	R - Om
	to complex mul to -j
	'''
	return -1/(2*math.pi*F*R)

def LFR(F, R):
	'''
	F - Herz
	L - Henri
	R - Om
	to complex mul to +j
	'''
	return R/(2*math.pi*F)


def makeTimeList(readableData, xmin, xstep):
	N = len(readableData)
	return np.linspace(start = xmin, stop = xstep*(N-1), num = N)

def readXml(fileName):
	tree = ET.parse(fileName)
	root = tree.getroot()
	f = []
	re = []
	im = []
	a_abs = []
	b_abs = []
	for et_data in root.iter('data'):
		for et_h in et_data.iter('c'):
			a = et_h.attrib
			f.append(float(a['f']))
			re.append(float(a['re']))
			im.append(float(a['im']))
			if 'a_re' in a:
				a_abs.append(abs(complex(float(a['a_re']),float(a['a_im']))))
				b_abs.append(abs(complex(float(a['b_re']),float(a['b_im']))))
			
	return { 'f': f, 're': re, 'im': im, 'a_abs': a_abs, 'b_abs': b_abs}

def readDataInt(filename="data.bin"):
	data_x = []
	data_y = []
	f = open(filename, "rb")
	data = f.read()
	f.close()

	for (x,y) in iter_unpack("HH", data):
		data_x.append(x)
		data_y.append(y)
	return (data_x, data_y)


def readData(filename="data.bin"):
	(data_x, data_y) = readDataInt(filename)
	return (np.asarray(data_x, dtype=np.float32), np.asarray(data_y, dtype=np.float32))

def plotXY(data_x, data_y):
	fig, ax = plt.subplots()

	#timeList = makeTimeList(data_x, 0, TIME_STEP)
	timeList = makeTimeList(data_x, 0, 1)

	ax.plot(timeList, data_x, color='red')
	ax.plot(timeList, data_y, color='blue')
	plt.show()

def removeAverage(data):
	x = np.average(data)
	return np.subtract(data, x)


def makeFFT(data):
	N = len(data)
	yf = fft(data)
	yf_abs = 2.0/N * np.absolute(yf[0:N//2])
	yf_dbfs = 20 * np.log10(yf_abs)
	ref_db = 86
	yf_dbfs = np.subtract(yf_dbfs, ref_db)
	xf = np.linspace(0.0, 1.0/(2.0*TIME_STEP), N//2)
	return (xf, yf_dbfs)


def plotFFT(data, color = 'blue'):
	(xf, yf) = makeFFT(data)

	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('dB(fft)')
	ax.set_ylim(bottom=-120, top=0)
	ax.plot(xf, yf, color=color)
	plt.grid()
	plt.show()

def plotFFT2(data1, data2):
	(xf1, yf1) = makeFFT(data1)
	(xf2, yf2) = makeFFT(data2)

	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('dB(fft)')
	ax.set_ylim(bottom=-120, top=0)
	ax.plot(xf1, yf1, color='red')
	ax.plot(xf2, yf2, color='blue')
	plt.grid()
	plt.show()

def normSin(arr, freq, step):
	'''
	Вычисляет перемноженное с синусом и косинусом значение
	'''
	fstep = 2*math.pi*step*freq
	asum = 0.
	bsum = 0.
	count = len(arr)
	for i in range(count):
		x = arr[i]
		asum += x*math.cos(fstep*i)
		bsum += x*math.sin(fstep*i)
	return complex(asum/count, bsum/count)

samples_per_second = 22e6/(8.5+8.5)
def calcSinParams(data_x, data_y, freq):
	step = 1/samples_per_second
	samples_per_cycle = int(samples_per_second/freq)
	print("samples_per_cycle=", samples_per_cycle)
	zx = normSin(data_x[0:samples_per_cycle], freq, step)
	zy = normSin(data_y[0:samples_per_cycle], freq, step)
	return (zx,zy)

def checkBits(data, text):
	#ищем неизменные биты.
	#если бит не изменяется - это косяк
	#не работает эта проверка, не всегда бит 

	bits_and = 0xFFFF
	bits_or = 0
	for x in data:
		bits_and = bits_and & x
		bits_or = bits_or | x

	for i in range(16):
		if (bits_and & (1<<i)):
			print(text, ": bit", i, " is 1 permanently")
		if (bits_or & (1<<i))==0:
			print(text, ": bit", i, " is 0 permanently")
	return

def ViewDataBin(filename):
	(data_xi, data_yi) = readDataInt(filename)
	checkBits(data_xi, "X")
	checkBits(data_yi, "Y")

	(data_x, data_y) = readData(filename)

	print(len(data_x), len(data_y))
	#print(data_x[0], data_y[0])

	#data_x = removeAverage(data_x)
	#data_y = removeAverage(data_y)

	#(zx, zy) = calcSinParams(data_x, data_y, freq=150)
	#print("zx=", abs(zx), "zy=", abs(zy), "phase=", cmath.phase(zx/zy)*180/math.pi)

	plotXY(data_x, data_y)
	#plotFFT(data_x, color = 'red')
	#plotFFT(data_y, color = 'blue')
	#plotFFT2(data_x, data_y)

def plotZ(f, re, im, xscale='log', ylabel='Z (Om)'):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel(ylabel)
	if xscale:
		ax.set_xscale(xscale)
	if re:
		ax.plot(f, re, color='red')
	if im:
		ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def plotY(f, re, im):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('Z (Cm)')
	ax.set_xscale('log')
	for i in range(len(f)):
		Z = complex(re[i],im[i])
		Y = 1/Z
		re[i] = Y.real
		im[i] = Y.imag

	if re:
		ax.plot(f, re, color='red')
	if im:
		ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def plotL(f, im):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('L (uH)')
	ax.set_xscale('log')

	for i in range(len(f)):
		im[i] = LFR(f[i], im[i])*1e6

	ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def plotС(f, re, im, prefix = None):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_xscale('log')
	c_max = 0

	for i in range(len(f)):
		im[i] = CFR(f[i], im[i])
		c_max = max(abs(im[i]), c_max)

	if not prefix:
		ax.set_ylabel('C (F)')
		mul = 1
	elif prefix == 'm':
		ax.set_ylabel('C (mF)')
		mul = 1e3
	elif prefix == 'u':
		ax.set_ylabel('C (uF)')
		mul = 1e6
	elif prefix == 'n':
		ax.set_ylabel('C (nF)')
		mul = 1e9
	elif prefix == 'p':
		ax.set_ylabel('C (pF)')
		mul = 1e12

	if c_max*mul > 1e3:
		ax.set_ylim(bottom=0, top=1000)
	#ax.set_ylim(bottom=0, top=100)

	for i in range(len(f)):
		im[i] = im[i]*mul

	ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def plotPhase(f, re, im):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('Phase')
	ax.set_xscale('log')

	for i in range(len(f)):
		im[i] = cmath.phase(complex(re[i], im[i]))*180/math.pi

	ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def plotAbs(f, re, im):
	fig, ax = plt.subplots()
	ax.set_xlabel('F (Hz)')
	ax.set_ylabel('Phase')
	ax.set_xscale('log')

	for i in range(len(f)):
		im[i] = abs(complex(re[i], im[i]))*180/math.pi

	ax.plot(f, im, color='blue')
	plt.grid()
	plt.show()

def ViewZx(filename):
	data = readXml(filename)
	f = data['f']
	re = data['re']
	im = data['im']

	serial = True
	if not serial:
		for i in range(len(f)):
			Z = complex(re[i],im[i])
			Y = 1/Z
			im[i] = -1/Y.imag
			re[i] = 1/Y.real

	#plotZ(f, re=re, im=im)
	#plotY(f, re, im)
	#plotZ(f, re=re, im=None)
	#plotZ(f, re=None, im=im)
	#plotL(f, im=im)
	plotС(f, re=re, im=im, prefix = 'n')
	#plotPhase(f, re=re, im=im)
	#plotAbs(f, re=re, im=im)

def ViewAbs(filename):
	data = readXml(filename)
	data = readXml(filename)
	f = data['f']
	a_abs = data['a_abs']
	b_abs = data['b_abs']
	plotZ(f, re=a_abs, im=b_abs, xscale=None, ylabel='a_b_abs')


if len(sys.argv)==1:
	print("view_data.py sinus.bin")
	print("view_data.py zx.xml")
	exit(1)

filename = sys.argv[1]

ext = filename[-4:]
if ext == ".bin":
	ViewDataBin(filename)

if ext == ".xml":
	if len(sys.argv)>2 and sys.argv[2]=='abs':
		ViewAbs(filename)
	else:
		ViewZx(filename)

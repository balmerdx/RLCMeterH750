import array
import math
import matplotlib.pyplot as plt
from struct import iter_unpack
from scipy.fftpack import fft
import scipy as np
from sys import argv

TIME_STEP = 1/2e6

def makeTimeList(readableData, xmin, xstep):
	'''
	xlist = []
	for i in range(0, len(readableData)):
		xlist.append(xmin+i*xstep)
	return xlist
	'''
	N = len(readableData)
	return np.linspace(start = xmin, stop = xstep*(N-1), num = N)


def readData(filename="data.bin"):
	data_x = []
	data_y = []
	f = open(filename, "rb")
	data = f.read()
	f.close()

	for (x,y) in iter_unpack("HH", data):
		data_x.append(x)
		data_y.append(y)
	return (np.asarray(data_x, dtype=np.float32), np.asarray(data_y, dtype=np.float32))

def plotXY(data_x, data_y):
	fig, ax = plt.subplots()

	timeList = makeTimeList(data_x, 0, TIME_STEP)

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

filename="data.bin"
if len(argv)>1:
	filename = argv[1]

(data_x, data_y) = readData(filename)

print(len(data_x), len(data_y))
#print(data_x[0], data_y[0])

data_x = removeAverage(data_x)
data_y = removeAverage(data_y)

#plotXY(data_x, data_y)

#plotFFT(data_x, color = 'red')
plotFFT(data_y, color = 'blue')
#plotFFT2(data_x, data_y)

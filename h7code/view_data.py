import array
import math
import matplotlib.pyplot as plt
from struct import iter_unpack

TIME_STEP = 1/2e6

def makeTimeList(readableData, xmin, xstep):
	xlist = []
	for i in range(0, len(readableData)):
		xlist.append(xmin+i*xstep)
	return xlist


def plotXY(data_x, data_y):
	fig, ax = plt.subplots()

	timeList = makeTimeList(data_x, 0, TIME_STEP)

	ax.plot(timeList, data_x, color='red')
	ax.plot(timeList, data_y, color='blue')
	plt.show()

def readData(filename="data.bin"):
	data_x = []
	data_y = []
	f = open(filename, "rb")
	data = f.read()
	f.close()

	for (x,y) in iter_unpack("HH", data):
		data_x.append(x)
		data_y.append(y)
	return (data_x, data_y)


(data_x, data_y) = readData()
print(len(data_x), len(data_y))
print(data_x[0], data_y[0])

plotXY(data_x, data_y)
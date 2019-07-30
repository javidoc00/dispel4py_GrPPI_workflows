# Get to stage where all is xc-d tehn append a new taskfarm according to what 
# is to be done with the xc's.
# and look at data from the shm in memory files.

import numpy as np
import time
import os, shutil
import traceback
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.backends
import scipy.fftpack as sci

file_temp = 'temp.npy'

file1 = 'prueba1.npy'
file2 = 'prueba2.npy'
file_fig = 'figure.png'
shift = 5000

data1=np.load(file1)
data2=np.load(file2)

print('data1: {:.2f}'.format(data1[1234]))
print('data2.conj: {:.2f}'.format(np.conjugate(data2)[1234]))
    
data = data1*np.conjugate(data2)

#data = np.load(file_temp)
#np.save(file_temp, data)

print('data: {:.2f}'.format(data[1234]))
ret = sci.ifft(data)
print('IFFT: {:.2f}'.format(ret[1234]))

ret = ret.real
print('REAL: {:.2f}'.format(ret[1234]))
xcorr1 = np.roll(ret, shift)[:2 * shift + 1]
print('ROLL: {:.2f}'.format(xcorr1[1234]))
print('ROLL: {:.2f}'.format(xcorr1[10]))
print('ROLL: {:.2f}'.format(xcorr1[2000]))

sps=4
plt.plot(np.linspace((-len(xcorr1)+1)/(2*sps),len(xcorr1)/(2*sps),len(xcorr1)), xcorr1)
plt.xlim(-1000,1000)
plt.savefig(file_fig)
plt.clf()

# Get to stage where all is xc-d tehn append a new taskfarm according to what 
# is to be done with the xc's.
# and look at data from the shm in memory files.

from dispel4py.core import GenericPE 
from dispel4py.workflow_graph import WorkflowGraph
from dispel4py.base import BasePE, IterativePE, ConsumerPE, create_iterative_chain

import numpy as np
import time
import os, shutil
import traceback
from tc_cross_correlation.xcorr import xcorrf, xcorrf_noFFT 
from  distutils.dir_util  import create_tree as mkadir
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.backends
from obspy import read, UTCDateTime

#ROOT_DIR = './tc_cross_correlation/OUTPUT/'
ROOT_DIR = os.environ['OUTPUT_DIR']

#starttime='2019-07-21T06:00:00.000'
starttime = os.environ['DISPEL4PY_XCORR_STARTTIME']

#endtime='2019-07-21T07:00:00.000'
endtime = os.environ['DISPEL4PY_XCORR_ENDTIME']

t_start = UTCDateTime(starttime)
t_finish = UTCDateTime(endtime)

class Product(GenericPE):
    def __init__(self):
        GenericPE.__init__(self)
        self._add_output('output', tuple_type=['number'])
    def process(self, inputs):
        lapse = int((t_finish - t_start) // 3600)
        for inc in range(0, lapse):
            store = []
            t_now = t_start + (inc*3600)
            dirs = [x for x in os.listdir(ROOT_DIR + 'DATA/'+ t_now.__str__()) if not x.startswith('.')]
            for dir in dirs:
                files = [x for x in os.listdir(ROOT_DIR+'/DATA/'+ t_now.__str__() +'/'+ dir) if not x.startswith('.')]
                for f in files:
                    file=ROOT_DIR+'DATA/'+ t_now.__str__() + '/'+ dir+'/'+f
                    str1=np.load(file)
                    index = len(store)
                    for i in range(index):
                        self.write('output',[i, index, store[i], str1, t_now])
                    #self.log('The station %s is index %s' %(f,index))
                    store.append(str1)

class Xcorr(IterativePE):
    def __init__(self):
        IterativePE.__init__(self)
    def _process(self, data):
        str1=data[2]
        str2=data[3]
        t_now=data[4]
        try:
            xcorr1 = xcorrf_noFFT(str1, str2, 5000)
            return (data[0], data[1], xcorr1, t_now)
        except:
            self.log('error in %s_%s xcorr' % (data[0], data[1]))
            self.log(traceback.format_exc())


class StoreToFile(ConsumerPE):
    def __init__(self, filename):
        ConsumerPE.__init__(self)
        self.filename = filename
        self.counter = 0
    def _process(self, data):
        xcorr1 = data[2]
        t_now=data[3]
        directory=ROOT_DIR+'XCORR/'+t_now.__str__()+'/'+str(data[0])+'_'+str(data[1])
        if not os.path.exists(directory):
    	    os.makedirs(directory)
        fout = directory+'/%s_%s_%s.out' % (self.filename, data[0], data[1])
        np.save(fout, xcorr1)
        self.counter += 1
            #self.log('wrote file %s' % fout)
	


class Plot(ConsumerPE):
    def __init__(self, filename):
        ConsumerPE.__init__(self)
        self.filename = filename
    def _process(self, data):
        xcorr1 = data[2]
        t_now=data[3]
        fout = ROOT_DIR+'XCORR/'+t_now.__str__()+'/' + '%s_%s_%s.plot.eps' % (self.filename, data[0], data[1])
        sps=4
        plt.plot(np.linspace((-len(xcorr1)+1)/(2*sps),len(xcorr1)/(2*sps),len(xcorr1)), xcorr1)
        plt.xlim(-1000,1000)
        plt.savefig(fout)
        plt.clf()



graph = WorkflowGraph()
product=Product()
xcorr1=Xcorr()
store=StoreToFile('Xcorr')
plot = Plot('Xcorr')
graph.connect(product, 'output', xcorr1, 'input')
graph.connect(xcorr1, 'output', store, 'input')
graph.connect(xcorr1, 'output', plot, 'input')


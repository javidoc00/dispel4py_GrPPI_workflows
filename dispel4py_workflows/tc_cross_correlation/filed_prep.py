from dispel4py.core import GenericPE
from dispel4py.base import BasePE, IterativePE, ConsumerPE, create_iterative_chain
from dispel4py.workflow_graph import WorkflowGraph

from obspy.clients.fdsn import Client
from obspy.signal.util import next_pow_2
from tc_cross_correlation.whiten import spectralwhitening_smooth
from tc_cross_correlation.normalization import onebit_norm, mean_norm, gain_norm, env_norm
from scipy.fftpack import fft
import numpy as np
from obspy.core import read
from obspy import UTCDateTime
from obspy.core.stream import Stream
from numpy import linspace
import os
import traceback
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt
import os
import pickle

class StreamProducer(IterativePE):

    def __init__(self, t_start, t_finish, channel):
        IterativePE.__init__(self)
        self.t_start=t_start
        self.t_finish=t_finish
        self.channel=channel
        self.counter = 0
        self.bulksize = 10
        
    def _process(self, filename):
      #self.log('reading inputs from %s' % filename)
        
      lapse = int((self.t_finish - self.t_start) // 3600)
      for aux in range(0, NUM_REP):
        for inc in range(0, lapse):
            t_now = t_start + (inc*3600)
            t_then = t_start + ((inc+1)*3600)
            dir = ROOT_DIR + 'SOURCE/'+ t_now.__str__()
            print (dir)
            if not os.path.exists(dir):
                os.makedirs(dir)
            
            if (LOAD_SAVE == 'LOAD'):
                print ('load')
                self._load_waveforms(dir, t_now)
            else:
                client = Client()
                stations_list = []
                stations_names = set()

                print ('save')
                with open(filename, 'r') as f:
                    i = 1
                    for station in f.readlines():
                        if i % self.bulksize != 0:
                            network, station_name = station.split()
                            stations_names.add((network, station_name))
                            # station_name = station.strip()
                            stations_list.append((network, station_name, "", self.channel, t_now, t_then))
                            i += 1
                        else:
                            if (LOAD_SAVE == 'SAVE'):
                                self._save_waveforms(client, stations_list, stations_names, dir, t_now)
                            else:
                                self._get_waveforms(client, stations_list, stations_names)
                            stations_list = []
                            stations_names = set()
                            i = 1
                if stations_list:
                    if (LOAD_SAVE == 'SAVE'):
                        self._save_waveforms(client, stations_list, stations_names, dir, t_now)
                    else:
                        self._get_waveforms(client, stations_list, stations_names)
                    #self.log('Downloaded %s waveforms' % self.counter)

    def _get_waveforms(self, client, stations_list, stations_names):
        try:
            # st = client.get_waveforms(network, station_name, "", self.channel, self.t_start, self.t_finish, attach_response=True)
            #self.log('Retrieving waveforms...')
            st = client.get_waveforms_bulk(stations_list, attach_response=True)
            for tr in st:
                network_name = tr.stats['network']
                station_name = tr.stats['station']
                stations_names.remove((network_name, station_name))
                #self.log('Found network %s, station %s, channel %s, t_start %s, t_finish %s' % (network_name, station_name, tr.stats['channel'], self.t_start, self.t_finish))
                self.write('output', [Stream(traces=[tr]), station_name, self.counter, t_now])
                self.counter += 1
            if stations_names:
                self.log('Failed to read stations %s' % stations_names)
        except:
            self.log('Failed to read stations XXXX %s' % stations_names)
            self.log(traceback.format_exc())
            pass

    def _save_waveforms(self, client, stations_list, stations_names, directory, t_now):
        try:
            # st = client.get_waveforms(network, station_name, "", self.channel, self.t_start, self.t_finish, attach_response=True)
            #self.log('Retrieving waveforms...')
            st = client.get_waveforms_bulk(stations_list, attach_response=True)
            for tr in st:
                network_name = tr.stats['network']
                station_name = tr.stats['station']
                stations_names.remove((network_name, station_name))
                #self.log('Found network %s, station %s, channel %s, t_start %s, t_finish %s' % (network_name, station_name, tr.stats['channel'], self.t_start, self.t_finish))
                fout = directory+'/%s_%s_downloaded.SAC' % (station_name, self.counter)
                self.save_trace ([Stream(traces=[tr]), station_name, self.counter, t_now], fout)
                self.counter += 1
            if stations_names:
                self.log('Failed to read stations %s' % stations_names)
        except:
            self.log('Failed to read stations XXXX %s' % stations_names)
            self.log(traceback.format_exc())
            pass

    def _load_waveforms(self, directory, t_now):
        try:
            dirs = [x for x in os.listdir(directory) if not x.startswith('.')]
            for file in dirs:
                self.write('output', self.load_trace(directory + "/" + file))
        except:
            self.log('Failed to read file XXXX %s' % file)
            self.log(traceback.format_exc())
            pass

    def save_trace(self, obj, name):
        with open(name + '.pkl', 'wb') as f:
            pickle.dump(obj, f, pickle.HIGHEST_PROTOCOL)

    def load_trace(self, name ):
        with open(name, 'rb') as f:
            return pickle.load(f)



class StreamToFile(ConsumerPE):
    def __init__(self):
        ConsumerPE.__init__(self)
    def _process(self, data):
        str1 = data[0]
        station= data[1]
        counter = data[2]
        t_now = data[3]
        dir = ROOT_DIR + 'DATA/'+ t_now.__str__()
        if not os.path.exists(dir):
            os.makedirs(dir)
        directory = dir +'/'+ station
        if not os.path.exists(directory):
            os.makedirs(directory)
        try:
            fout = directory+'/%s_%s_preprpcessed.SAC' % (station, counter)
        except TypeError:
            # maybe there's no "%s" in the string so we ignore count - bad idea?
            fout = file_dest
        np.save(fout, str1)

def factors(n):
    return [(i, n / i) for i in range(1, int(n**0.5)+1) if n % i == 0]

def decimate(str1, sps):
    str1.decimate(int(str1[0].stats.sampling_rate/sps))
    j = int(str1[0].stats.sampling_rate/sps)
    if j > 16:
        facts=factors(j)[-1]
        str1[0].decimate(facts[0])
        str1[0].decimate(facts[1])
    else:
        str1[0].decimate(j)
    return str1
def detrend(str1):
    str1.detrend('simple')
    return str1
def demean(str1):
    str1.detrend('demean')
    return str1
def remove_response(str1, pre_filt,units):
    str1.remove_response(output=units, pre_filt=pre_filt)        
    return str1
def filter(str1, freqmin=0.01, freqmax=1., corners=4, zerophase=False):
    str1.filter('bandpass', freqmin=freqmin, freqmax=freqmax, corners=corners, zerophase=zerophase)
    return str1
def calc_norm(str1,norm,N):   
    # Phase 1f: Apply one of the normalization methods
    if norm is 'onebit':
        #Method 1 - one-bit normalization
        str1 = onebit_norm(str1)
    
    elif norm is 'mean':
        #Method 2 - moving-average normalization
        str1 = mean_norm(str1,N)
        
    elif norm is 'gain':
        #Method 3 - "triangle" normalization
        str1 = gain_norm(str1,N)
    
    elif norm is 'env':
        #Method 3 - "triangle" normalization
        str1 = env_norm(str1,N)
    return str1

def whiten(str1,smooth):
    if smooth is not None:
         str1 = spectralwhitening_smooth(str1, smooth)
    return str1

def calc_fft(str1, type, shift):
    if type is not None:
        data = str1[0].data
        N1 = len(data)
        data = data.astype(type)
        # Always use 2**n-sized FFT, perform xcorr
        size = max(2 * shift + 1, (N1) + shift)
        nfft = next_pow_2(size)
        print ("station %s and network %s - size in calc_fft %s " % (str1[0].stats['station'], str1[0].stats['network'], size)) 
        #Calculate fft of data1 and data2
        IN1 = fft(data, nfft)
        return IN1

        
#        # test
#        data.resize(nfft);
#        data2 = data.astype('complex128')
#        data = fft(data,nfft)
#        data2 = fft(data2,nfft)
#        print (np.array_equal(data,data2))
#        for x in range(0,len(data)):
#            if (data[x] > data2[x] + 1.0E-10) and (data[x] < data2[x] - 1.0E-10):
#                print ('data: ({0.real:.20f} + {0.imag:.20f}i)'.format(data[x]))
#                print ('data2: ({0.real:.20f} + {0.imag:.20f}i)'.format(data2[x]))
#        return data2
#        # end test
    else:
        return str1[0].data    
 
class PreTaskPE(IterativePE):
    def __init__(self, compute_fn=None, params={}):
        IterativePE.__init__(self)
        self._add_output('output', tuple_type=['result', 'station', 'counter', 't_now'])
        self.compute_fn = compute_fn
        self.params = params
    def _process(self, data):
        str1 = data[0]
        station = data[1]
        counter = data[2]
        t_now = data[3]
        try:
           result = self.compute_fn(str1, **self.params)
           return [result, station, counter, t_now]
        except:
           self.log(traceback.format_exc())
           self.log ("Getting an error in preprocess: %s-%s" % (str1[0].stats['station'], str1[0].stats['network']))
           return None

LOAD_SAVE = os.environ['LOAD_SAVE']

#ROOT_DIR = './tc_cross_correlation/OUTPUT/'
ROOT_DIR = os.environ['OUTPUT_DIR']
NUM_REP = int(os.environ['NUM_REP'])

#starttime='2019-07-21T06:00:00.000'
starttime = os.environ['DISPEL4PY_XCORR_STARTTIME']

#endtime='2019-07-21T07:00:00.000'
endtime = os.environ['DISPEL4PY_XCORR_ENDTIME']

t_start = UTCDateTime(starttime)
t_finish = UTCDateTime(endtime)
channel='BHZ'
streamProducer=StreamProducer(t_start, t_finish, channel)
streamProducer.name = 'streamProducer'
streamToFile = StreamToFile()
streamToFile.name='StreamToFile'
functions = [ 
                (decimate, { 'sps' : 4 }), 
                detrend, 
                demean, 
                (remove_response, { 'pre_filt' : (0.005, 0.006, 30.0, 35.0),'units': 'VEL' }),
                (filter, {'freqmin':0.01, 'freqmax':1., 'corners':4, 'zerophase':False}),
                (calc_norm, { 'norm':'env','N' : 15 }),
                (whiten, {'smooth':None}),
                (calc_fft, {'type':'float64', 'shift':5000})
            ]
preTask = create_iterative_chain(functions, PreTaskPE)
graph = WorkflowGraph()

graph.connect(streamProducer, StreamProducer.OUTPUT_NAME, preTask, 'input')
graph.connect(preTask, 'output', streamToFile, StreamToFile.INPUT_NAME)


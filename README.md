# Seismic Noise Cross-Correlation codes (dispel4py/GrPPI version)

You need to install the following libraries: Eigen3, FFTW3, MathGL. For Ubuntu Linux just execute: 

    sudo apt install libeigen3-dev
    sudo apt install libfftw3-dev
    sudo apt install mathgl

More information about Eigen3 can be found [here](http://eigen.tuxfamily.org/index.php?title=Main_Page).
More information about FFTW3 can be found [here](http://www.fftw.org/).
More information about mathgl can be found [here](http://mathgl.sourceforge.net/doc_en/Main.html#Main).

Also, it is recommended to install Intel TBB library for better performance. For Ubuntu Linux just execute: 

    sudo apt install libtbb-dev

More information about Intel TBB can be found [here](https://www.threadingbuildingblocks.org/).

## Compilation of the GrPPI part

The easiest way to compile The GrPPI code is using an *out-of-source*
directory under the GrPPI root directory:

~~~ 
mkdir grppi_workflows/build
cd grppi_workflows/build
~~~

Then, you may generate the scripts by just doing:

~~~
cmake ..
~~~

Afterwards, you launch the build process with:

~~~
make
~~~

Finally you copy the executable to the dispel4py tc_cross_correlation directory

~~~
cd ../..
cp grppi_workflows/build/use_cases/tc_cross_correlation/tc_cross_correlation dispel4py_workflows/tc_cross_correlation/
~~~

## Execution of the dispel4py/GrPPI version


- The first workflow (realtime_prep.py) stores the results in a directory called OUTPUT/DATA. 
- The second one (tc_cross_correlation) stores the results in OUTPUT/XCORR directory. 
- This is our script for executing both workflows with multi mapping (dispel4py) and TBB mapping (GrPPI): 
	

    	export DISPEL4PY_XCORR_STARTTIME=2015-04-06T06:00:00.000
    	export DISPEL4PY_XCORR_ENDTIME=2015-04-06T07:00:00.000
    	rm -rf ./tc_cross_correlation/OUTPUT/DATA
    	rm -rf ./tc_cross_correlation/OUTPUT/XCORR
    	mkdir  ./tc_cross_correlation/OUTPUT/DATA
    	mkdir ./tc_cross_correlation/OUTPUT/XCORR

    	dispel4py multi tc_cross_correlation/realtime_prep.py -f tc_cross_correlation/realtime_xcorr_input.jsn -n 4
    	tc_cross_correlation/tc_cross_correlation ./tc_cross_correlation/OUTPUT/ $DISPEL4PY_XCORR_STARTTIME tbb

- The last step is to open the file " tc_cross_correlation/realtime_xcorr_input.jsn” and change the path of the file Copy-Uniq-OpStationList-NetworkStation.txt

		xxx/tc_cross_correlation/Copy-Uniq-OpStationList-NetworkStation.txt  


- You could change the realtime_xcorr_input.jsn for using Uniq-OpStationList-NetworkStation.txt (it contains all the stations) instead of Copy-Uniq-OpStationList-NetworkStation.txt (it contains only a few of stations for testing the workflow). This data has been obtained from the [IRIS](http://ds.iris.edu/ds/nodes/dmc/earthscope/usarray/_US-TA-operational/) website. 

		{
    		"streamProducer" : [ { "input" : “/xxxxxxxxx/tc_cross_correlation/Uniq-OpStationList-NetworkStation.txt" } ]
		}


- It is important that you delete DATA and XCORR directories every time before starting to run your workflows. 

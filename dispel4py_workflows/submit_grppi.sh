set -x

  #export WORKFLOWS_DIR="/Users/javierfernandezmunoz/EPCC/version_buena/"
  export WORKFLOWS_DIR="/lustre/home/dc010/jfmunoz/test/dispel4py_GrPPI_workflows/"

  export NUM_CORES=10
  export LOAD_SAVE="SAVE"  # [SAVE, LOAD]
  export OPERATION="PREP"  # [ALL, PREP, XCROSS]

  export PYTHONPATH=$PYTHONPATH:.
  export GRPPI_DIR=${WORKFLOWS_DIR}/grppi_workflows/build/use_cases/tc_cross_correlation/
  export OUTPUT_DIR=${WORKFLOWS_DIR}/dispel4py_workflows/tc_cross_correlation/OUTPUT/
  export DISPEL4PY_XCORR_STARTTIME=2019-07-21T00:00:00.000
  export DISPEL4PY_XCORR_ENDTIME=2019-07-25T05:00:00.000

  if [ "$LOAD_SAVE" == "SAVE" ]; then
    rm -rf ./tc_cross_correlation/OUTPUT/SOURCE
    mkdir  ./tc_cross_correlation/OUTPUT/SOURCE
  fi

  if [ "$OPERATION" != "XCROSS" ]; then
    rm -rf ./tc_cross_correlation/OUTPUT/DATA
    mkdir  ./tc_cross_correlation/OUTPUT/DATA
    dispel4py multi tc_cross_correlation/filed_prep.py -f tc_cross_correlation/realtime_xcorr_input.jsn -n $NUM_CORES
  fi

  if [ "$OPERATION" != "PREP" ]; then
    rm -rf ./tc_cross_correlation/OUTPUT/XCORR
    mkdir ./tc_cross_correlation/OUTPUT/XCORR
    #dispel4py multi tc_cross_correlation/realtime_xcorr_mod.py -n $NUM_CORES
    #taskset -c 0-$(($NUM_CORES-1)) ${GRPPI_DIR}/tc_cross_correlation $OUTPUT_DIR $DISPEL4PY_XCORR_STARTTIME $DISPEL4PY_XCORR_ENDTIME tbb
    ${GRPPI_DIR}/tc_cross_correlation $OUTPUT_DIR $DISPEL4PY_XCORR_STARTTIME $DISPEL4PY_XCORR_ENDTIME tbb
  fi

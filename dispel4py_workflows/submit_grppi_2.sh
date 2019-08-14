set -x

  export WORKFLOWS_DIR="/Users/javierfernandezmunoz/EPCC/version_buena/"
  #export WORKFLOWS_DIR="/lustre/home/dc010/jfmunoz/test/dispel4py_GrPPI_workflows/"

  export NUM_CORES=10
  export NUM_REP=2
  export LOAD_SAVE="LOAD"  # [SAVE, LOAD]
  export OPERATION="ALL"  # [ALL, PREP, XCROSS]

  export PYTHONPATH=$PYTHONPATH:.
  export GRPPI_DIR=${WORKFLOWS_DIR}/grppi_workflows/build/use_cases/tc_cross_correlation/
  export OUTPUT_DIR=${WORKFLOWS_DIR}/dispel4py_workflows/tc_cross_correlation/OUTPUT/
  export DISPEL4PY_XCORR_STARTTIME=2019-07-21T06:00:00.000
  export DISPEL4PY_XCORR_ENDTIME=2019-07-21T08:00:00.000

  #for NUM_REP in 1 2 4 6 8 16 32 64; do
  for NUM_REP in 1 2; do

    echo "BEGIN: NUM_REP = $NUM_REP"
    if [ "$LOAD_SAVE" == "SAVE" ]; then
      rm -rf ./tc_cross_correlation/OUTPUT/SOURCE
      mkdir  ./tc_cross_correlation/OUTPUT/SOURCE
    fi

    if [ "$OPERATION" != "XCROSS" ]; then
      rm -rf ./tc_cross_correlation/OUTPUT/DATA
      mkdir  ./tc_cross_correlation/OUTPUT/DATA
      echo "BEGIN: prep-python NUM_REP = $NUM_REP"
      dispel4py multi tc_cross_correlation/filed_prep.py -f tc_cross_correlation/realtime_xcorr_input2.jsn -n $NUM_CORES
      echo "END: prep-python NUM_REP = $NUM_REP"
    fi

    if [ "$OPERATION" != "PREP" ]; then
      rm -rf ./tc_cross_correlation/OUTPUT/XCORR
      mkdir ./tc_cross_correlation/OUTPUT/XCORR
      echo "BEGIN: xcross-python NUM_REP = $NUM_REP"
      dispel4py multi tc_cross_correlation/realtime_xcorr_mod.py -n $NUM_CORES
      echo "END: xcross-python NUM_REP = $NUM_REP"
      echo "BEGIN: xcross-c++ NUM_REP = $NUM_REP"
      taskset -c 0-$(($NUM_CORES-1)) ${GRPPI_DIR}/tc_cross_correlation $OUTPUT_DIR $DISPEL4PY_XCORR_STARTTIME $DISPEL4PY_XCORR_ENDTIME $NUM_REP tbb
      #${GRPPI_DIR}/tc_cross_correlation $OUTPUT_DIR $DISPEL4PY_XCORR_STARTTIME $DISPEL4PY_XCORR_ENDTIME $NUM_REP tbb
      echo "END: xcross-c++ NUM_REP = $NUM_REP"
    fi
    echo "END: NUM_REP = $NUM_REP"

  done

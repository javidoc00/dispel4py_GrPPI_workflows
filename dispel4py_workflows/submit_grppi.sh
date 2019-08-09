set -x

  export PYTHONPATH=$PYTHONPATH:.

  export GRPPI_DIR=/lustre/home/dc010/jfmunoz/test/dispel4py_GrPPI_workflows/grppi_workflows/build/use_cases/tc_cross_correlation/
  export OUTPUT_DIR=/lustre/home/dc010/jfmunoz/test/dispel4py_GrPPI_workflows/dispel4py_workflows/tc_cross_correlation/OUTPUT2/

  export DISPEL4PY_XCORR_STARTTIME=2019-08-07T06:00:00.000
  export DISPEL4PY_XCORR_ENDTIME=2019-08-07T07:00:00.000

  rm -rf ${OUTPUT_DIR}/DATA
  rm -rf ${OUTPUT_DIR}/XCORR
  mkdir ${OUTPUT_DIR}/DATA
  mkdir ${OUTPUT_DIR}/XCORR

  dispel4py multi tc_cross_correlation/realtime_prep.py -f tc_cross_correlation/realtime_xcorr_input.jsn -n 10
  #dispel4py multi tc_cross_correlation/realtime_xcorr_mod.py -n 10
  taskset -c 0-9 ${GRPPI_DIR}/tc_cross_correlation $OUTPUT_DIR $DISPEL4PY_XCORR_STARTTIME tbb

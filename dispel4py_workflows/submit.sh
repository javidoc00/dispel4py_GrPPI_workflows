set -x

  export PYTHONPATH=$PYTHONPATH:.
  export DISPEL4PY_XCORR_STARTTIME=2019-07-21T06:00:00.000
  export DISPEL4PY_XCORR_ENDTIME=2019-07-21T07:00:00.000
  rm -rf ./tc_cross_correlation/OUTPUT/DATA
  rm -rf ./tc_cross_correlation/OUTPUT/XCORR
  mkdir  ./tc_cross_correlation/OUTPUT/DATA
  mkdir ./tc_cross_correlation/OUTPUT/XCORR

  dispel4py multi tc_cross_correlation/realtime_prep.py -f tc_cross_correlation/realtime_xcorr_input.jsn -n 10
  dispel4py multi tc_cross_correlation/realtime_xcorr_mod.py -n 10

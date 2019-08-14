#!/bin/bash --login
#PBS -l select=1:ncpus=36
#PBS -l walltime=00:02:00

#PBS -A ????

# Change to the directory that the job was submitted from
cd $PBS_O_WORKDIR

# Load any required modules
module load mpt
module load intel-mpi-17
module load gcc/6.3.0
module load cmake/3.14.1
module load intel-tbb/16.0.3.210
module load fftw/3.3.5-gcc6
module load anaconda/python3

source activate dispel4py-env

# Set the number of threads to 1
#   This prevents any threaded system libraries from automatically
#   using threading.
export OMP_NUM_THREADS=1

export PYTHONPATH=$PYTHONPATH:/lustre/home/z04/rosaf2/dispel4py_workflows/tc_cross_correlation
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lustre/home/dc010/jfmunoz/test/libpng_install/lib64/

/lustre/home/dc010/jfmunoz/test/dispel4py_GrPPI_workflows/dispel4py_workflows/submit_grppi.sh 2> error.txt > output.txt


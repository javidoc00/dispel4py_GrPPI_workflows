/*
 * Copyright 2018 Universidad Carlos III de Madrid
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FFTW_FFTW_ITF_H
#define FFTW_FFTW_ITF_H

#include <iostream>
#include <complex>
#include <map>
#include <utility>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <fftw3.h>

class fftw_itf {

  public:
  
    template<typename _Scalar, int _Rows, int _Cols>
    static Eigen::Array<std::complex<_Scalar>, _Rows, _Cols>
                fft (Eigen::Array<std::complex<_Scalar>, _Rows, _Cols> & in)
    {
        return fft_engine (in, FFTW_FORWARD, fftw_itf::Singleton().flags_);
    }

    template<typename _Scalar, int _Rows, int _Cols>
    static Eigen::Array<std::complex<_Scalar>, _Rows, _Cols>
                ifft (Eigen::Array<std::complex<_Scalar>, _Rows, _Cols> & in)
    {
        return fft_engine (in, FFTW_BACKWARD, fftw_itf::Singleton().flags_);
    }
    
    static void set_flags(int flags) {
        fftw_itf::Singleton().flags_ = flags;
    }

    static int get_flags() {
        return fftw_itf::Singleton().flags_;
    }

  private:
    std::map<std::pair<int,int>, fftw_plan> plans_;
    std::map<std::pair<int,int>, fftw_complex*> in;
    std::map<std::pair<int,int>, fftw_complex*> out;
    mutable std::mutex mutex_;
    mutable std::atomic<bool> planning_{false};
    mutable std::condition_variable cond_plan_;
    int flags_ = FFTW_ESTIMATE;

    fftw_itf() {}

    static fftw_itf & Singleton() {
        static fftw_itf instance;
        return instance;
    }

    template<typename _Scalar, int _Rows, int _Cols>
    static Eigen::Array<std::complex<_Scalar>, _Rows, _Cols>
                fft_engine (Eigen::Array<std::complex<_Scalar>, _Rows, _Cols> & in,
                int plan_sign, unsigned int flags)
    {
        // if input array is not a vector, error
        if ((in.rows() != 1) && (in.cols() != 1)) {
            throw std::runtime_error("format error: input array is not a vector.");
        }
        
        // ensure to eval input vector
        in.eval();
        
        // create output vector allocating memory
        Eigen::Array<std::complex<_Scalar>, _Rows, _Cols> out(in.rows(), in.cols());

        std::unique_lock<std::mutex> lock(fftw_itf::Singleton().mutex_);

        // execute
        fftw_complex *aux_in, *aux_out;

        int N = in.size();
        aux_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        aux_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

        memcpy (aux_in,in.data(), sizeof(std::complex<double>)*in.size());

        //std::cout <<  "Plan done: (sign = " << plan_sign << ", flags = " << flags << ")" << std::endl;
        fftw_plan actual_plan = fftw_plan_dft_1d(N, aux_in, aux_out, plan_sign, flags);
        fftw_execute(actual_plan);

        memcpy (out.data(),aux_out, sizeof(std::complex<double>)*in.size());

        lock.unlock();

        return out;
    }
};

#endif

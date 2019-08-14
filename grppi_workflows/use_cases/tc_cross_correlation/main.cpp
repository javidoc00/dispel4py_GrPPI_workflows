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
// Standard library
#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <random>
#include <experimental/optional>
#include <experimental/filesystem>
#include <complex>
#include <time.h>

// grppi
#include "grppi/grppi.h"

//Eigen
#include <Eigen/Core>
#include <Eigen/Dense>

//mathgl
#include "mgl2/mgl.h"

// Samples shared utilities
#include "../util/util.h"

// aux
#include "npy.hpp"
#include "fftw_itf.hpp"
#include "eigen_shift.hpp"


namespace ex = std::experimental;
namespace fs = std::experimental::filesystem;

using TStreamC = Eigen::Array<std::complex<double>, Eigen::Dynamic, 1>;
using TStreamR = Eigen::Array<double, Eigen::Dynamic, 1>;


ex::optional<std::tuple<unsigned long, std::shared_ptr<TStreamC>,
                        unsigned long, std::shared_ptr<TStreamC>,
                        unsigned long, fs::path>>
            get_cross_elements (const fs::path &ROOT_DIR,
                                const std::vector<fs::path> &hours_list)
{
  static unsigned long num_done_hours = 0;

  static bool change_time = true;
  static std::vector<std::shared_ptr<TStreamC>> list_init;
  static std::vector<std::pair<unsigned long,unsigned long>> list_cross;
  static std::map<unsigned long,std::string> list_paths;
  static unsigned long count = 0;
  static fs::path starttime;
  
  
  if (change_time) {
      if (num_done_hours == hours_list.size()) {
          //std::cout << "end" << std::endl;
          return {};
      }
      
      list_init.clear();
      list_cross.clear();
      list_paths.clear();
      count = 0;
      starttime = hours_list[num_done_hours];
      // get list of file paths
      //std::cout << "-- get_cross_elements: list of files to process --" << std::endl;
      for (auto &dir : fs::directory_iterator{ROOT_DIR / fs::path("DATA") / starttime}) {
          if (dir.path().filename().string()[0] == '.') continue;
          for (auto &file : fs::directory_iterator{dir.path()}) {
              if (file.path().filename().string()[0] == '.') continue;
              list_paths[list_paths.size()] = file.path().string();
              //std::cout << list_paths.size()-1 << ": " << list_paths[list_paths.size()-1] << std::endl;
          }
      }
      //std::cout << ".------------------------------------------------." << std::endl;

      // allocate mamory for intermediate lists
      unsigned long num_cross = (list_paths.size() * (list_paths.size() - 1)) / 2;
      //std::cout << "num_cross = " << num_cross << std::endl;

      list_init.resize(list_paths.size());
      list_cross.resize(num_cross);
      
      // fill up the cross correlation list
      unsigned long iter = 0;
      for (unsigned long i=0; i<list_paths.size(); i++) {
        for (unsigned long j=0; j<i; j++) {
            list_cross[iter] = std::make_pair(j,i);
            iter++;
        }
      }
      //std::cout << "iter = " << iter << std::endl;
    
      // load first set if there is one
      if (list_paths.size() > 0) {
          //std::cout << "LOAD 0: " << list_paths[0] << std::endl;
          list_init[0] = std::make_shared<TStreamC>(npy::LoadArrayFromNumpy<TStreamC>(list_paths[0]));
          //std::cout << "LOAD 0 (" << list_init[0].size() << ", " << list_init[0](100) << "): " << list_paths[0] << std::endl;
      }
      // set change time actions done
      num_done_hours++;
      change_time = false;
  }


  // if there are remining cross correaltion cases -> return them
  if (count < list_cross.size()) {
      auto ret = list_cross[count];
      // if a new vector array is needed -> read file and store the data
      if (ret.first == 0) {
          //std::cout << "LOAD " << ret.first << ": " << list_paths[ret.first] << std::endl;
          list_init[ret.second] = std::make_shared<TStreamC>(npy::LoadArrayFromNumpy<TStreamC>(list_paths[ret.second]));
          //std::cout << "LOAD " << ret.first << " (" << list_init[ret.first].size() << ", " << list_init[ret.first](100) << "): " << list_paths[ret.first] << std::endl;
      }
      //std::cout << ret.first << ", " << ret.second << ", " << count << " -> get_cross_elements" << std::endl;

      count++;
      if (count == list_cross.size()) {
          // if last item, try another time
          change_time=true;
      }
      //std::cout << "count = " << count-1 << std::endl;

      return {{ret.first, list_init[ret.first],
               ret.second, list_init[ret.second],
               count-1, starttime}};
      
  // no more cases -> return false
  } else {
      std::cout << "ERROR: this should never happen" << std::endl;
      return {};
  }
}


std::tuple<unsigned long, unsigned long, std::shared_ptr<TStreamR>, fs::path>
        compute_cross (std::tuple<unsigned long, std::shared_ptr<TStreamC>,
                                  unsigned long, std::shared_ptr<TStreamC>,
                                  unsigned long, fs::path> data)
{

    //std::cout <<  std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> compute_cross (" << list_init[std::get<0>(data)](100) << ", " << list_init[std::get<1>(data)](100) << ")" << std::endl;

    int shift = 5000;
    
    auto shr_array1 = std::get<1>(data);
    auto shr_array2 = std::get<3>(data);

    auto size1 = shr_array1->size();
    auto size2 = shr_array2->size();
    auto size = (size1 < size2) ? size1 : size2;
    
    //std::cout <<  "size1: " << size1 <<  " size2: " << size2 <<  " size: " << size << std::endl;

//    std::cout <<  "init: " << list_init[std::get<0>(data)].head(size)(1234) << std::endl;
//    std::cout <<  "init.conj: " << list_init[std::get<1>(data)].head(size).conjugate()(1234) << std::endl;
//
//    TStreamC tempA = fftw_itf::fft(list_init[std::get<0>(data)]) ;
//    TStreamC tempB = fftw_itf::fft(list_init[std::get<1>(data)]) ;
//
//    TStreamC temp1 = tempA.head(size) * tempB.head(size).conjugate();
    TStreamC temp1 = shr_array1->head(size) *
                     shr_array2->head(size).conjugate();
    // std::cout <<  "temp1: " << temp1(1234) << std::endl;

    TStreamC temp2 = fftw_itf::ifft(temp1) / size;
    // std::cout <<  "temp2: " << temp2(1234) << std::endl;

    TStreamR temp3 = temp2.real();
    // std::cout <<  "temp3: " << temp3(1234) << std::endl;

    TStreamR temp4 = eigen_shift::vShift(temp3,shift);
    // std::cout <<  "temp4: " << temp4(1234) << std::endl;

    return {std::get<0>(data), std::get<2>(data),
            std::make_shared<TStreamR>(temp4.head(2 * shift + 1)),
            std::get<5>(data)};
}


std::tuple<unsigned long, unsigned long, std::shared_ptr<TStreamR>, fs::path>
     store_cross_elements (std::tuple<unsigned long, unsigned long,
                                      std::shared_ptr<TStreamR>, fs::path> data,
                           const fs::path &ROOT_DIR)

{
    //std::cout  << std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> store_cross_elements"<< std::endl;

    fs::path starttime = std::get<3>(data);
    std::string filename = "Xcorr";
    fs::path directory = ROOT_DIR / fs::path("XCORR") / starttime /
                         fs::path(std::to_string(std::get<0>(data)) + "_" +
                                  std::to_string(std::get<1>(data))) ;
    if (!fs::exists(directory)) {
        fs::create_directories(directory);
    }
    directory /= fs::path(filename + "_" + std::to_string(std::get<0>(data))
                                   + "_" + std::to_string(std::get<1>(data)) + ".out");
    
    //std::cout << directory << std::endl;
    auto shr_cross_array = std::get<2>(data);
    npy::SaveArrayAsNumpy(directory.string(), (*shr_cross_array) );

    return data;
}

void plot_cross_elements (std::tuple<unsigned long, unsigned long,
                                     std::shared_ptr<TStreamR>, fs::path> data,
                          const fs::path &ROOT_DIR)

{
    //std::cout  << std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> plot_cross_elements"<< std::endl;

    // get filename "
    fs::path starttime = std::get<3>(data);
    fs::path filename = ROOT_DIR / fs::path("XCORR") / starttime /
                        fs::path("Xcorr_" + std::to_string(std::get<0>(data)) + "_" +
                                 std::to_string(std::get<1>(data)) + ".plot.eps");
    
    //std::cout << filename << std::endl;
    // set data for graph
    mglData data_y;
    mglData data_x;

    // calculate linspace
    int sps=4;
    auto shr_cross_array = std::get<2>(data);
    int size = shr_cross_array->size();
    TStreamR linspace = TStreamR::LinSpaced(size,-(size+1)/(2*sps), size/(2*sps));
    linspace.eval();

    data_y.Link(shr_cross_array->data(), shr_cross_array->size());
    //data_y.Link(linspace.data(),linspace.size());
    data_x.Link(linspace.data(),linspace.size());

    // set grph attibutes
    mglGraph gr;
    gr.SetTickTempl('x',"%.0f"); gr.SetTickTempl('y',"%.0f");
    gr.SetTuneTicks(0);
    gr.SetFontSizePT(6.0);
    gr.Box("",false);
    gr.SetRanges(-1000, 1000, (data_y.Minimal() * 1.1), (data_y.Maximal() * 1.1));
    //std::cout << "(" << data_y[0] << ", " << data_y[1] << ", " << size << ")" << std::endl;

    // draw and write graph
    gr.Axis();
    gr.Plot(data_x,data_y);
    gr.WriteEPS(filename.c_str());

}

void tc_cross_correlation (grppi::dynamic_execution & exec,
                           const fs::path &root_dir,
                           const std::vector<fs::path> &hours_list) {
    
  grppi::pipeline(exec,
    [&root_dir, &hours_list] () ->
            ex::optional<std::tuple<unsigned long, std::shared_ptr<TStreamC>,
                                    unsigned long, std::shared_ptr<TStreamC>,
                                    unsigned long, fs::path>> {
      return get_cross_elements(root_dir, hours_list);
    },
    grppi::farm(4,
        [] (auto result) {
            return compute_cross (result);
        }
    ),
    grppi::farm(4,
        [&root_dir] (auto result) {
            return store_cross_elements (result, root_dir);
        }
    ),
    grppi::farm(4,
        [&root_dir] (auto result) {
            return plot_cross_elements (result, root_dir);
        }
    )  
  );
}

void print_message(const std::string & prog,
                   const std::string & msg) {
    
  std::cerr << msg << std::endl;
  std::cerr << "Usage: " << prog << " root_dir start_time end_time mode" << std::endl;
  std::cerr << "  root_dir: Root directory for input and output data files" << std::endl;
  std::cerr << "  start_time: begin time mark for the selected files" << std::endl;
  std::cerr << "  end_time: end time mark for the selected files" << std::endl;
  std::cerr << "  mode:" << std::endl;
  print_available_modes(std::cerr);
}


int main(int argc, char **argv) {
    
  using namespace std;

  if(argc < 5){
    print_message(argv[0], "Invalid number of arguments.");
    return -1;
  }

  fs::path root_dir (argv[1]);
  if (!fs::exists(root_dir)) {
    print_message(argv[0], "Invalid root directory.");
    return -1;
  }

  std::string start_time (argv[2]);
  std::string end_time (argv[3]);
 
  struct tm tm_start, tm_end;
  time_t t_start, t_end;
  unsigned long lapse;
 
  //if (strptime(argv[2], "%Y-%m-%dT%H/%M/%S.000", &tm_start) == NULL) {
  if (strptime(argv[2], "%Y-%m-%dT%H:%M:%S.000", &tm_start) == NULL) {
      std::cout << argv[2] << std::endl;
    print_message(argv[0], "Invalid start time.");
    return -1;
  }

  if (strptime(argv[3], "%Y-%m-%dT%H:%M:%S.000", &tm_end) == NULL) {
    print_message(argv[0], "Invalid end time.");
    return -1;
  }
  t_start = mktime(&tm_start);
  t_end = mktime(&tm_end);
  lapse = (t_end - t_start) / 3600;
  std::vector<fs::path> hours_list(lapse);
  for (unsigned long i=0; i<lapse; i++) {
    char aux_str[256];
    std::time_t t_now = t_start + (i*3600);
    auto tm_now = std::gmtime(&t_now);
    strftime(aux_str, sizeof(aux_str), "%Y-%m-%dT%H:%M:%S.000000Z", tm_now);
    hours_list[i]=fs::path(aux_str);
    std::cout << hours_list[i] << std::endl;
  }
  if (!run_test(argv[4], tc_cross_correlation, root_dir, hours_list)) {
    print_message(argv[0], "Invalid policy.");
    return -1;
  }

  return 0;
}

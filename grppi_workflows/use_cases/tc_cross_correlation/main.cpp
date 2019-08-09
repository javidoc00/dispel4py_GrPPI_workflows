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


ex::optional<std::tuple<unsigned long,unsigned long, unsigned long>>
            get_cross_elements (std::vector<TStreamC> &list_init,
                                std::vector<TStreamR> &list_end,
                                const fs::path &ROOT_DIR,
                                const fs::path & starttime)
{
  static bool first_time = true;
  static std::map<unsigned long,std::string> list_paths;
  static std::vector<std::pair<unsigned long,unsigned long>> list_cross;
  static unsigned long count = 0;

  if (first_time) {
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
      list_end.resize(num_cross);
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
          list_init[0] = npy::LoadArrayFromNumpy<TStreamC>(list_paths[0]);
          //std::cout << "LOAD 0 (" << list_init[0].size() << ", " << list_init[0](100) << "): " << list_paths[0] << std::endl;
      }
      // set first time actions done
      first_time = false;
  }


  // if there are remining cross correaltion cases -> return them
  if (count < list_cross.size()) {
      auto ret = list_cross[count];
      // if a new vector array is needed -> read file and store the data
      if (ret.first == 0) {
          //std::cout << "LOAD " << ret.first << ": " << list_paths[ret.first] << std::endl;
          list_init[ret.second] = npy::LoadArrayFromNumpy<TStreamC>(list_paths[ret.second]);
          //std::cout << "LOAD " << ret.first << " (" << list_init[ret.first].size() << ", " << list_init[ret.first](100) << "): " << list_paths[ret.first] << std::endl;
      }
      //std::cout << ret.first << ", " << ret.second << ", " << count << " -> get_cross_elements" << std::endl;
      count++;
      //std::cout << "count = " << count-1 << std::endl;

      return {{ret.first,ret.second,count-1}};
      
  // no more cases -> return false
  } else {
      //std::cout << "end" << std::endl;
      return {};
  }
}


std::tuple<unsigned long,unsigned long, unsigned long>
        compute_cross (std::tuple<unsigned long,unsigned long,unsigned long> data,
                       std::vector<TStreamC> &list_init,
                       std::vector<TStreamR> &list_end)
{

    //std::cout <<  std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> compute_cross (" << list_init[std::get<0>(data)](100) << ", " << list_init[std::get<1>(data)](100) << ")" << std::endl;

    int shift = 5000;
    
    auto size1 = list_init[std::get<0>(data)].size();
    auto size2 = list_init[std::get<1>(data)].size();
    auto size = (size1 < size2) ? size1 : size2;
    
//    std::cout <<  "init: " << list_init[std::get<0>(data)].head(size)(1234) << std::endl;
//    std::cout <<  "init.conj: " << list_init[std::get<1>(data)].head(size).conjugate()(1234) << std::endl;
//
//    TStreamC tempA = fftw_itf::fft(list_init[std::get<0>(data)]) ;
//    TStreamC tempB = fftw_itf::fft(list_init[std::get<1>(data)]) ;
//
//    TStreamC temp1 = tempA.head(size) * tempB.head(size).conjugate();
    TStreamC temp1 = list_init[std::get<0>(data)].head(size) *
                     list_init[std::get<1>(data)].head(size).conjugate();
    //std::cout <<  "temp1: " << temp1(1234) << std::endl;

    TStreamC temp2 = fftw_itf::ifft(temp1) / size;
    //std::cout <<  "temp2: " << temp2(1234) << std::endl;

    TStreamR temp3 = temp2.real();
    //std::cout <<  "temp3: " << temp3(1234) << std::endl;

    TStreamR temp4 = eigen_shift::vShift(temp3,shift);
    //std::cout <<  "temp4: " << temp4(1234) << std::endl;

    list_end[std::get<2>(data)] = temp4.head(2 * shift + 1);
    return data;
}


std::tuple<unsigned long,unsigned long, unsigned long>
     store_cross_elements (std::tuple<unsigned long,unsigned long,unsigned long> data,
                           std::vector<TStreamR> &list_end,
                           const fs::path &ROOT_DIR,
                           const fs::path & starttime)

{
    //std::cout  << std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> store_cross_elements"<< std::endl;

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
    npy::SaveArrayAsNumpy(directory.string(),list_end[std::get<2>(data)]);

    return data;
}

void plot_cross_elements (std::tuple<unsigned long,unsigned long,unsigned long> data,
                          std::vector<TStreamR> &list_end,
                          const fs::path &ROOT_DIR,
                          const fs::path & starttime)

{
    //std::cout  << std::get<0>(data) << ", " << std::get<1>(data) << ", " << std::get<2>(data) << " -> plot_cross_elements"<< std::endl;

    // get filename "
    fs::path filename = ROOT_DIR / fs::path("XCORR") / starttime /
                        fs::path("Xcorr_" + std::to_string(std::get<0>(data)) + "_" +
                                 std::to_string(std::get<1>(data)) + ".plot.eps");
    
    //std::cout << filename << std::endl;
    // set data for graph
    mglData data_y;
    mglData data_x;

    // calculate linspace
    int sps=4;
    int size = list_end[std::get<2>(data)].size();
    TStreamR linspace = TStreamR::LinSpaced(size,-(size+1)/(2*sps), size/(2*sps));
    linspace.eval();

    data_y.Link(list_end[std::get<2>(data)].data(),list_end[std::get<2>(data)].size());
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
                           const fs::path &start_time) {
    
  std::vector<TStreamC> list;
  std::vector<TStreamR> list2;

  grppi::pipeline(exec,
    [&root_dir, &start_time, &list, &list2] () ->
            ex::optional<std::tuple<unsigned long,unsigned long, unsigned long>> {
      return get_cross_elements(list, list2, root_dir, start_time);
    },
    grppi::farm(4,
        [&list, &list2] (auto result) {
            return compute_cross (result, list, list2);
        }
    ),
    grppi::farm(4,
        [&root_dir, &start_time, &list2] (auto result) {
            return store_cross_elements (result, list2, root_dir, start_time);
        }
    ),
    grppi::farm(4,
        [&root_dir, &start_time, &list2] (auto result) {
            return plot_cross_elements (result, list2, root_dir, start_time);
        }
    )  
  );
}

void print_message(const std::string & prog,
                   const std::string & msg) {
    
  std::cerr << msg << std::endl;
  std::cerr << "Usage: " << prog << " root_dir start_time mode" << std::endl;
  std::cerr << "  root_dir: Root directory for input and output data files" << std::endl;
  std::cerr << "  start_time: time mark for the selected files" << std::endl;
  std::cerr << "  mode:" << std::endl;
  print_available_modes(std::cerr);
}


int main(int argc, char **argv) {
    
  using namespace std;

  if(argc < 4){
    print_message(argv[0], "Invalid number of arguments.");
    return -1;
  }

 fs::path root_dir (argv[1]);
  if (!fs::exists(root_dir)) {
    print_message(argv[0], "Invalid root directory.");
    return -1;
  }

 fs::path start_time (argv[2]);

  if (!run_test(argv[3], tc_cross_correlation, root_dir, start_time)) {
    print_message(argv[0], "Invalid policy.");
    return -1;
  }

  return 0;
}

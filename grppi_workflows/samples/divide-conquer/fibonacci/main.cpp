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
#include <vector>
#include <fstream>
#include <chrono>
#include <string>
#include <numeric>
#include <stdexcept>
#include <random>

// grppi
#include "grppi/grppi.h"

// Samples shared utilities
#include "../../util/util.h"

void fibonacci(grppi::dynamic_execution & exec, int n) {
  using namespace std;

  auto res = grppi::divide_conquer(exec,
    n,
    [](int x) -> vector<int> {
      return { x-1, x-2 };
    },
    [](int x) { return x<2; },
    [](int) { return 1; },
    [](int s1, int s2) {
      return s1+s2;
    }
  );

  cout << "Fibonacci(" << n << ")= " << res << endl;
}

void print_message(const std::string & prog, const std::string & msg) {
  using namespace std;

  cerr << msg << endl;
  cerr << "Usage: " << prog << " n mode" << endl;
  cerr << "  n: Order number in fibonacci series" << endl;
  cerr << "  mode:" << endl;
  print_available_modes(cerr);
}


int main(int argc, char **argv) {
    
  using namespace std;

  if(argc < 3){
    print_message(argv[0], "Invalid number of arguments.");
    return -1;
  }

  int n = stoi(argv[1]);
  if(n <= 0){
    print_message(argv[0], "Invalid problem size. Use a positive number.");
    return -1;
  }

  if (!run_test(argv[2], fibonacci, n)) {
    print_message(argv[0], "Invalid policy.");
    return -1;
  }

  return 0;
}

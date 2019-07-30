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
#include <experimental/optional>

// grppi
#include "grppi/grppi.h"

// Samples shared utilities
#include "../../util/util.h"

void print_power(grppi::dynamic_execution & e, int n) {
  using namespace std;

  auto generator = [i=1,max=n+1]() mutable -> grppi::optional<int> {
    if (i<max) return i++;
    else return {};
  };

  grppi::pipeline(e,
    generator,
    grppi::repeat_until(
      [](int x) { return 2*x; },
      [](int x) { return x>1024; }
    ),
    [](int x) { cout << x << endl; }
  );
}

void print_message(const std::string & prog, const std::string & msg) {
  using namespace std;

  cerr << msg << endl;
  cerr << "Usage: " << prog << " size window_size offset mode" << endl;
  cerr << "  size: Size of the initially generated sequence" << endl;
  cerr << "  mode:" << endl;
  print_available_modes(cerr);
}


int main(int argc, char **argv) {
    
  using namespace std;

  if (argc < 3) {
    print_message(argv[0], "Invalid number of arguments.");
    return -1;
  }

  int size = stoi(argv[1]);
  if (size<=0) {
    print_message(argv[0], "Invalid sequence size. Use a positive number.");
    return -1;
  }

  if (!run_test(argv[2], print_power, size)) {
    print_message(argv[0], "Invalid policy.");
    return -1;
  }

  return 0;
}

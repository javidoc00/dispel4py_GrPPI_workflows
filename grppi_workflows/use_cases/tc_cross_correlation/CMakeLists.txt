# Copyright 2018 Universidad Carlos III de Madrid
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


find_package (Eigen3 3.3 NO_MODULE)
find_package (FFTW)
find_package (MathGL2)


if (EIGEN3_FOUND AND FFTW_FOUND AND MATHGL2_FOUND)

  include_directories(${EIGEN3_INCLUDE_DIR} ${FFTW_INCLUDE_DIRS} ${MATHGL2_INCLUDE_DIRS})

  add_executable(tc_cross_correlation main.cpp)

  target_link_libraries(tc_cross_correlation 
    ${CMAKE_THREAD_LIBS_INIT} 
    ${TBB_LIBRARIES} 
    ${FFTW_LIBRARIES}
    ${MATHGL2_LIBRARIES}
    -lstdc++fs)

  add_executable(pru test.cpp)

  target_link_libraries(pru 
    ${FFTW_LIBRARIES}
    ${MATHGL2_LIBRARIES}
    -lstdc++fs)

endif (EIGEN3_FOUND AND FFTW_FOUND AND MATHGL2_FOUND)
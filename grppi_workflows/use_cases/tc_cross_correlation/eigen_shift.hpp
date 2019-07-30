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
#ifndef EIGEN_SHIFT_H
#define EIGEN_SHIFT_H

namespace eigen_shift {
#include <Eigen/Dense>

template<typename _Scalar, int _Rows, int _Cols>
inline Eigen::Array<_Scalar, _Rows, _Cols>
            vShift(Eigen::Array<_Scalar, _Rows, _Cols> & data, int down)
{
  using TStream = Eigen::Array<_Scalar, _Rows, _Cols>;

  if ((data.rows() != 1) && (data.cols() != 1)) {
       throw std::runtime_error("Array not a vector");
  }
  
  if (down <= 0) {
      return data;
  } else {
      TStream ret(data.rows(),data.cols());
      auto rest = data.size() - down;
      ret.topRows(down) = data.bottomRows(down);
      ret.bottomRows(rest) = data.topRows(rest);
      return ret;
  }
}

}

#endif

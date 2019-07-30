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
#ifndef GRPPI_COMMON_OPTIONAL_H
#define GRPPI_COMMON_OPTIONAL_H

#if __cplusplus < 201703L
#include <experimental/optional>
#else
#include <optional>
#endif

namespace grppi {

/**
 * \brief An optional type usable both in C++14 and C++17.
 */
template <typename T>
using optional =
#if __cplusplus < 201703L
  std::experimental::optional<T>;
#else
  std::optional<T>;
#endif

}


#endif //GRPPI_COMMON_OPTIONAL_H

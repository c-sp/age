//
// Copyright 2020 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <algorithm> // std::for_each

#include <age_utilities.hpp>



age::uint32_t age::crc32(const uint8_vector& data)
{
    return crc32(begin(data), end(data));
}

age::uint32_t age::crc32(uint8_vector::const_iterator begin, uint8_vector::const_iterator end)
{
    age::uint32_t crc = 0xFFFFFFFF;

    std::for_each(begin,
                  end,
                  [&](const uint8_t& v) {
                      crc ^= v;
                      for (int i = 0; i < 8; ++i)
                      {
                          crc = (crc & 1U)
                                    ? (crc >> 1U) ^ 0xEDB88320
                                    : crc >> 1U;
                      }
                  });

    return ~crc;
}

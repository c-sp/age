//
// Copyright 2021 Christoph Sprenger
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

#ifndef AGE_TESTER_WRITE_LOG_HPP
#define AGE_TESTER_WRITE_LOG_HPP

#include <emulator/age_gb_types.hpp>

#include <filesystem>
#include <vector>



namespace age::tester
{
    void write_log(const std::filesystem::path& log_path, const std::vector<gb_log_entry>& log_entries);

} // namespace age::tester



#endif // AGE_TESTER_WRITE_LOG_HPP

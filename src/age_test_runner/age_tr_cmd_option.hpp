//
// © 2023 Christoph Sprenger <https://github.com/c-sp>
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
#ifndef AGE_TR_CMD_OPTION_HPP
#define AGE_TR_CMD_OPTION_HPP

#include <cassert>
#include <string>
#include <utility>



namespace age::tr
{
    class age_tr_cmd_option
    {
    public:
        age_tr_cmd_option(char        opt_short_name,
                          std::string opt_long_name,
                          std::string opt_description)
            : age_tr_cmd_option(opt_short_name,
                                std::move(opt_long_name),
                                false,
                                std::move(opt_description))
        {}

        age_tr_cmd_option(char        opt_short_name,
                          std::string opt_long_name,
                          bool        argument_required,
                          std::string opt_description)
            : m_opt_short_name(opt_short_name),
              m_opt_long_name(std::move(opt_long_name)),
              m_argument_required(argument_required),
              m_opt_description(std::move(opt_description))
        {
            assert(m_opt_short_name.length() == 1);
            assert(m_opt_long_name.length() > 1);
            assert(!m_opt_description.empty());
        }

        [[nodiscard]] const char         opt_short_name() const { return m_opt_short_name; }
        [[nodiscard]] const std::string& opt_long_name() const { return m_opt_long_name; }
        [[nodiscard]] bool               argument_required() const { return m_argument_required; }
        [[nodiscard]] const std::string& opt_description() const { return m_opt_description; }

        bool operator<(const age_tr_cmd_option& rhs) const
        {
            return m_opt_short_name < rhs.m_opt_short_name;
        }

    private:
        char        m_opt_short_name;
        std::string m_opt_long_name;
        bool        m_argument_required;
        std::string m_opt_description;
    };

} // namespace age::tr



#endif // AGE_TR_CMD_OPTION_HPP

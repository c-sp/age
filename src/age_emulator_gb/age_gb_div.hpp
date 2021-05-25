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

#ifndef AGE_GB_DIV_HPP
#define AGE_GB_DIV_HPP

//!
//! \file
//!

#include <age_types.hpp>

#include "common/age_gb_clock.hpp"



namespace age
{

    struct gb_div_reset_details
    {
        //! the next counter increment aligned with the previous DIV
        //! (relative to the current clock cycle)
        int m_old_next_increment = 0;

        //! the next counter increment aligned with the current DIV reset
        //! (relative to the current clock cycle)
        int m_new_next_increment = 0;

        //! positive value:
        //! the next counter increment is delayed by the DIV reset
        //! (m_new_next_increment - m_old_next_increment).
        //!
        //! negative value:
        //! the DIV reset causes an immediate counter increment
        //! (-m_old_next_increment).
        int m_clk_adjust = 0;
    };



    class gb_div
    {
        AGE_DISABLE_COPY(gb_div);

    public:
        explicit gb_div(const gb_clock& clock);

        [[nodiscard]] gb_div_reset_details calculate_reset_details(int lowest_counter_bit) const;
        [[nodiscard]] int                  get_div_offset() const;

        [[nodiscard]] uint8_t read_div() const;
        void                  write_div();

    private:
        const gb_clock& m_clock;
        int             m_old_div_offset = 0; //!< clock offset before the last reset
        int             m_div_offset     = 0; //!< used to re-align DIV to clock on DIV-writes
    };

} // namespace age



#endif // AGE_GB_DIV_HPP

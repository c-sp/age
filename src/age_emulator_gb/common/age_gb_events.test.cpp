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

#include <gtest/gtest.h>

#include "age_gb_events.hpp"

#include <limits>

namespace
{
    constexpr int max_clock_cycle = std::numeric_limits<int>::max();

} // namespace



TEST(AgeGbSortedEvents, PollsNoneIfEmpty)
{
    age::gb_sorted_events events;
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::none);
}

TEST(AgeGbSortedEvents, PollsChronologically)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 30);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_vblank, 20);

    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_mode2);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_vblank);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_mode0);
}

TEST(AgeGbSortedEvents, PollsBasedOnCycle)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 20);

    EXPECT_EQ(events.poll_next_event(9), age::gb_event::none);
    EXPECT_EQ(events.poll_next_event(10), age::gb_event::lcd_interrupt_mode0);

    EXPECT_EQ(events.poll_next_event(19), age::gb_event::none);
    EXPECT_EQ(events.poll_next_event(21), age::gb_event::lcd_interrupt_mode2);
}



TEST(AgeGbSortedEvents, KeepsTrackOfScheduledEventCycle)
{
    age::gb_sorted_events events;

    EXPECT_EQ(events.get_event_cycle(age::gb_event::lcd_interrupt_vblank), age::gb_no_clock_cycle);
    events.schedule_event(age::gb_event::lcd_interrupt_vblank, 123);
    EXPECT_EQ(events.get_event_cycle(age::gb_event::lcd_interrupt_vblank), 123);
}

TEST(AgeGbSortedEvents, ReplacesScheduledEventEarlier)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 20);
    events.schedule_event(age::gb_event::lcd_interrupt_vblank, 30);
    events.schedule_event(age::gb_event::lcd_interrupt_lyc, 40);

    EXPECT_EQ(events.poll_next_event(5), age::gb_event::none);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 5);
    EXPECT_EQ(events.poll_next_event(5), age::gb_event::lcd_interrupt_mode2);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_mode0);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_vblank);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_lyc);
}

TEST(AgeGbSortedEvents, ReplacesScheduledEventLater)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 20);
    events.schedule_event(age::gb_event::lcd_interrupt_vblank, 30);
    events.schedule_event(age::gb_event::lcd_interrupt_lyc, 40);

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 50);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_mode2);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_vblank);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_lyc);
    EXPECT_EQ(events.poll_next_event(max_clock_cycle), age::gb_event::lcd_interrupt_mode0);
}



TEST(AgeGbSortedEvents, RemovesScheduledEvent)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 20);
    events.schedule_event(age::gb_event::lcd_interrupt_vblank, 30);

    events.remove_event(age::gb_event::lcd_interrupt_mode0);
    EXPECT_EQ(events.poll_next_event(15), age::gb_event::none);
}

TEST(AgeGbSortedEvents, IgnoredNotScheduledEventOnRemove)
{
    age::gb_sorted_events events;

    events.schedule_event(age::gb_event::lcd_interrupt_mode0, 10);
    events.schedule_event(age::gb_event::lcd_interrupt_mode2, 20);

    events.remove_event(age::gb_event::lcd_interrupt_vblank);
    EXPECT_EQ(events.poll_next_event(15), age::gb_event::lcd_interrupt_mode0);
}

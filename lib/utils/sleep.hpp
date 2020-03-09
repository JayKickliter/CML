#pragma once

/*
    Name: sleep.hpp

    Copyright(c) 2019 Mateusz Semegen
    This code is licensed under MIT license (see LICENSE file for details)
*/

//cml
#include <common/bit.hpp>
#include <common/cc_attributes.hpp>
#include <common/frequency.hpp>
#include <common/macros.hpp>
#include <common/time_tick.hpp>
#include <debug/assert.hpp>
#include <hal/mcu.hpp>
#include <hal/systick.hpp>

namespace cml {
namespace utils {

struct sleep
{
    static void ms(common::time_tick a_time)
    {
        assert(true == hal::systick::is_enabled());

        common::time_tick start = hal::systick::get_counter();
        while (common::time_tick_diff(hal::systick::get_counter(), start) <= a_time);
    }

    inline static void us(common::time_tick a_time)
    {
        assert(hal::mcu::get_sysclk_frequency_hz() >= common::MHz(1));
        assert(a_time > 0);

#ifdef CML_DWT_PRESENT
        assert(true == hal::mcu::is_dwt_enabled());

        DWT->CYCCNT = 0;
        const common::uint32 max = DWT->CYCCNT + (hal::mcu::get_sysclk_frequency_hz() / common::MHz(1) * (a_time - 1));

        while (DWT->CYCCNT < max);
#endif // CMLDWT_PRESENT

#ifndef CML_DWT_PRESENT
        common::uint32 count = ((((hal::mcu::get_sysclk_frequency_hz() / common::MHz(1))) / 4) * (a_time - 1));

        __asm__ __volatile__("1: sub %0, #1 \n"
                             "   cmp %0, #0 \n"
                             "   bne  1b    \n"
                             : "+r" (count));

#endif // !CML_DWT_PRESENT
    }

    template<typename Register_t>
    static void until(const Register_t* a_p_register, common::uint32 a_flag, bool a_status)
    {
        while (a_status == common::is_flag(*a_p_register, a_flag));
    }

    template<typename Register_t>
    static bool until(const Register_t* a_p_register,
                      common::uint32 a_flag,
                      bool a_status,
                      common::time_tick a_start,
                      common::time_tick a_timeout_ms)
    {
        bool status  = true;
        bool timeout = false;

        while (true == status && false == timeout)
        {
            timeout = a_timeout_ms <= common::time_tick_diff(hal::systick::get_counter(), a_start);
            status  = common::is_flag(*a_p_register, a_flag) == a_status;
        }

        return ((false == status) && (false == timeout));
    }

    sleep()             = delete;
    sleep(sleep&&)      = delete;
    sleep(const sleep&) = delete;
    ~sleep()            = delete;

    sleep& operator = (sleep&&)      = delete;
    sleep& operator = (const sleep&) = delete;
};

} // namespace utils
} // namespace cml
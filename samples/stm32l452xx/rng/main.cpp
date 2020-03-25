/*
    Name: main.cpp

    Copyright(c) 2020 Mateusz Semegen
    This code is licensed under MIT license (see LICENSE file for details)
*/

//cml
#include <hal/GPIO.hpp>
#include <hal/mcu.hpp>
#include <hal/rng.hpp>
#include <hal/systick.hpp>
#include <hal/USART.hpp>
#include <utils/Console.hpp>
#include <utils/sleep.hpp>

int main()
{
    using namespace cml::common;
    using namespace cml::hal;
    using namespace cml::utils;

    mcu::enable_hsi_clock(mcu::Hsi_frequency::_16_MHz);
    mcu::set_sysclk(mcu::Sysclk_source::hsi, { mcu::Bus_prescalers::AHB::_1,
                                               mcu::Bus_prescalers::APB1::_1,
                                               mcu::Bus_prescalers::APB2::_1 });

    if (mcu::Sysclk_source::hsi == mcu::get_sysclk_source())
    {
        mcu::set_nvic({ mcu::NVIC_config::Grouping::_4, 16u << 4u });

        USART::Config usart_config =
        {
            115200u,
            USART::Oversampling::_16,
            USART::Word_length::_8_bits,
            USART::Stop_bits::_1,
            USART::Flow_control::none,
            USART::Parity::none,
        };

        USART::Clock usart_clock
        {
            USART::Clock::Source::sysclk,
            mcu::get_sysclk_frequency_hz(),
        };

        Alternate_function_pin::Config usart_pin_config =
        {
            Alternate_function_pin::Mode::push_pull,
            Alternate_function_pin::Pull::up,
            Alternate_function_pin::Speed::low,
            0x7u
        };

        mcu::disable_msi_clock();
        systick::enable(0x0);

        GPIO gpio_port_a(GPIO::Id::a);
        gpio_port_a.enable();

        Alternate_function_pin console_usart_TX_pin(&gpio_port_a, 2);
        Alternate_function_pin console_usart_RX_pin(&gpio_port_a, 3);

        console_usart_TX_pin.enable(usart_pin_config);
        console_usart_RX_pin.enable(usart_pin_config);

        USART console_usart(USART::Id::_2);
        bool usart_ready = console_usart.enable(usart_config, usart_clock, 0x1u, 10);

        if (true == usart_ready)
        {
            Console console(&console_usart);
            console.enable();

            console.write_line("CML rng sample. CPU speed: %u MHz", mcu::get_sysclk_frequency_hz() / MHz(1));

            mcu::enable_hsi48_clock(mcu::Hsi48_frequency::_48_MHz);
            mcu::set_clk48_clock_mux_source(mcu::Clk48_mux_source::hsi48);

            bool rng_ready = rng::enable(0x1u, 30);

            if (true == rng_ready)
            {
                while (true)
                {
                    uint32 v = 0;
                    bool ok = rng::get_value_polling(&v, 30);

                    if (true == ok)
                    {
                        console.write_line("Random number: %u", v);
                    }
                    else
                    {
                        console.write_line("Random number generation error");
                    }

                    sleep::ms(1000);
                }
            }
            else
            {
                console.write_line("Cannot enable RNG");
            }

        }
    }

    while (true);
}
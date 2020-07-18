#pragma once

/*
    Name: Command_line.hpp

    Copyright(c) 2019 Mateusz Semegen
    This code is licensed under MIT license (see LICENSE file for details)
*/

//std
#include <cstdint>

//cml
#include <cml/collection/Ring.hpp>
#include <cml/collection/Vector.hpp>
#include <cml/common/cstring.hpp>
#include <cml/debug/assert.hpp>
#include <cml/hal/peripherals/USART.hpp>
#include <cml/utils/config.hpp>

namespace cml {
namespace utils {

class Command_line
{
public:

    struct Callback
    {
        struct Parameter
        {
            const char* a_p_value = nullptr;
            uint32_t length       = 0;
        };

        using Function = void(*)(const collection::Vector<Parameter>&, void*);

        const char* p_name = nullptr;

        Function function  = nullptr;
        void* p_user_data  = nullptr;
    };

public:

    Command_line(hal::peripherals::USART* a_p_io_stream,
                 const char* a_p_prompt,
                 const char* a_p_command_not_found_message)
        : p_io_stream(a_p_io_stream)
        , p_prompt(a_p_prompt)
        , p_command_not_found_message(a_p_command_not_found_message)
        , prompt_length(common::cstring::length(a_p_prompt, config::command_line::line_buffer_capacity))
        , command_not_found_message_length(common::cstring::length(a_p_command_not_found_message,
                                                                   config::command_line::line_buffer_capacity))
        , line_length(0)
        , escape_sequence_buffer{ 0, 0, 0 }
        , escape_sequence_buffer_view(this->escape_sequence_buffer, sizeof(this->escape_sequence_buffer))
        , input_buffer_view(this->input_buffer, config::command_line::input_buffer_capacity)
        , callback_parameters_buffer_view(this->callback_parameters_buffer,
                                          config::command_line::callback_parameters_buffer_capacity)
        , callbacks_buffer_view(this->callbacks_buffer, config::command_line::callbacks_buffer_capacity)
    {
        assert(nullptr != this->p_prompt);
        assert(nullptr != this->p_io_stream);
    }

    Command_line()                    = delete;
    Command_line(Command_line&&)      = default;
    Command_line(const Command_line&) = default;
    ~Command_line()                   = default;

    Command_line& operator = (Command_line&&)      = default;
    Command_line& operator = (const Command_line&) = default;

    void update();

    bool register_callback(const Callback& a_callback)
    {
        return this->callbacks_buffer_view.push_back(a_callback);
    }

    void enable();
    void disable();

    void write_prompt()
    {
        this->p_io_stream->transmit_bytes_polling(this->p_prompt, this->prompt_length);
    }

private:

    class Commands_carousel
    {
    public:

        struct Command
        {
            char buffer[config::command_line::line_buffer_capacity] = { 0 };
            uint32_t length = 0;
        };

    public:

        Commands_carousel()
            : length(0)
            , read_index(0)
            , write_index(0)
        {}

        Commands_carousel(Commands_carousel&&)      = default;
        Commands_carousel(const Commands_carousel&) = default;
        ~Commands_carousel()                        = default;

        Commands_carousel& operator = (Commands_carousel&&)      = default;
        Commands_carousel& operator = (const Commands_carousel&) = default;

        void push(const char* a_p_line, uint32_t a_length);

        const Command& read_next() const;
        const Command& read_prev() const;

        uint32_t get_length() const
        {
            return this->length;
        }

    private:

        Command commands[config::command_line::commands_carousel_capacity];

        uint32_t length;
        mutable uint32_t read_index;
        mutable uint32_t write_index;
    };

private:

    collection::Vector<Callback::Parameter> get_callback_parameters(char* a_p_line,
                                                                    uint32_t a_length,
                                                                    const char* a_p_separators,
                                                                    uint32_t a_separators_count);

    bool execute_command(const collection::Vector<Callback::Parameter>& a_parameters);
    void execute_escape_sequence(char a_first, char a_second);

    void write_new_line()
    {
        this->p_io_stream->transmit_bytes_polling("\n", 1);
    }

private:

    hal::peripherals::USART* p_io_stream;

    const char* p_prompt;
    const char* p_command_not_found_message;

    uint32_t prompt_length;
    uint32_t command_not_found_message_length;
    uint32_t line_length;

    char escape_sequence_buffer[3];
    char input_buffer[config::command_line::input_buffer_capacity];

    Callback::Parameter callback_parameters_buffer[config::command_line::callback_parameters_buffer_capacity];
    Callback callbacks_buffer[config::command_line::callbacks_buffer_capacity];

    char line_buffer[config::command_line::line_buffer_capacity];

    collection::Vector<char> escape_sequence_buffer_view;
    collection::Ring<char> input_buffer_view;
    collection::Vector<Callback::Parameter> callback_parameters_buffer_view;
    collection::Vector<Callback> callbacks_buffer_view;

    Commands_carousel commands_carousel;
};

}// namespace utils
}// namespace cml
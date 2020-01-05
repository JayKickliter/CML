#pragma once

/*
    Name: cc_attributes.hpp

    Copyright(c) 2019 Mateusz Semegen
    This code is licensed under MIT license (see LICENSE file for details)
*/

#ifdef __GNUG__
#define _force_align(v) __attribute__(aligned(v))
#define _force_pack __attribute__((__packed__))
#define _force_inline __attribute__(always_inline)
#endif
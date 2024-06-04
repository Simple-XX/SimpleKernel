/*
   RISC-V machine interrupts.
   SPDX-License-Identifier: Unlicense

   https://five-embeddev.com/

*/

#ifndef RISCV_INTERRUPTS_HPP
#define RISCV_INTERRUPTS_HPP

#include <cstdint>

namespace riscv {
    struct interrupts {
        static constexpr std::uint32_t msi = 3;
        static constexpr std::uint32_t mti = 7;
        static constexpr std::uint32_t mei = 11;
        static constexpr std::uint32_t ssi = 1;
        static constexpr std::uint32_t sti = 5;
        static constexpr std::uint32_t sei = 9;
        static constexpr std::uint32_t usi = 0;
        static constexpr std::uint32_t uti = 4;
        static constexpr std::uint32_t uei = 8;
    };/*interrupts*/
    struct exceptions {
    };/*exceptions*/
} /* riscv */


#endif /* RISCV_INTERRUPTS_HPP */

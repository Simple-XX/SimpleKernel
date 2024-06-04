/*
   Simple machine mode timer driver for RISC-V standard timer.
   SPDX-License-Identifier: Unlicense

   https://five-embeddev.com/

*/

#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdint>
#include <chrono>

namespace driver {

    /** SiFive-hifive1-revb TIMER device parameters
     */
    struct default_timer_config {
        // See
        // freedom-e-sdk/bsp/sifive-hifive1-revb/core.dts
        // psdlfaltclk: clock@6
        // Fixed to 32Khz
        static constexpr unsigned int MTIME_FREQ_HZ=32768;
    };

    /** Default definintion of a the memory mapped mtimer CSR registers.
    The RISC-V spec does not specify and address, so they may be mapped to any address location.
    The addresses here are from freedom-e-sdk/bsp/sifive-hifive1-revb/design.svd
    / /
    */
    struct mtimer_address_spec {
        static constexpr std::uintptr_t MTIMECMP_ADDR = 0x2000000 + 0x4000;
        static constexpr std::uintptr_t MTIME_ADDR = 0x2000000 + 0xBFF8;
    };

    /** Simple TIMER driver class 
     */
    template<class BASE_DURATION=std::chrono::microseconds,
             class ADDRESS_SPEC=mtimer_address_spec, 
             class CONFIG=default_timer_config> class timer {
    public :

        /** Duration of each timer tick */
        using timer_ticks = std::chrono::duration<int, std::ratio<1, CONFIG::MTIME_FREQ_HZ>>;


        /** Set the timer compare point using a std::chrono::duration timer offset 
         */
        template<class T=BASE_DURATION> void set_time_cmp(T time_offset) {
            set_ticks_time_cmp(std::chrono::duration_cast<timer_ticks>(time_offset));
        }
        /** Get the system timer as a std::chrono::duration value 
         */
        template<class T=BASE_DURATION> T get_time(void) {
            return std::chrono::duration_cast<T>(get_ticks_time());
        }
        /** Set the time compare point in ticks of the system timer counter.
         */
        void set_ticks_time_cmp(timer_ticks time_offset) {
            set_raw_time_cmp(time_offset.count());
        }
        /** Return the current system time as a duration since the mtime counter was initialized 
         */
        timer_ticks get_ticks_time(void) {
            return timer_ticks(get_raw_time());
        }
        /** Set the raw time compare point in system timer clocks.
         * @param clock_offset Time relative to current mtime when 
         * @note The time range of the 64 bit timer is large enough not to consider a wrap around of mtime.
         * An interrupt will be generated at mtime + clock_offset.
         * See http://five-embeddev.com/riscv-isa-manual/latest/machine.html#machine-timer-registers-mtime-and-mtimecmp
         */
        void set_raw_time_cmp(uint64_t clock_offset) {
            // First of all set 
            auto new_mtimecmp = get_raw_time() + clock_offset;
            if constexpr ( __riscv_xlen == 64) {
                // Single bus access
                auto mtimecmp = reinterpret_cast<volatile std::uint64_t *>(ADDRESS_SPEC::MTIMECMP_ADDR);
                *mtimecmp = new_mtimecmp;
            } else {
                auto mtimecmpl = reinterpret_cast<volatile std::uint32_t *>(ADDRESS_SPEC::MTIMECMP_ADDR);
                auto mtimecmph = reinterpret_cast<volatile std::uint32_t *>(ADDRESS_SPEC::MTIMECMP_ADDR+4);
                // AS we are doing 32 bit writes, an intermediate mtimecmp value may cause spurious interrupts.
                // Prevent that by first setting the dummy MSB to an unacheivable value
                *mtimecmph = 0xFFFFFFFF;  // cppcheck-suppress redundantAssignment
                // set the LSB
                *mtimecmpl = static_cast<uint32_t>(new_mtimecmp & 0x0FFFFFFFFUL);
                // Set the correct MSB
                *mtimecmph = static_cast<uint32_t>(new_mtimecmp >> 32); // cppcheck-suppress redundantAssignment
            }
        }

        /** Read the raw time of the system timer in system timer clocks
         */
        uint64_t get_raw_time(void) {
            if constexpr ( __riscv_xlen == 64) {
                // Directly read 64 bit value
                auto mtime = reinterpret_cast<volatile std::uint64_t *>(ADDRESS_SPEC::MTIME_ADDR);
                return *mtime;
            } else {
                auto mtimel = reinterpret_cast<volatile std::uint32_t *>(ADDRESS_SPEC::MTIME_ADDR);
                auto mtimeh = reinterpret_cast<volatile std::uint32_t *>(ADDRESS_SPEC::MTIME_ADDR+4);
                uint32_t mtimeh_val;
                uint32_t mtimel_val;
                do {
                    // There is a small risk the mtimeh will tick over after reading mtimel
                    mtimeh_val = *mtimeh;
                    mtimel_val = *mtimel;
                    // Poll mtimeh to ensure it's consistent after reading mtimel
                    // The frequency of mtimeh ticking over is low
                } while (mtimeh_val != *mtimeh);
                return (static_cast<std::uint64_t>(mtimeh_val)<<32)|mtimel_val;
            } 
        }
    };

}

#endif // #ifdef TIMER_HPP


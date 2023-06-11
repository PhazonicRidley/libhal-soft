// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <array>
#include <span>

#include <libhal/adc.hpp>
#include <libhal/output_pin.hpp>
#include <libhal/steady_clock.hpp>

namespace hal {

/**
 * @brief A driver for Analog to Digital mulitplexers, creates adc pin objects
 * for each mux output
 */
class adc_multiplexer
{
public:
  static adc_multiplexer create(std::span<hal::output_pin*> p_signal_pins,
                                hal::adc& p_source_pin);

  /**
   * @brief Reads the channel
   * @param p_mux_port
   * @return result<adc::read_t>
   */
  hal::result<hal::adc::read_t> read_channel(std::uint16_t p_mux_port);

private:
  /**
   * @param p_signal_pins A span of output pins to represent the signal pins of
   * the mux, assuming the mux signals digitally.
   * @param p_source_pin The adc source pin that reads the output of the mux.
   */
  adc_multiplexer(std::span<output_pin*> p_signal_pins, hal::adc& p_source_pin);

private:
  std::span<output_pin*> m_signal_pins;
  hal::adc* m_source_pin;
};

/**
 * @brief An internal class of a hal ADC implimentation to represent a
 * multiplexer pin
 */
class adc_mux_pin : public hal::adc
{
  /**
   * @brief Returns a multiplexer ADC pin
   * @param p_multiplexer The multiplexer object to manage each mux pin.
   * @param p_channel The channel number of the pin.
   */
  friend adc_mux_pin make_adc(adc_multiplexer& p_multiplexer,
                              std::uint8_t p_channel);

private:
  adc_mux_pin(adc_multiplexer& p_mux, std::uint8_t p_mux_port);
  hal::result<read_t> driver_read() override;

  adc_multiplexer* m_mux;
  std::uint8_t m_mux_port;
};

adc_mux_pin make_adc(adc_multiplexer& p_multiplexer, std::uint8_t p_channel);

}  // namespace hal

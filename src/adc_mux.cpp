#include <libhal-soft/adc_mux.hpp>

namespace hal {
using namespace hal::literals;
using namespace std::chrono_literals;

// free functions

/**
 * @brief Set the adc mux to a channel
 *
 * @param p_position The desired channel
 * @param p_signal_pins A span of the source pins
 * @return status
 */
hal::status set_mux_channel(std::uint16_t p_position,
                            std::span<output_pin*> p_signal_pins)
{

  for (std::size_t i = 0; i < p_signal_pins.size(); i++) {

    bool value = bool(p_position & (1 << i));
    HAL_CHECK(p_signal_pins[i]->level(value));
  }
  return success();
}

adc_mux_pin make_adc(adc_multiplexer& p_multiplexer, std::uint8_t p_channel)
{
  return adc_mux_pin(p_multiplexer, p_channel);
}

// impls for adc_multiplexer

adc_multiplexer::adc_multiplexer(std::span<output_pin*> p_signal_pins,
                                 hal::adc& p_source_pin)
  : m_signal_pins{ p_signal_pins }
  , m_source_pin{ &p_source_pin } {};

adc_multiplexer adc_multiplexer::create(
  std::span<hal::output_pin*> p_signal_pins,
  hal::adc& p_source_pin)
{
  return adc_multiplexer(p_signal_pins, p_source_pin);
}

hal::result<hal::adc::read_t> adc_multiplexer::read_channel(
  std::uint16_t p_mux_port)
{
  set_mux_channel(p_mux_port, m_signal_pins);
  return HAL_CHECK(m_source_pin->read());
}

// impls for adc_mux_pin
adc_mux_pin::adc_mux_pin(adc_multiplexer& p_mux, std::uint8_t p_mux_port)
  : m_mux{ &p_mux }
  , m_mux_port{ p_mux_port } {};

hal::result<hal::adc::read_t> adc_mux_pin::driver_read()
{
  return m_mux->read_channel(m_mux_port);
}
}  // namespace hal

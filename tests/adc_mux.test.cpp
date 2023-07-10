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

#include <libhal-soft/adc_mux.hpp>

#include <queue>
#include <vector>

#include <libhal-mock/adc.hpp>
#include <libhal-mock/output_pin.hpp>

#include <boost/ut.hpp>

namespace hal {

void adc_mux_test()
{
  // Set up for all tests
  using namespace boost::ut;
  using level_t = hal::output_pin::level_t;
  using read_t = hal::adc::read_t;

  // configure virtual testing pins
  constexpr int num_pins = 2;
  auto mock_pin0 = mock::output_pin();
  auto mock_pin1 = mock::output_pin();
  std::array<hal::output_pin*, num_pins> signal_pins = { &mock_pin0,
                                                         &mock_pin1 };
  for (auto& s : signal_pins) {
    s->level() = level_t{ .state = false };
  }

  mock::adc source_adc = mock::adc();
  auto adc_sample_pool = std::array<read_t, 4>{ read_t{ .sample = 0 },
                                                read_t{ .sample = 1.5 },
                                                read_t{ .sample = 2 },
                                                read_t{ .sample = 2.5 } };
  auto sample_queue = std::queue<read_t>();
  for (auto& s : adc_sample_pool) {
    sample_queue.push(s);
  }
  source_adc.set(sample_queue);

  // Expected constants

  "adc_mux"_test = [signal_pins, source_adc]() mutable {
    // Setup
    adc_multiplexer test_mux = adc_multiplexer::create(signal_pins, source_adc);
    adc_mux_pin test_adc_port_zero = hal::make_adc(test_mux, 0);
    constexpr float expected_sample_zero = 0;

    // Exercise
    auto test_sample = test_adc_port_zero.read().value().sample;

    // Verify
    expect(that % expected_sample_zero == test_sample);
  };

  // simulate reading from another pin
  "adc_mux_pin_switch"_test = [signal_pins, source_adc]() mutable {
    // Setup
    adc_multiplexer test_mux = adc_multiplexer::create(signal_pins, source_adc);
    adc_mux_pin first_mux_pin = hal::make_adc(test_mux, 1);
    adc_mux_pin second_mux_pin = hal::make_adc(test_mux, 2);
    constexpr float expected_sample_zero = 0;
    constexpr float expected_sample_three_halves = 1.5;

    // Exercise
    auto first_test_value = first_mux_pin.read().value().sample;
    auto first_signal_state =
      std::pair<bool, bool>{ signal_pins[0]->level().value().state,
                             signal_pins[1]->level().value().state };
    auto second_test_value = second_mux_pin.read().value().sample;
    auto second_signal_state =
      std::pair<bool, bool>{ signal_pins[0]->level().value().state,
                             signal_pins[1]->level().value().state };

    // Verify

    // Verify the first read value and multiplexer state.
    expect(that % expected_sample_zero == first_test_value);
    expect(that % true == first_signal_state.first);
    expect(that % false == first_signal_state.second);

    // Verify the second read value and multiplexer state.
    expect(that % expected_sample_three_halves == second_test_value);
    expect(that % false == second_signal_state.first);
    expect(that % true == second_signal_state.second);
  };

  "adc_mux_error"_test = [signal_pins, source_adc]() mutable {
    // Set up
    adc_multiplexer test_mux = adc_multiplexer::create(signal_pins, source_adc);

    // Exercise
    auto test_read_data = std::array<hal::result<hal::adc::read_t>, 4>{
      hal::make_adc(test_mux, 0).read(),
      hal::make_adc(test_mux, 1).read(),
      hal::make_adc(test_mux, 2).read(),
      hal::make_adc(test_mux, 3).read()
    };
    auto error_test = hal::make_adc(test_mux, 4).read();

    // Verify
    for (auto& p : test_read_data) {
      expect(that % true == p.has_value());
    }
    expect(that % true == error_test.has_error());
  };
};

}  // namespace hal
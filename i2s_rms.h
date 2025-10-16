#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
extern "C"
{
#include "driver/i2s.h"
}

namespace esphome
{
  namespace i2s_rms
  {

    class I2SRMSSensor : public PollingComponent, public sensor::Sensor
    {
    public:
      // Pins + rate (configurable via Python stub if you added set_pins)
      int pin_bclk = 26;
      int pin_ws = 27;
      int pin_data = 33;
      int sample_rate = 44100;

      I2SRMSSensor() : PollingComponent(25) {}

      // Allow YAML/lambda to set pins (optional, if you added it in sensor.py)
      void set_pins(int bclk, int ws, int data)
      {
        pin_bclk = bclk;
        pin_ws = ws;
        pin_data = data;
      }

      // Deferred control
      void start();
      void stop();
      bool is_started() const { return started_; }

      // Component hooks
      void setup() override; // now: no driver install here
      void update() override;
      void dump_config() override;

    protected:
      bool started_{false};
    };

  } // namespace i2s_rms
} // namespace esphome

#include "i2s_rms.h"
#include "esphome/core/log.h"

namespace esphome
{
  namespace i2s_rms
  {

    static const char *TAG = "i2s_rms";

    void I2SRMSSensor::setup()
    {
      // Defer starting I²S until user enables it (or wires are connected)
      ESP_LOGI(TAG, "Deferred start: waiting for enable before installing I2S RX driver.");
    }

    void I2SRMSSensor::start()
    {
      if (started_)
        return;

      i2s_config_t cfg = {
          .mode = (i2s_mode_t)(I2S_MODE_SLAVE | I2S_MODE_RX), // listener only
          .sample_rate = (uint32_t)sample_rate,
          .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
          .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
          .communication_format = I2S_COMM_FORMAT_STAND_I2S,
          .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
          .dma_buf_count = 4,
          .dma_buf_len = 256,
          .use_apll = false,
          .tx_desc_auto_clear = false,
          .fixed_mclk = 0};

      i2s_pin_config_t pins = {
          .bck_io_num = pin_bclk,            // BCLK IN
          .ws_io_num = pin_ws,               // LRCLK IN
          .data_out_num = I2S_PIN_NO_CHANGE, // never drive outputs
          .data_in_num = pin_data            // DATA IN
      };

      if (i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) != ESP_OK)
      {
        ESP_LOGE(TAG, "i2s_driver_install failed");
        return;
      }
      if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK)
      {
        ESP_LOGE(TAG, "i2s_set_pin failed");
        i2s_driver_uninstall(I2S_NUM_0);
        return;
      }

      i2s_zero_dma_buffer(I2S_NUM_0);
      started_ = true;
      ESP_LOGI(TAG, "I2S RX started: BCLK=%d, WS=%d, DATA=%d", pin_bclk, pin_ws, pin_data);
    }

    void I2SRMSSensor::stop()
    {
      if (!started_)
        return;
      i2s_driver_uninstall(I2S_NUM_0);
      started_ = false;
      ESP_LOGI(TAG, "I2S RX stopped.");
    }

    void I2SRMSSensor::update()
    {
      if (!started_)
      {
        // Not started: advertise silence and exit quickly (no watchdog issues)
        publish_state(0.0f);
        return;
      }

      // Non-blocking read; if no clocks/data, we still return immediately
      int16_t buf[1024];
      size_t bytes_read = 0;
      esp_err_t err = i2s_read(I2S_NUM_0, buf, sizeof(buf), &bytes_read, 0);
      if (err != ESP_OK || bytes_read == 0)
      {
        publish_state(0.0f);
        return;
      }

      // Compute RMS on the chunk we got
      int n = bytes_read / sizeof(int16_t);
      double acc = 0;
      for (int i = 0; i < n; i++)
      {
        int32_t s = buf[i];
        acc += (double)s * (double)s;
      }

      float rms = sqrt(acc / (double)n) / 32768.0f; // 0..1
      float loud = rms * 2.0f;                      // mild gain
      if (loud > 1.0f)
        loud = 1.0f;

      publish_state(loud);
    }

    void I2SRMSSensor::dump_config()
    {
      ESP_LOGCONFIG(TAG, "I2S RMS Sensor (Slave RX, deferred start)");
      LOG_SENSOR("  ", "Loudness", this);
      ESP_LOGCONFIG(TAG, "  BCLK Pin: %d", pin_bclk);
      ESP_LOGCONFIG(TAG, "  WS Pin: %d", pin_ws);
      ESP_LOGCONFIG(TAG, "  DATA Pin: %d", pin_data);
      ESP_LOGCONFIG(TAG, "  Sample Rate: %d Hz", sample_rate);
      ESP_LOGCONFIG(TAG, "  Started: %s", started_ ? "yes" : "no");
    }

  } // namespace i2s_rms
} // namespace esphome

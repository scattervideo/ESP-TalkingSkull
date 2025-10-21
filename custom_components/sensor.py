import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL, UNIT_EMPTY, ICON_SIGNAL

CONF_PIN_BCLK = "pin_bclk"
CONF_PIN_WS   = "pin_ws"
CONF_PIN_DATA = "pin_data"

i2s_rms_ns = cg.esphome_ns.namespace("i2s_rms")
I2SRMSSensor = i2s_rms_ns.class_("I2SRMSSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_SIGNAL,
    accuracy_decimals=2
).extend({
    cv.GenerateID(): cv.declare_id(I2SRMSSensor),
    cv.Optional(CONF_UPDATE_INTERVAL, default="25ms"): cv.update_interval,
    cv.Optional(CONF_PIN_BCLK, default=26): cv.int_,
    cv.Optional(CONF_PIN_WS,   default=27): cv.int_,
    cv.Optional(CONF_PIN_DATA, default=33): cv.int_,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(var.set_pins(config[CONF_PIN_BCLK],
                        config[CONF_PIN_WS],
                        config[CONF_PIN_DATA]))

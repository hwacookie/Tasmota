/*
  xnrg_01_hlw8012.ino - HLW8012 (Sonoff Pow) energy sensor support for Tasmota

  Copyright (C) 2021  Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_ENERGY_SENSOR
#ifdef USE_LED_METER
/*********************************************************************************************\
 * LED_METER use blinking LED at Power Meter
 *
 * Based on Source: Shenzhen Heli Technology Co., Ltd
\*********************************************************************************************/

#define XNRG_30                1

#define LDR_PIN  3

#define LED_PIN  1



volatile uint32_t tick_2 = 0;
volatile uint32_t tick_1 = 0;
volatile uint32_t tick_0 = 0;
volatile uint32_t tick_now = 0;
volatile bool ledOn = false;
float curEnergy = 0.0;
volatile uint32_t tickCounter = 0;


/********************************************************************************************/

// Fix core 2.5.x ISR not in IRAM Exception
#ifndef USE_WS2812_DMA  // Collides with Neopixelbus but solves exception
void LDRInterrupt(void) ICACHE_RAM_ATTR;
#endif  // USE_WS2812_DMA

/**
 * This method is called when a falling flank of the LDR-PIN is detected.
 */
void LDRInterrupt(void)  
{
  tick_now = micros();
  if ((tick_now-tick_0) > 100000) {
    tick_2 = tick_1;
    tick_1 = tick_0;
    tick_0 = tick_now;
    ledOn = true;
    tickCounter++;
  }
}

void LEDMeterEvery200ms(void) {
  #ifdef LED_PIN
  if (ledOn) {
    digitalWrite(LED_PIN, 1);
  } else {
    digitalWrite(LED_PIN, 0);
  }
  ledOn = false;
  #endif  
}

/**
 * Update the stats every second
 */
void LedMeterEverySecond(void)
{

  float diff = tick_0 - tick_2;
  if (diff>0) {
    curEnergy = (3600000000.0 / diff) * 4.0;
    Energy.active_power[0] = curEnergy;
    Energy.data_valid[0] = 1;
  }

  EnergyUpdateTotal(tickCounter * 2, false);
  AddLog(LOG_LEVEL_DEBUG, PSTR("NRG: curEnergy:%4_f ticks: %d"), &curEnergy, tickCounter);
}

void LedMeterSnsInit(void)
{
  #ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 1);
  #endif

  pinMode(LDR_PIN, INPUT_PULLUP);
  attachInterrupt(LDR_PIN, LDRInterrupt, FALLING);
}

void LedMeterDrvInit(void)
{
  Energy.current[0] = NAN;
  Energy.voltage[0] = NAN;
  Energy.active_power[0] = NAN;;
  Energy.data_valid[0] = 0;
  Energy.power_factor[0] = NAN;
  Energy.frequency[0] = NAN;
  Energy.apparent_power[0] = NAN;
  Energy.reactive_power[0] = NAN;
  Energy.voltage_common = true;                  // Use single voltage
  Energy.frequency_common = true;                // Use single frequency
  Energy.current_available = false;
  Energy.voltage_available = false;

  AddLog(LOG_LEVEL_DEBUG, PSTR("NRG: Register LED_METER"));
  TasmotaGlobal.energy_driver = XNRG_30;
}

bool LedMeterCommand(void)
{

  bool serviced = true;

/*
  if ((CMND_POWERCAL == Energy.command_code) || (CMND_VOLTAGECAL == Energy.command_code) || (CMND_CURRENTCAL == Energy.command_code)) {
    // Service in xdrv_03_energy.ino
  }

  else if (CMND_ENERGY_RESET == Energy.command_code) {
    if (XdrvMailbox.data_len && Hlw.cf_power_pulse_length ) {
      Settings.energy_power_calibration = ((uint32_t)(CharToFloat(XdrvMailbox.data) * 10) * Hlw.cf_power_pulse_length ) / Hlw.power_ratio;
    }
  }
  else if (CMND_VOLTAGESET == Energy.command_code) {
    if (XdrvMailbox.data_len && Hlw.cf1_voltage_pulse_length ) {
      Settings.energy_voltage_calibration = ((uint32_t)(CharToFloat(XdrvMailbox.data) * 10) * Hlw.cf1_voltage_pulse_length ) / Hlw.voltage_ratio;
    }
  }
  else if (CMND_CURRENTSET == Energy.command_code) {
    if (XdrvMailbox.data_len && Hlw.cf1_current_pulse_length) {
      Settings.energy_current_calibration = ((uint32_t)(CharToFloat(XdrvMailbox.data)) * Hlw.cf1_current_pulse_length) / Hlw.current_ratio;
    }
  }
  else 
  */
  serviced = false;  // Unknown command

  return serviced; 
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xnrg30(uint8_t function)
{
  bool result = false;

  switch (function) {
    case FUNC_EVERY_200_MSECOND:
      LEDMeterEvery200ms();
      break;
    case FUNC_ENERGY_EVERY_SECOND:
      LedMeterEverySecond();
      break;
    case FUNC_COMMAND:
      result = LedMeterCommand();
      break;
    case FUNC_INIT:
      LedMeterSnsInit();
      break;
    case FUNC_PRE_INIT:
      LedMeterDrvInit();
      break;
  }
  return result;
}

#endif  // USE_LED_METER
#endif  // USE_ENERGY_SENSOR

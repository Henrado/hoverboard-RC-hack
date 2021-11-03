# Hoverboard-RC-hack
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Dette reoet implementerer Field Oriented Control (FOC) for vanlige hoverboards. Sammenlignet med kommuteringsmetoden (som de kommer med) tilbyr denne FOC-kontrollmetoden overlegen ytelse med:
  - redusert støy og vibrasjoner
  - jevnt dreiemoment og forbedret motoreffektivitet. Dermed lavere energiforbruk
  - feltsvekkelse for å øke maksimalt hastighetsområde
  - **Mulighet for å kontrollere motorene over seriell komunikaskjon.**


Table of Contents
=======================

* [Planen fremover](#hardware)
* [Hardware](#hardware)
* [FOC Firmware](#foc-firmware)
* [Flashing](#flashing)
* [Troubleshooting](#troubleshooting)
* [Diagnostics](#diagnostics)
* [Projects and Links](#projects-and-links)
* [Contributions](#contributions)

#### For the hoverboard sideboard firmware, see the following repositories:
 - [hoverboard-sideboard-hack-GD](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-GD)
 - [hoverboard-sideboard-hack-STM](https://github.com/EmanuelFeru/hoverboard-sideboard-hack-STM)

#### For the FOC controller design, see the following repository:
 - [bldc-motor-control-FOC](https://github.com/EmanuelFeru/bldc-motor-control-FOC)

#### Videos:
<table>
  <tr>
    <td><a href="https://youtu.be/IgHCcj0NgWQ" title="Hovercar" rel="noopener"><img src="/docs/pictures/videos_preview/hovercar_intro.png"></a></td>
    <td><a href="https://youtu.be/gtyqtc37r10" title="Cruise Control functionality" rel="noopener"><img src="/docs/pictures/videos_preview/cruise_control.png"></a></td>
    <td><a href="https://youtu.be/jadD0M1VBoc" title="Hovercar pedal functionality" rel="noopener"><img src="/docs/pictures/videos_preview/hovercar_pedals.png"></a></td>
  </tr>
  <tr>
    <td><a href="https://youtu.be/UnlbMrCkjnE" title="Commutation vs. FOC (constant speed)" rel="noopener"><img src="/docs/pictures/videos_preview/com_foc_const.png"></a></td>
    <td><a href="https://youtu.be/V-_L2w10wZk" title="Commutation vs. FOC (variable speed)" rel="noopener"><img src="/docs/pictures/videos_preview/com_foc_var.png"></a></td>       
    <td><a href="https://youtu.be/tVj_lpsRirA" title="Reliable Serial Communication" rel="noopener"><img src="/docs/pictures/videos_preview/serial_com.png"></a></td>
  </tr>
</table>

---
## Planen fremover


---
## Hardware

![mainboard_pinout](/docs/pictures/mainboard_pinout.png)

The original Hardware supports two 4-pin cables that originally were connected to the two sideboards. They break out GND, 12/15V and USART2&3 of the Hoverboard mainboard. Both USART2&3 support UART, PWM, PPM, and iBUS input. Additionally, the USART2 can be used as 12bit ADC, while USART3 can be used for I2C. Note that while USART3 (right sideboard cable) is 5V tolerant, USART2 (left sideboard cable) is **not** 5V tolerant.

Typically, the mainboard brain is an [STM32F103RCT6](/docs/literature/[10]_STM32F103xC_datasheet.pdf), however some mainboards feature a [GD32F103RCT6](/docs/literature/[11]_GD32F103xx-Datasheet-Rev-2.7.pdf) which is also supported by this firmware.

For the reverse-engineered schematics of the mainboard, see [20150722_hoverboard_sch.pdf](/docs/20150722_hoverboard_sch.pdf)


---
## FOC Firmware

In this firmware 3 control types are available:
- Commutation
- SIN (Sinusoidal)
- FOC (Field Oriented Control) with the following 3 control modes:
  - **VOLTAGE MODE**: in this mode the controller applies a constant Voltage to the motors. Recommended for robotics applications or applications where a fast motor response is required.
  - **SPEED MODE**: in this mode a closed-loop controller realizes the input speed target by rejecting any of the disturbance (resistive load) applied to the motor. Recommended for robotics applications or constant speed applications.
  - **TORQUE MODE**: in this mode the input torque target is realized. This mode enables motor "freewheeling" when the torque target is `0`. Recommended for most applications with a sitting human driver.

#### Comparison between different control methods

|Control method| Complexity | Efficiency | Smoothness | Field Weakening | Freewheeling | Standstill hold |
|--|--|--|--|--|--|--|
|Commutation| - | - | ++ | n.a. | n.a. | + |
|Sinusoidal| + | ++ | ++ | +++ | n.a. | + |
|FOC VOLTAGE| ++ | +++ | ++ | ++ | n.a. | +<sup>(2)</sup> |
|FOC SPEED| +++ | +++ | + | ++ | n.a. | +++ |
|FOC TORQUE| +++ | +++ | +++ | ++ | +++<sup>(1)</sup> | n.a<sup>(2)</sup> |

<sup>(1)</sup> By enabling `ELECTRIC_BRAKE_ENABLE` in `config.h`, the freewheeling amount can be adjusted using the `ELECTRIC_BRAKE_MAX` parameter.<br/>
<sup>(2)</sup> The standstill hold functionality can be forced by enabling `STANDSTILL_HOLD_ENABLE` in `config.h`.

In all FOC control modes, the controller features maximum motor speed and maximum motor current protection. This brings great advantages to fulfil the needs of many robotic applications while maintaining safe operation.


### Field Weakening / Phase Advance

 - By default the Field weakening is disabled. You can enable it in config.h file by setting the FIELD_WEAK_ENA = 1
 - The Field Weakening is a linear interpolation from 0 to FIELD_WEAK_MAX or PHASE_ADV_MAX (depeding if FOC or SIN is selected, respectively)
 - The Field Weakening starts engaging at FIELD_WEAK_LO and reaches the maximum value at FIELD_WEAK_HI
 - The figure below shows different possible calibrations for Field Weakening / Phase Advance
 ![Field Weakening](/docs/pictures/FieldWeakening.png)
 - If you re-calibrate the Field Weakening please take all the safety measures! The motors can spin very fast!


### Parameters
 - All the calibratable motor parameters can be found in the 'BLDC_controller_data.c'. I provided you with an already calibrated controller, but if you feel like fine tuning it feel free to do so
 - The parameters are represented in Fixed-point data type for a more efficient code execution
 - For calibrating the fixed-point parameters use the [Fixed-Point Viewer](https://github.com/EmanuelFeru/FixedPointViewer) tool
 - The controller parameters are given in [this table](https://github.com/EmanuelFeru/bldc-motor-control-FOC/blob/master/02_Figures/paramTable.png)


---
## Flashing

Right to the STM32, there is a debugging header with GND, 3V3, SWDIO and SWCLK. Connect GND, SWDIO and SWCLK to your SWD programmer, like the ST-Link found on many STM devboards.

If you have never flashed your sideboard before, the MCU is probably locked. To unlock the flash, check-out the wiki page [How to Unlock MCU flash](https://github.com/EmanuelFeru/hoverboard-firmware-hack-FOC/wiki/How-to-Unlock-MCU-flash).

Do not power the mainboard from the 3.3V of your programmer! This has already killed multiple mainboards.

Make sure you hold the powerbutton or connect a jumper to the power button pins while flashing the firmware, as the STM might release the power latch and switches itself off during flashing. Battery > 36V have to be connected while flashing.

To build and flash choose one of the following methods:

Flere forskjellige måter å velge mellom men jeg valgte å bruke:
**[RoboDurden's](https://github.com/RoboDurden) online compiler:[https://pionierland.de/hoverhack/](https://pionierland.de/hoverhack/)**

---
## Troubleshooting
First, check that power is connected and voltage is >36V while flashing.
If the board draws more than 100mA in idle, it's probably broken.

If the motors do something, but don't rotate smooth and quietly, try to use an alternative phase mapping. Usually, color-correct mapping (blue to blue, green to green, yellow to yellow) works fine. However, some hoverboards have a different layout then others, and this might be the reason your motor isn't spinning.

Nunchuk not working: Use the right one of the 2 types of nunchuks. Use i2c pullups.

Nunchuk or PPM working bad: The i2c bus and PPM signal are very sensitive to emv distortions of the motor controller. They get stronger the faster you are. Keep cables short, use shielded cable, use ferrits, stabilize voltage in nunchuk or reviever, add i2c pullups. To many errors leads to very high accelerations which triggers the protection board within the battery to shut everything down.

Recommendation: Nunchuk Breakout Board https://github.com/Jan--Henrik/hoverboard-breakout

Most robust way for input is to use the ADC and potis. It works well even on 1m unshielded cable. Solder ~100k Ohm resistors between ADC-inputs and gnd directly on the mainboard. Use potis as pullups to 3.3V.


---
## Diagnostics
The errors reported by the board are in the form of audible beeps:
- **1 beep  (low pitch)**: Motor error (see [possible causes](https://github.com/EmanuelFeru/bldc-motor-control-FOC#diagnostics))
- **2 beeps (low pitch)**: ADC timeout
- **3 beeps (low pitch)**: Serial communication timeout
- **4 beeps (low pitch)**: General timeout (PPM, PWM, Nunchuk)
- **5 beeps (low pitch)**: Mainboard temperature warning
- **1 beep slow (medium pitch)**: Low battery voltage < 36V
- **1 beep fast (medium pitch)**: Low battery voltage < 35V
- **1 beep fast (high pitch)**: Backward spinning motors

For a more detailed troubleshooting connect an [FTDI Serial adapter](https://s.click.aliexpress.com/e/_AqPOBr) or a [Bluetooth module](https://s.click.aliexpress.com/e/_A4gkMD) to the DEBUG_SERIAL cable (Left or Right) and monitor the output data using the [Hoverboard Web Serial Control](https://candas1.github.io/Hoverboard-Web-Serial-Control/) tool developed by [Candas](https://github.com/Candas1/).

---
## Projects and Links

- **Original firmware:** [https://github.com/NiklasFauth/hoverboard-firmware-hack](https://github.com/NiklasFauth/hoverboard-firmware-hack)
- **[Candas](https://github.com/Candas1/) Hoverboard Web Serial Control:** [https://github.com/Candas1/Hoverboard-Web-Serial-Control](https://github.com/Candas1/Hoverboard-Web-Serial-Control)
- **[RoboDurden's](https://github.com/RoboDurden) online compiler:** [https://pionierland.de/hoverhack/](https://pionierland.de/hoverhack/)
- **Hoverboard hack for AT32F403RCT6 mainboards:** [https://github.com/cloidnerux/hoverboard-firmware-hack](https://github.com/cloidnerux/hoverboard-firmware-hack)
- **Hoverboard hack for split mainboards:** [https://github.com/flo199213/Hoverboard-Firmware-Hack-Gen2](https://github.com/flo199213/Hoverboard-Firmware-Hack-Gen2)
- **Hoverboard hack from BiPropellant:** [https://github.com/bipropellant](https://github.com/bipropellant)
- **Hoverboard breakout boards:** [https://github.com/Jan--Henrik/hoverboard-breakout](https://github.com/Jan--Henrik/hoverboard-breakout)

<a/>

- **Bobbycar** [https://github.com/larsmm/hoverboard-firmware-hack-FOC-bbcar](https://github.com/larsmm/hoverboard-firmware-hack-FOC-bbcar)
- **Wheel chair:** [https://github.com/Lahorde/steer_speed_ctrl](https://github.com/Lahorde/steer_speed_ctrl)
- **TranspOtterNG:** [https://github.com/Jan--Henrik/transpOtterNG](https://github.com/Jan--Henrik/transpOtterNG)
- **ST Community:** [Custom FOC motor control](https://community.st.com/s/question/0D50X0000B28qTDSQY/custom-foc-control-current-measurement-dma-timer-interrupt-needs-review)

<a/>

- **Telegram Community:** If you are an enthusiast join our [Hooover Telegram Group](https://t.me/joinchat/BHWO_RKu2LT5ZxEkvUB8uw)


---
## Contributions

Every contribution to this repository is highly appreciated! Feel free to create pull requests to improve this firmware as ultimately you are going to help everyone.

If you want to donate to keep this firmware updated, please use the link below:

---

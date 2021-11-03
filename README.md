# Hoverboard-RC-hack
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

Dette reoet implementerer Field Oriented Control (FOC) for vanlige hoverboards. Sammenlignet med kommuteringsmetoden (som de kommer med) tilbyr denne FOC-kontrollmetoden overlegen ytelse med:
  - redusert støy og vibrasjoner
  - jevnt dreiemoment og forbedret motoreffektivitet. Dermed lavere energiforbruk
  - feltsvekkelse for å øke maksimalt hastighetsområde
  - **Mulighet for å kontrollere motorene over seriell komunikaskjon.**


Innholdsfortegnelse 
=======================

* [Planen fremover](#hardware)
* [Hardware](#hardware)
* [FOC Firmware](#foc-firmware)
* [Flashing](#flashing)
* [Troubleshooting](#troubleshooting)
* [Diagnostics](#diagnostics)
* [Projects and Links](#projects-and-links)
* [Contributions](#contributions)

#### For hvordan FOC kontrolleren fungerer sjekk ut EmanuelFeru sin forklaring i repoet:
 - [bldc-motor-control-FOC](https://github.com/EmanuelFeru/bldc-motor-control-FOC)

#### Videos:
Dette er bare noe av det man kan bruke den til:
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

Slik har jeg montert min robot:
**BILDER**

---
## Planen fremover
 1. Gå over til "tank" mode slik at leg slipper å bruke hoverbordet sin mixer og jeg kan gi helt konkret fart på hvert hjul
 -- [implement Emanuels suggestion for tank mode](https://github.com/labrats-x7/hoverboard-firmware-hack-FOC/commit/eea5af8deb34350907c21a9ecd53411c79b00102)
 2. Imoplementere ROS til å gjøre det samme som nå
 3. Få ROS til å kontrollere roboten enten inne med lidar eller ute med GPS

Stepp 2 og videre blir inspirasjon fra: 
[Converting a hoverboard into a self-driving mobile robot with ROS](https://medium.com/@alxm/converting-a-hoverboard-into-a-self-driving-mobile-robot-with-ros-d886c867e8a9)

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

**Jeg har valgt å bruke FOC i VOLTAGE MODE for nøyaktighet**

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

Det flere forskjellige måter å kompilere repet men jeg valgte å bruke:
**[RoboDurden's](https://github.com/RoboDurden) online compiler:[https://pionierland.de/hoverhack/](https://pionierland.de/hoverhack/)**


---
## Diagnostics
The errors reported by the board are in the form of audible beeps:
- **1 beep  (low pitch)**: Motor error (see [possible causes](https://github.com/EmanuelFeru/bldc-motor-control-FOC#diagnostics))
- **2 beeps (low pitch)**: ADC timeout
- **3 beeps (low pitch)**: Serial communication timeout <-- Skjer ofte/arduinoen mister kontakten med hoverbordet
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

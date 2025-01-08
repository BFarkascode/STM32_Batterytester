# STM32_Batterytester
Bare metal project to implement a battery tester on an STM32_F405RG Feather board.

## General description
Here is another mini project, this time to show, how to implement an accurate battery level sensing solution on a battery powered system using STM32.

### A few words on the Adafruit Feather
I consider the STM32_F405RG Feather board from Adafruit as a viable alternative to Nucleo boards.

It must be said though that it lacks many of the features we might have got used to already using the L053R8. For instance, it does not have an STLink attached to it, thus we will need to rig one up to it using its SWDIO/SWCLK pads (or use its bootloader, see here: Programming the Adafruit Feather STM32F405 Express with STM32CubeIDE - Maker.io). There are some USB stick-style devices available on the market that can be used for programming it, though one can easily modify the L053R8 Nucleo board (or any other) for the purpose: simply remove the bridges from the CN2 connector (effectively severing the Nucleo’s STLink from the L053R8) and hook up the F405RG to power/GND and the CN4 programming connector. Please note that we won’t be able to communicate with the PC using this solution so “printf” will not work with the IDE’s console this time.

Anyway, the actual feature I find very useful in Feathers is that they all have their own power management systems and associated connectors to run them using a LiPO battery. More precisely, they can charge a battery connected to them when they are powered through USB and have a resistor bridge on their Vbat pins (the pure battery voltage output) to measure the battery voltage using an ADC. They are very much plug-and-play battery powered solutions, perfect for any small MCU-based portable device.

### ADC fail on battery
General knowledge floating around on the internet is that one can simply use an ADC to detect a voltage level on anything connected to an MCU and can take the received/measured value as accurate.

Taking everything granted is not particularly wise though…

When we calibrate the ADC (which we must do on every MCU power-up), we are using Vref – a reference voltage – to define the voltage step that represents one step within the ADC. For example, in case we have a 12-bit ADC with a Vref of 3.3V, then one step of the ADC will represent 3.3 V / 4096 = 0.805 mV. Thus, when we run the ADC and detect a certain number of steps - say, 1800 - we can calculate in software a measured voltage of 3.3V / 4096 * 1800 = 1.45V.

On some devices, Vref for the ADC is broken out on a separate pin, detached from the power supplied to the MCU (the Vdda), in others, the Vref and the Vdda is connected together internally, so the two are the same.

This later is going to be the case in the Feather board we are using. Here, an adequately reliable and stable power supply will just give 3.3V to both the MCU and the ADC – usually the case when powered through USB.

When we are running on battery though, we don’t have a reliable and stable power supply. Hell, even powering the board through an un-optimal supply – like the Nucleo board I am using to program the Feather – we will NOT have exactly 3.3V on the MCU and, by proxy, on the ADC. (Real life example: I had only 3.03V on the Feather board but still used 3.3V for the calculations. As such, the detected 1.45V mentioned above was only 3.03V/4096 * 1800 = 1.33V in reality…) This is of course not a problem if we know the exact Vdda/Vref voltage or have a known external Vref - given such pin is broken out on the MCU – but can be a pain if the immediate Vdda is unknown.

Unfortunately, running solely on battery, we WILL have a Vdda value that will change depending on the battery discharge profile, meaning that the ADC’s Vref will also change. In practical sense, this means that the calculated/measured voltage value will be dropping as long as Vref is greater than the arbitrarily given “assumed” reference voltage (in the above example calculations, 3.3V), but will then start to INCREASE when Vref drops under this “assumed” voltage. This can seriously mess things up where a battery “assumed” to be full will practically be empty instead.

In order to avoid this issue, we need some way to know, what the Vref of the ADC actually is at any given moment. Luckily, there is a way to exactly do that: what we need is to measure the Vrefint internal reference voltage within the MCU and then use that to calculate the Vdda. Since within the Feather that is going to be the same as Vref, we will be able to have an accurate Vref this way, no matter how much voltage the battery will provide to our device.

## Previous relevant projects
I recommend checking my previous ADC project, albeit the ADC works different on the F405 than on the L053:
- SAMD21_ADC-DACDriver

## To read
I have pretty much implemented the solution presented here, ported to the F405 using bare metal instead of HAL:
- https://community.st.com/t5/stm32-mcus/how-to-use-the-stm32-adc-s-internal-reference-voltage/ta-p/621425

I recommend giving the F405 datasheet and refman also a good look since they specify the exact sampling parameters for the ADC as well as give the Vrefint calibration specs: where the value is stored in memory as well as at which voltage it was acquired.

Lastly, one should check the battery characteristics to understand the discharge profile as well as the voltage limits of the battery. It is also critical to know, which pin on the battery connector is power, which one is GND.

## Particularities
### A few words on the battery
I am using a 3.7V LiPO battery from EEMB to power the Feather which, at full charge, will give out 4.2V, then rapidly drop to 3.7V, followed by a slow decrease to 3.2V. Reaching 3.2V, the battery’s internal electronics will shut down power to protect the LiPO cells.

!!!CAUTION IF YOU ARE USING OFF THE SHELF LiPO BATTERIES!!!

Not all batteries come with the same connector pin distribution that is compatible with the Feather! Connecting the wrong distro to the Feather will put power to the GND pin and GND to the power pin which is not a healthy setup for any MCU. If necessary, switch polarity on the battery by removing the cables from the connector and plugging them back the other way around.

### A few words on other stuff
As mentioned above, the ADC works slightly differently in the F405 than in the L053 so the code written before is not compatible with the Feather. The driver is pretty simple though – simpler than for the L053R8 – so I will omit describing it in detail. Technically, the only thing to pay attention to is that we will manually need to tell the ADC, which channels to measure and with what sequence (SQRx registers).

The Vrefint is on the 17th channel and must be activated within the ADC’s init function (Note: the internal thermometer is also activated flipping the same bit just in case one wishes to implement the original L053R8 ADC project with the F405RG). 
The Vrefint calibration for the F405 was done at 3.3V and the values are stored at address 0x1FFF7A2A. The calibration value is a uint16_t, so an appropriately sized pointer was set to read it out.

The Vbat’s 50/50 resistor bridge is connected to the PA3 pin on the Feather, so we will need the ADC_IN3 – the 3rd channel of the ADC – to measure the battery.

## User guide
Not much to say. We are reading out the battery value every second and store it in a variable. Depending on the value we read out, if it is lower than a threshold (here, 3.4 V), we turn on the Feather’s internal LED to signal the battery is running low.
We can easily test the code by powering it through both the Nucleo and the battery and then removing the battery. With a properly charged battery, the LED will be off when the battery is attached, while it will light up with the battery removed.

## Conclusion
My solution is rather clunky since I am merely executing two single ADC conversions, simply changing the channel we are reading out. This could be improved using a DMA, though for my immediate use case, such slow solution is perfectly adequate.


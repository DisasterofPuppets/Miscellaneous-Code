# Miscellaneous Code Repository and Skydiving

A bunch of stuff here that may be helpful for troubleshooting.
As always...

## ⚠️ Disclaimer
> **Use any of this code at your own risk.** While designed for safe testing, incorrect usage or testing may damage hardware, mark your firstborn a crow familiar, or you may even lose a leg. Always ensure you understand your wiring and component limits. If you break hardware/yourself, the author is not liable.



## 📋 Code Repository by Platform

  ### &nbsp;&nbsp;📟 Arduino IDE
  
  - **⚡ ESP32 GPIO Test - Voltage Check**
    
    **Code:**&nbsp;&nbsp;&nbsp;&nbsp;[ESP32_GPIO_Test.ino](https://github.com/DisasterofPuppets/Miscellaneous-Code/blob/main/ESP32_GPIO_Test.ino)  
    
    **Usage:**&nbsp;&nbsp;&nbsp;Enter the pin number in serial monitor and the program will set it to High.
    
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Measure between ground and selected pin to confirm 3.3v measurement - if not, pin may be faulty.
    
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Additional instructions included in code comments.

  - **🎤 Home Assistant - ESP32 Speaker and Microphone Test**
    
    **Code:**&nbsp;&nbsp;&nbsp;&nbsp;[HomeAssistant_ESP32_Satellite_Soundcheck.ino](https://github.com/DisasterofPuppets/Miscellaneous-Code/blob/main/HomeAssistant_ESP32_Satelite_Soundcheck.ino)
    
    **Usage:**&nbsp;&nbsp;&nbsp;You will be prompted to select either the microphone or speaker check by entering a number in the serial.
    
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The Speaker check will ask for a tone (440 is good) and play it for 5 seconds to confirm the speaker is working
    
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The Microphone check will blink the onboard LED to the volume of your voice to confirm audio input is working.
   
   
   

# AnchorOTA Library
The AnchorOTA library enables over the air updates for the BW16 RTL8720DN boards. This library is written for Arduino but should be easily modifiable to work on other platforms.

## Prerequisites
---
For this Library to work the Ameba Arduino Kit has to be installed:  
https://github.com/ambiot/ambd_arduino  

A full guide on how to upload code to BW16 can be found here:
https://www.instructables.com/RTL8720DN/  

Download or Clone this repository and pull the folder into your Arduino libraries folder or use the 'Add .ZIP Libraries' function in the ArduinoIDE.  

## Step by step instructions
---
### 1 - Load example
Start by going to File>Examples>AnchorOTA and chosing one of the examples provided by the library.
- **ota_basic** will have the chip waiting for you to send it updates
- **ota_cloud_basic** the chip will regularily request updates from a provided Ip addresss
- **ota_non_block** and **ota_cloud_non_block** contain the same functionalities as ota_basic and ota_coud_basic but allow the program to continue while waiting for updates
### 2 - Add WiFi configuration
Whichever example you chose, you will need to edit the sketch in the lines shown here by adding your WiFi SSID and Password.

    char ssid[] = "yourNetwork";     //  your network SSID (name)
    char pass[] = "secretPassword";  // your network password

### 3 - Upload the sketch to BW16
If the guide in prerequisites has been followed, this should work.  (If a successful upload to address 1 does not change the chip behavior, the reason might be an earlier ota update into ota address 2 which is always checked first by the bootloader. To fix use 'erase flash' feature of the Arduino IDE before uploading again.)  
### 4 - Compile a sketch to use in the OTA update
Use a new or modify the current sketch to recognize a successful update, e.g.

    Serial.println("Updated bin");
Compile the file using 'Verify' in the ArduinoIDE.
### 5 - Save the compiled file
The compiled binary can be found as **km0_km4_image2.bin** under:

    C:\Users\User\AppData\Local\Arduino15\packages\realtek\tools\ameba_d_tools\1.0.8
Depending on the chosen example sketch the .bin file will have to be saved
- in the **UploadServer** folder if you used **ota_basic**
- in the **DownloadServer** folder if you used **ota_cloud**
### 6 - Start the OTA
Edit start.bat to add the right port, ip address, and filename, or call the .exe directly.
#### For UploadServer use command:
    UploadServer -f [filename] -i [chip ip address] -p [port]
#### For DownloadServer use command:
    DownloadServer [port] [filename]
Alternatively there are Python files with the same functionality in the respective folders.
### 7 - Repeat the OTA
Updating over the air should work infinite times, as long as the updated binaries contain OTA functionality.

## OTA Updates on bw16 rtl8720dn
---
-	OTA Address 1: 0x00006000  
-	OTA Address 2: 0x00106000  
-	Address 2 is always checked first  
-	Images in the Address are always expected as image for chip km0 (low energy) first, image for chip km4 (high performance) second, that's why image **km0_km4_image2.bin** is needed  
-	Images start with 8 signature bytes **0x35, 0x39, 0x31, 0x38, 0x31, 0x31, 0x37, 0x38** to communicate a valid image OR anything else for Invalid  
-   If an image with the 'valid' signature fails to load then booting the chip fails completely
-   OTA Updates have to contain OTA update functions to preserve OTA update functionality 
-   First upload over Serial port saves in OTA Address 1; after that addresses have to be altered for every OTA  
-   Hard Fault Error likely means the program tried to overwrite an image that is currently in use; prevented by resetting image signature bytes in case of a failed OTA update
-   Serial uploads write the image at Address 1. If an earlier OTA update was saved at Address 2 this will be booted. Erase Flash and upload again to fix.
-   Sending an Image requires a file header with 3 Words (12 Bytes): Checksum, 0, Image Length (see python code) 
-   All Image Words in little Endian (LSB First), Bytes in big Endian (MSB First)  
-   The checksum is calculated by adding up all bytes of the image, making use of overflows

## Helpful links
---
General BW16 Docs:  
https://amebaiotdocuments.readthedocs.io/en/latest/ambd_arduino/BW16_/index.html  
Uploading Code to BW16:  
https://github.com/mikey60/BW16-RTL8720DN-Module-Arduino  
https://www.amebaiot.com/en/amebad-bw16-arduino-getting-started/  
OTA Explanations:  
https://www.amebaiot.com.cn/en/ameba-arduino-ota/  
https://www.e-paper-display.cn/99IOT/Started%20guide/Ameba-D-Application-Note-v10_215535.pdf  
https://forum.amebaiot.com/t/resources-bw16-troubleshooting-guide/678  
https://github.com/ambiot/ambd_arduino/pull/130  

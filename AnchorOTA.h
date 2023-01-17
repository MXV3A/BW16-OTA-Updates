#ifndef _ANCHOROTA_H_
#define _ANCHOROTA_H_

#include "flash_api.h"

#define OTA_ADDRESS_1 0x6000
#define OTA_ADDRESS_2 0x106000

#define OTA_VALID 0x35393138
#define OTA_VALID2 0x31313738
#define OTA_INVALID 0x35393130 //anything that's not OTA_VALID

#define BUFSIZE 512
#define DEFAULT_OTA_MDNS_BUF 128
#define IMAGE_SECTOR_SIZE 4096
#define DEFAULT_IMAGE_DOWNLOAD_TIMEOUT 30000

class OTAClass {

public:
    OTAClass();

    int beginArduinoMdnsService(char *device_name, uint16_t port);  //Advertising local ip for 1. OTA variant | alternative AmebaMDNS in .ino file | or skip function
    int endArduinoMdnsService();

    int beginLocal(uint16_t port, bool rebootOnSuccess = true); //1. OTA variant -> Chip is Server to receive images
	int beginCloud(uint32_t ipaddress, uint16_t port, bool rebootOnSuccess = true); //2. OTA variant -> Chip is Client to request images

private:
    int receiveImage(int socket, int imageLength);
    int receiveImageHeader(int socket, int& imageLength, int& imageChecksum);
    int calculateImageChecksum(int& calculatedChecksum, int imageLength);
    int eraseImageFlash(int imageLength);
    int choseOtaAddress();
    int createImageHeader();

    //unused
    int outputHeader(uint32_t address);
    int scanFlashMemory(uint32_t endAddress);   //Memory scan to find signal bytes
    uint32_t otaAddrOld;
    uint32_t otaAddrNew;

    flash_t flash;
    unsigned char *mdnsBuffer;
    void *mdnsServiceId;
    void *txtRecord;
};

extern OTAClass OTA;

#endif

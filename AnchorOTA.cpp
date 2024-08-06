#include "Arduino.h"
#include "AnchorOTA.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ard_socket.h"
#include "flash_api.h"
#include "sys_api.h"
#include "mDNS.h"

#ifdef __cplusplus
}
#endif

OTAClass::OTAClass() {
    otaAddrOld = 0xFFFFFFFF;
    otaAddrNew = 0xFFFFFFFF;
    mdnsBuffer = NULL;
    mdnsServiceId = NULL;
    txtRecord = NULL;
}

int OTAClass::beginArduinoMdnsService(char *device_name, uint16_t port) {

    int ret = -1;

    mdnsBuffer = (unsigned char *) malloc (DEFAULT_OTA_MDNS_BUF);
    txtRecord = malloc (sizeof(TXTRecordRef));

    do {
        if (mDNSResponderInit() != 0) {
            printf("Fail to init mDNS service\r\n");
            break;
        }

        TXTRecordCreate((TXTRecordRef *)txtRecord, DEFAULT_OTA_MDNS_BUF, mdnsBuffer);

        TXTRecordSetValue((TXTRecordRef *)txtRecord, "board",       strlen("ameba_rtl8720dn"), "ameba_rtl8720dn");
        TXTRecordSetValue((TXTRecordRef *)txtRecord, "auth_upload", strlen("no"),             "no");
        TXTRecordSetValue((TXTRecordRef *)txtRecord, "tcp_check",   strlen("no"),             "no");
        TXTRecordSetValue((TXTRecordRef *)txtRecord, "ssh_upload",  strlen("no"),             "no");

        //Note: device_name is ignored by ameba library mDNS.h
        mdnsServiceId = mDNSRegisterService(device_name, "_arduino._tcp", "local", port, (TXTRecordRef *)txtRecord);

        TXTRecordDeallocate((TXTRecordRef *)txtRecord);

        ret = 0;
    } while (0);

    if (ret < 0) {
        printf("Fail to begin mDNS service\r\n");
    }

    return ret;
}

int OTAClass::endArduinoMdnsService() {
    if (mdnsServiceId != NULL) {
        mDNSDeregisterService(mdnsServiceId);
    }
    mDNSResponderDeinit();
    if (mdnsBuffer != NULL) {
        free(mdnsBuffer);
        mdnsBuffer = NULL;
    }
    if (txtRecord != NULL) {
        free(txtRecord);
        txtRecord = NULL;
    }
    return 0;
}

int OTAClass::beginCloud(uint32_t ipaddress, uint16_t port, bool rebootOnSuccess) {

    int ret = -1;

    int clientSocket = -1;

    do {
        if(choseOtaAddress() < 0) break;

        clientSocket = start_client(ipaddress, port, 0);
        if (clientSocket < 0) {
            printf("Fail to create socket\r\n");
            break;
        }

        int imageLength, imageChecksum;
        if(receiveImageHeader(clientSocket, imageLength, imageChecksum) < 0) break;

        if(eraseImageFlash(imageLength) < 0) break;

        if(receiveImage(clientSocket, imageLength) < 0)break;

        int calculatedChecksum = 0;
        if(calculateImageChecksum(calculatedChecksum, imageLength) < 0) break;

        if (calculatedChecksum != imageChecksum) {
            printf("Bad checksum: %08X expected: %08X\r\n", (unsigned int)calculatedChecksum, (unsigned int)imageChecksum);
            break;
        }

        if(createImageHeader() < 0) break;

        ret = 0;
        printf("OTA success\r\n");

    } while (0);

    //cleanup
    if (clientSocket >= 0) close_socket(clientSocket);
    if (ret < 0) {
        printf("OTA fail\r\n");
        flash_write_word(&flash, otaAddrNew, OTA_INVALID);
        flash_write_word(&flash, otaAddrOld, OTA_VALID);
    }
    else if(rebootOnSuccess) sys_reset();

    return ret;
}

int OTAClass::beginLocal(uint16_t port, bool rebootOnSuccess) {

    int ret = -1;

    int serverSocket = -1;
    int clientSocket = -1;

    do {
        if(choseOtaAddress() < 0) break;

        serverSocket = start_server(port, 0);
        if (serverSocket < 0) {
            printf("Fail to create socket\r\n");
            break;
        }

        if (sock_listen(serverSocket , 1) < 0) {
            printf("Listen fail\r\n");
            break;
        }

        printf("Wait for client\r\n");
        clientSocket = get_available(serverSocket);
        printf("Client connected.\r\n");

        set_sock_recv_timeout(clientSocket, DEFAULT_IMAGE_DOWNLOAD_TIMEOUT);

        int imageLength, imageChecksum;
        if(receiveImageHeader(clientSocket, imageLength, imageChecksum) < 0) break;

        if(eraseImageFlash(imageLength) < 0) break;

        if(receiveImage(clientSocket, imageLength) < 0)break;

        int calculatedChecksum = 0;
        if(calculateImageChecksum(calculatedChecksum, imageLength) < 0) break;

        if (calculatedChecksum != imageChecksum) {
            printf("Bad checksum: %08X expected: %08X\r\n", (unsigned int)calculatedChecksum, (unsigned int)imageChecksum);
            break;
        }

        if(createImageHeader() < 0) break;

        ret = 0;
        printf("OTA success\r\n");

    } while (0);

    //cleanup
    if (serverSocket >= 0) close_socket(serverSocket);
    if (clientSocket >= 0) close_socket(clientSocket);
    if (ret < 0) {
        printf("OTA fail\r\n");
        flash_write_word(&flash, otaAddrNew, OTA_INVALID);
        flash_write_word(&flash, otaAddrOld, OTA_VALID);
    }
    else if(rebootOnSuccess) sys_reset();

    return ret;
}

int OTAClass::receiveImage(int socket, int imageLength){
    if(socket == -1) return -1;
    unsigned char *buf = (unsigned char *) malloc (BUFSIZE);
        if (buf == NULL) {
            printf("Fail to allocate memory\r\n");
            return -1;
        }
    printf("Start Download.\r\n");
    int processedLength = 0;
    int readLength = 0;
    while( processedLength < imageLength ) {
        memset(buf, 0, BUFSIZE);
        readLength = recv_data(socket, buf, BUFSIZE, 0);

        if (readLength < 0) {
            printf("Socket Error: %d\r\n", get_sock_errno(socket));
            break;
        }

        if(readLength == 0){
            printf("Connection closed by peer\r\n");
            break;
        }

        if (flash_stream_write(&flash, otaAddrNew + processedLength, readLength, buf) < 0) {
            printf("Write sector fail\r\n");
            break;
        }

        processedLength += readLength;
    }

    if (processedLength != imageLength) {
        printf("Download fail\r\n");
        return -1;
    }
    printf("Download successful\r\n");
    free(buf);
    return 0;
}

int OTAClass::receiveImageHeader(int socket, int& imageLength, int& imageChecksum){
    if(socket == -1) return -1;
    uint8_t fileInfo[12];
    if(recv_data(socket, fileInfo, sizeof(fileInfo), 0) < 0){
        printf("Failed to receive Image Header");
        return -1;
    }

    //uint8_t to uint32_t
    imageLength = 0;
    imageChecksum = 0;
    for(int i=0; i<4; i++){
        imageLength = (imageLength << 8) + fileInfo[11-i];
        imageChecksum = (imageChecksum << 8) + fileInfo[3-i];
    }

    printf("Image Length: %d\r\n",imageLength);
    if(imageLength == 0){
        printf("Empty Image or Failed to read Header");
        return -1;
    }
    if(imageLength > OTA_ADDRESS_2 - OTA_ADDRESS_1){
        printf("Image too large\r\n");
        return -1;
    }
    return 0;
}

int OTAClass::choseOtaAddress(){
    uint32_t signature1 = 1, signature2 = 1;
    flash_read_word(&flash, OTA_ADDRESS_1, &signature1);
    flash_read_word(&flash, OTA_ADDRESS_2, &signature2);
    if(signature1 == 1 || signature2 == 1){
        printf("No OTA Address Found; Signature read failed.\r\n");
        return -1;
    }
    else if(signature2 == OTA_VALID){ //Even if both images are valid image 2 gets booted, so we overwrite image 1
        otaAddrNew = OTA_ADDRESS_1;
        otaAddrOld = OTA_ADDRESS_2;
        return 0;
    }
    else if(signature1 == OTA_VALID){ //Image 1 is booted so we overwrite image 2
        otaAddrNew = OTA_ADDRESS_2;
        otaAddrOld = OTA_ADDRESS_1;
        return 0;
    }
    else{ //Both images invalid means we don't know which image is in use; this normally shouldn't happen
        printf("No OTA Address chosen to prevent Hard Fault Error.\r\n");
        printf("Currently used Image couldn't be identified.\r\n");
        return -1;
    }
}

int OTAClass::calculateImageChecksum(int& calculatedChecksum, int imageLength){
    if(otaAddrNew != OTA_ADDRESS_1 && otaAddrNew != OTA_ADDRESS_2) return -1;
    if(imageLength <= 0) return -1;
    unsigned char *buf = (unsigned char *) malloc (BUFSIZE);
    if (buf == NULL) {
        printf("Fail to allocate memory\r\n");
        return -1;
    }
    int processedLength = 0;
    calculatedChecksum = 0;
    while ( processedLength < imageLength ) {
        int readLength = (processedLength + BUFSIZE < imageLength) ? BUFSIZE : (imageLength - processedLength);
        flash_stream_read(&flash, otaAddrNew + processedLength, readLength, buf);
        for (int i=0; i<readLength; i++) calculatedChecksum += (buf[i] & 0xFF);
        processedLength += readLength;
    }
    free(buf);
    return 0;
}

int OTAClass::eraseImageFlash(int imageLength){
    if(otaAddrNew != OTA_ADDRESS_1 && otaAddrNew != OTA_ADDRESS_2) return -1;
    if(imageLength <= 0) return -1;
    int imageSizeInSectors = ((imageLength - 1) / IMAGE_SECTOR_SIZE) + 1;
    for (int i = 0; i < imageSizeInSectors; i++) {
        //void function; if it fails -> Hard Fault Error :/
        flash_erase_sector(&flash, otaAddrNew + i * IMAGE_SECTOR_SIZE);
    }
    return 0;
}

int OTAClass::createImageHeader(){
    uint32_t signature1 = 1, signature2 = 1;
    // Mark image 1 as new image
    flash_write_word(&flash, otaAddrNew + 0, OTA_VALID);
    flash_write_word(&flash, otaAddrNew + 4, OTA_VALID2);
    
    flash_read_word(&flash, otaAddrNew +  0, &signature1);
    flash_read_word(&flash, otaAddrNew + 4, &signature2);
    if (signature1 != OTA_VALID || signature2 != OTA_VALID2) {
        printf("Put signature fail\r\n");
        return -1;
    }

    // Mark image 2 as old image
    flash_write_word(&flash, otaAddrOld + 0, OTA_INVALID);

    return 0;
}

int OTAClass::outputHeader(uint32_t address){
    uint32_t headerWord = 0;
    printf("Header of Image at Address %08X\r\n",(unsigned int)address);
    for(int i=0; i<0x18; i+=0x04){
        flash_read_word(&flash, address + i, &headerWord);
        printf("%08X\r\n", (unsigned int)headerWord);
    }
    printf("\r\n");
    return 0;
}

int OTAClass::scanFlashMemory(uint32_t endAddress) {
    printf("Start Image Scan from 0x0 to %08X\r\n", (unsigned int)endAddress);
    for(int i=0; i<(int)endAddress; i+=0x04){
        uint32_t currentContent = 0;
        flash_read_word(&flash, i, &currentContent);
        //OTA address
        if(currentContent == OTA_ADDRESS_1) printf("Found OTA_ADDRESS_1 in %08X\r\n", i);
        if(currentContent == OTA_ADDRESS_2) printf("Found OTA_ADDRESS_2 in %08X\r\n", i);
        //Suggested signal byte by doc
        if(currentContent == 0x81958711) printf("Found 0x81958711 in %08X\r\n", i);
        //deprecated OTA image start
        if(currentContent == OTA_INVALID) printf("Found OTA_INVALID in %08X\r\n", i);
        //valid OTA image start
        if(currentContent == OTA_VALID) printf("Found OTA_VALID in %08X\r\n", i);
    }
    printf("Finished Image Scan\r\n");

    return 0;
}

OTAClass OTA;


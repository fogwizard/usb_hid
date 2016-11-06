/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009
 
 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"

#define ENUMERATE 1
#define USB_LEN   61

#define SOH  1
#define STX  2
#define EOT  4
#define ACK	 6
#define NAK  21
#define CAN  24
#define SUB  26
//#define SUB 'c'
//#define ACK 'D'
//#define NAK 'E'
//#define CAN 'F'
int write_count = 0;
int file_recsize = 0;
int file_reccount = 0;
FILE *filep;
const char *filename = "file1.txt";
// Headers needed for sleeping.
#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

typedef enum {
fsm_err     = -1,
fsm_cpl     =  0, 
fsm_ongoing =  1,
}fsm_type;

#if 1
		// CRC 字节值表
       //字地址 0 - 255 (只取低8位)
       //位地址 0 - 255 (只取低8位)
       /* CRC 高位字节值表 */
static unsigned char auchCRCHi[] = {
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
            0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
            0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
            0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
            0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
            0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
            0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
            0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
            0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
            0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
        };
       /* CRC低位字节值表*/
static unsigned char  auchCRCLo[] = {
            0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
            0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
            0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
            0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
            0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
            0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
            0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
            0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
            0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
            0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
            0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
            0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
            0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
            0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
            0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
            0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
            0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
            0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
            0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
            0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
            0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
            0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
            0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
            0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
            0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
            0x43, 0x83, 0x41, 0x81, 0x80, 0x40
        };

#endif
static int receive_file_data(hid_device *dev, void * data, unsigned size, const char *name, int fsize);
unsigned short crc16(unsigned char *puchMsg,unsigned short usDataLen);

static fsm_type xmodem_send_char(hid_device *handle, char c,int timeout)
{
	int res = 0;
	int retry = 0;
	static unsigned  char sbuf[65] = {0};

	memset(sbuf,0x00,sizeof(sbuf));
	// Toggle LED (cmd 0x80). The first byte is the report number (0x1).
	sbuf[0] = 0x00;
	sbuf[1] = c;
	retry = 0;
	do {
		res = hid_write(handle, sbuf, USB_LEN);
		if (res < 0) {
			printf("[%d]Error: %ls\n", ++retry, hid_error(handle));
		}
		Sleep(900);
	}while((res < 0) && (retry <10));

}

static fsm_type do_xmodem(hid_device * handle)
{
	int res,rec_count;
	unsigned char i,temp,buf[61];
	unsigned char bufout[61*6];
	static enum {
		STATE_START,
		STATE_SEND_REQUEST,/* C */
		STATE_SEND_REQUEST_WAIT,
		STATE_SEND_ACK,/* D */
		STATE_SEND_ACK_WAIT,
		STATE_SEND_HANDSHAPE,/* C */
		STATE_SEND_HANDSHAPE_WAIT,
		STATE_RECV,/* recv block */
		STATE_RECV_ACK, /* D */
		STATE_CPL,
	}s_state = STATE_START;
	int fsize;
	int retry;
	unsigned char *pBuf = NULL;
	unsigned short crc;
    int r = 0;
    int bytes = 0;

	switch(s_state) {

		case STATE_START:
			s_state = STATE_SEND_REQUEST;
			rec_count = 0;
			break;
		case STATE_SEND_REQUEST:/* C */
			printf("send C \n");
			xmodem_send_char(handle, 'C', -1);
			s_state = STATE_SEND_REQUEST_WAIT;
			break;
		case STATE_SEND_REQUEST_WAIT:
			Sleep(200);
			res = 0;
			s_state = STATE_SEND_ACK;
			break;
		case STATE_SEND_ACK:/* D */  //这里需要读出文件名，并判断文件名是否等于file(1..2..3).bin等。
			res = hid_read(handle, buf, sizeof(buf));
			if(res > 0){
				crc= crc16(buf,(sizeof(buf)-2));
				if((buf[0]==SOH)&&(buf[3] == 'f')&&(buf[59] ==(unsigned char)(crc>>8))&&(buf[60] ==(unsigned char)(crc&0x00ff))){  //判断首字节和文件名是否在文件规则内
					printf("send D \n");
					printf("we have receive the file name and the capacity \n");				
					if((filep = fopen(filename, "wb"))==NULL) {
						printf("The file %s can not be opened.\n",filename);
						xmodem_send_char(handle,NAK, -1);
						s_state = STATE_SEND_REQUEST_WAIT;
						break;
					}
					i = 0;
					rec_count = 0;
					while(buf[i+3] !=SUB){
						bufout[rec_count++] = buf[3+i++];
					}
					bufout[rec_count++] = ' ';
					fwrite(&bufout,rec_count ,1,filep);
					write_count += rec_count;
					fseek(filep,write_count,0);
					i = 0;
					rec_count = 0;
					file_recsize = 0;
					file_reccount = 0;
					while(buf[i+3+write_count] !=SUB){
						temp = (buf[3+i+write_count]/16);
						if(temp <=9){
							bufout[rec_count] = temp + '0';
						}else{
							bufout[rec_count] = temp - 10 + 'A';
						}
						rec_count++;							
						temp = (buf[3+i+write_count]%16);
						if(temp <=9){
							bufout[rec_count] = temp + '0';
						}else{
							bufout[rec_count] = temp - 10 + 'A';
						}
						file_recsize = file_recsize<<8;	//接收数据字节数
						file_recsize += buf[3+i+write_count];
						rec_count++;
						i++;
					}
					printf("%d",file_recsize);printf("\n");
					bufout[rec_count++] = 'h';
					bufout[rec_count++] = ' ';
					fwrite(&bufout,rec_count,1,filep);
					write_count += rec_count;
					//fclose(filep);
					//COPY the filename to pbuf；
					xmodem_send_char(handle,ACK, -1);
					s_state = STATE_SEND_ACK_WAIT;
				}else{//不正确，发送NAK，让下位机重新发送
					printf("filename err \n");
					xmodem_send_char(handle,NAK, -1);
					s_state = STATE_SEND_REQUEST_WAIT;
				}			
			}else{//没接收到数据或者接受到错误的数据，返回重新请求
				printf("send filename but no data \n");
				s_state = STATE_SEND_REQUEST;
			}
			break;
		case STATE_SEND_ACK_WAIT:
			Sleep(200);
			res = 0;
			s_state = STATE_SEND_HANDSHAPE;
			break;
		case STATE_SEND_HANDSHAPE:/* C */ 
			printf("send C \n");
			xmodem_send_char(handle, 'C', -1);
			s_state = STATE_SEND_HANDSHAPE_WAIT;
			break;
		case STATE_SEND_HANDSHAPE_WAIT:
			Sleep(200);
			s_state = STATE_RECV;
			break;
		case STATE_RECV:/* recv block *///接收文件
			Sleep(500);
			res = hid_read(handle, buf, sizeof(buf));
			if(res >0){  //
				crc= crc16(buf,(sizeof(buf)-2));
				if((buf[59] ==(unsigned char)(crc>>8))&&(buf[60] ==(unsigned char)(crc&0x00ff))){
					if(buf[0]==SOH){  //接收文件中
						//COPY filedata to pbuf;
						s_state = STATE_RECV_ACK;
						fseek(filep,write_count,0);
						printf("%d",write_count);printf("\n");
						printf("receiving filedata %d\n",buf[1]);
						i = 0;
						rec_count = 0;
						while(i<56){
							temp = (buf[3+i]/16);
							if(temp <=9){
								bufout[rec_count] = temp + '0';
							}else{
								bufout[rec_count] = temp - 10 + 'A';
							}
							rec_count++;							
							temp = (buf[3+i]%16);
							if(temp <=9){
								bufout[rec_count] = temp + '0';
							}else{
								bufout[rec_count] = temp - 10 + 'A';
							}
							rec_count++;							
							i++;
							file_reccount++;
							if(i%4 == 0){//每次接收四个字节就是一个32位数据
								bufout[rec_count++] = 'h';
								bufout[rec_count++] = ' ';
							}
							if(file_reccount >=file_recsize){
								printf("receive data finish\n");
								break;
							}
						}
						fwrite(&bufout,rec_count,1,filep);
						write_count += rec_count;
						xmodem_send_char(handle, ACK, -1);
					}else if(buf[0]==EOT){ //Xmodem接收文件结束
						s_state = STATE_CPL;
						xmodem_send_char(handle, ACK, -1);
						printf("receive complete signal \n");
					}else{  //数据干扰出错，否则不会到这里
						s_state = STATE_RECV;
						printf("receive error data \n");
					}
				}else{//CRC校验错误
					printf("CRC16 checkout err");
					printf("%d",crc);printf("\n");
					crc = buf[59]*256+buf[60];
					printf("%d",crc);printf("\n");
					xmodem_send_char(handle,NAK, -1);
					s_state = STATE_RECV;
				}
			}else{	//没接收到数据，等到一段时间还没有数据，结束过程
					s_state = STATE_RECV;
					printf("send filedata but no data \n");
				}
			break;
		case STATE_RECV_ACK: /* D */
			printf("send D \n");
			//xmodem_send_char(handle, ACK, -1);
			s_state = STATE_RECV;//DEBUG
			break;
		case STATE_CPL:
			fsize = 128;
			pBuf =(unsigned char *) malloc(fsize);  //debug
			printf("recv file\n");
			fclose(filep);
			while(1);
		//	receive_file_data(handle, pBuf, fsize, "data.dat", fsize);//这个文件有错，没能保存数据
			Sleep(200);
			s_state = STATE_START;
			while(1);
			return fsm_cpl;
	}
	return fsm_ongoing;
}

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[256];
	#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;

#ifdef WIN32
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#endif

	struct hid_device_info *devs, *cur_dev;
	
	if (hid_init())
		return -1;
#if ENUMERATE
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;	
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);
#endif

	// Set up the command buffer.
	memset(buf,0x00,sizeof(buf));
	buf[0] = 0x00;
	buf[1] = 0x81;
	
	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x0483, 0x5750, NULL);
	if (!handle) {
		printf("unable to open device\n");
 		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);
	
	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	res = hid_read(handle, buf, USB_LEN);

#if 0
	// Send a Feature Report to the device
	buf[0] = 0x2;
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = hid_send_feature_report(handle, buf, 64);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf,0,sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}
#endif

	memset(buf,0,sizeof(buf));

	while (1) {
		res = do_xmodem(handle);
		if (res == fsm_err) {
			break;
		}
	}

#if 0
	// Read requested state. hid_read() has been set to be
	// non-blocking by the call to hid_set_nonblocking() above.
	// This loop demonstrates the non-blocking nature of hid_read().
	res = 0;
	while (res == 0) {
		res = hid_read(handle, buf, 65);
		if (res == 0)
			printf("waiting...\n");
		if (res < 0)
			printf("Unable to read()\n");
		#ifdef WIN32
		Sleep(990);
		#else
		usleep(500*1000);
		#endif
	}

	printf("Data read:\n   ");
	// Print out the returned buffer.
	for (i = 0; i < res; i++)
		printf("%02hhx ", buf[i]);
	printf("\n");
#endif

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

static int receive_file_data(hid_device *usb, void * data, unsigned size, const char *name, int fsize)
{
    unsigned char *status = (unsigned char *)data;
    FILE *fp;
    int r = 0;
    int rece_count = 0;
    int bytes = 0;

    if (size == 0 ){
            sprintf(ERROR, "Nothing to read (%d bytes)", r);
    }

    if((fp = fopen(name, "wb"))==NULL) {
        printf("The file %s can not be opened.\n",name);
        return -1;
    }

    fprintf(stderr, "open: file: (%s)\n", name);

    for(;;) {
	if (fsize > 64 ){
        	r = hid_read(usb, status, 64);
	} else if (fsize > 0) {
        	r = hid_read(usb, status, fsize);
	} else {/* fsize == 0 */
		goto eof;
	}
        if(r < 0) {
            sprintf(ERROR, "status read failed (%s)", strerror(errno));
            break;
	}
        status[r] = 0;
	fsize -= r;
	bytes += r;
        fprintf(stderr, "write:(%d)", r);
    	if ((++rece_count %4) == 0)
        	fprintf(stderr, "\n");

	fwrite(status, r , 1 ,fp);
	/* if fsize <= 0, that is to say, end of file */
        if(fsize <= 0) {
eof:        fprintf(stderr, "\nINFO: End Of File Found:(last pack:%d bytes)\n", r);
            fprintf(stderr, "\nINFO: Total: %dbytes, %dkb, %dMb\n", bytes, (bytes>>10), (bytes>>20));
	    /* read fastboot OKay from devices*/
            r = hid_read(usb, status, 4);
            status[r] = 0;
            fprintf(stderr, "INFO: last recv=%d\n", r);
	    hid_close(usb);
            fclose(fp);
            return 0;
        }
    }

    hid_close(usb);
    fclose(fp);

    return -1;
}

unsigned short crc16(unsigned char *puchMsg,unsigned short usDataLen) 
{ 
	unsigned char uchCRCHi = 0xFF ; /* 高CRC字节初始化 */ 
	unsigned char uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */ 
	unsigned char uIndex ; /* CRC循环中的索引 */ 
	while (usDataLen--) /* 传输消息缓冲区 */ 
	{ 
		uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */ 
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ; 
		uchCRCLo = auchCRCLo[uIndex] ; 
	} 
	return (uchCRCHi << 8 | uchCRCLo) ; 
}










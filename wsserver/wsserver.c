#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include "wsserver.h"


const char http_header[] = "GET / HTTP/1.1";
const char ws_key[] = "Sec-WebSocket-Key";
const char magic_key[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const char response_end[] = "\r\n\r\n";
const int kClientKeyLen = 24;

const unsigned int kPayLoadBound1 = 125;
const unsigned int kPayLoadBound2 = 65535;


int wsaccept(int sock, char* recv_data){
	char *p;
	int i;

	char key_buf[60] = {0};
	unsigned char wsclient_key[60];
	unsigned char sha1_hash[20];
	char hash_buf[20];
	char *b64_buf;
	char response[1024] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

	for(i=0; i<strlen(http_header); i++){
		if(http_header[i] != recv_data[i]) return -1;
	}

	if(p = strstr(recv_data, ws_key)){
		strncat(key_buf, p+19, kClientKeyLen);
		strncat(key_buf, magic_key, sizeof(magic_key));
		for(i=0; i<sizeof(key_buf); i++){
			wsclient_key[i] = key_buf[i];
		}

		SHA1(wsclient_key, sizeof(wsclient_key), sha1_hash);
		base64Encode(sha1_hash, sizeof(sha1_hash), &b64_buf);
		strncat(response, b64_buf, sizeof(b64_buf)+20);
		strncat(response, response_end, sizeof(response_end));

		if(send(sock, response, strlen(response), 0) < 0){
			return -1;
		}
	}

	return 0;
}

int wssend(int sock, const char* send_data, int length){
	int frame_len = 0;
	int i, result;

	if(length <= kPayLoadBound1){
		frame_len = length + 2;
		char frame[frame_len];
		frame[0] = 0x81;
		frame[1] = length;

		for(i=0; i<length + 1; i++){
			frame[i+2] = send_data[i];
		}
		if(send(sock, frame, frame_len, 0) < 0){
			result = -1;
		} else{
			result = length;
		}

	} else if(length <= kPayLoadBound2){
		// if send data length is over 125 bytes
		// stuff your framing codes..
	} else{
		// if send data lenght is over 65535 bytes
		// stuff your framing codes..
	}

	return result;
}

//const int tBuf_size = 20;
const int _recv_buf_size = 131;
const int _recv_buf1k_size = 1008;
const int _recv_buf2k_size = 2008;
const int _recv_buf5k_size = 5008;
//unsigned char tBuf[tBuf_size];
unsigned char _recv_buf[_recv_buf_size];
unsigned char _recv_buf1k[_recv_buf1k_size];
unsigned char _recv_buf2k[_recv_buf2k_size];
unsigned char _recv_buf5k[_recv_buf5k_size];
// ...

int header_flag = 0;
int next_buf_val = 0; // 0 is less than 126, 1 is less than 65536, 2 is more than
int recv_len;
unsigned char key_idx = 0;
unsigned char mask_key[4];


int wsrecv(int sock, char* recv_buf, int length){
	int i;
	if(header_flag == 0){
		unsigned char temp[2] = {0};
		int m1, t1 = 0;
		while(t1 < 2){
			if((m1 =  recv(sock, temp+t1, 2-t1, 0)) < 0) return -1;
			t1 += m1;
		}

		if(temp[1] < 254){
			recv_len = temp[1] - 128;
			unsigned char temp2[4];

			int m2, t2 = 0;
			while(t2 < 4){
				if((m2 = recv(sock, temp2+t2, 4-t2, 0)) < 0) return -1;
				t2 += m2;
			}

			mask_key[0] = temp2[0];
			mask_key[1] = temp2[1];
			mask_key[2] = temp2[2];
			mask_key[3] = temp2[3];
		} else if(temp[1] == 254){
			unsigned char temp2[6];
			if(recv(sock, temp2, 6, 0) < 0) return -1;
			recv_len = recv_len | (temp2[0] << 8);
			recv_len = recv_len | temp2[1];
			mask_key[0] = temp2[2];
			mask_key[1] = temp2[3];
			mask_key[2] = temp2[4];
			mask_key[3] = temp2[5];
		} else if(temp[1] == 255){
			unsigned char temp2[12];
			if(recv(sock, temp2, 12, 0) < 0) return -1;
			unsigned char big_recv_len[8];
			big_recv_len[0] = temp2[0];
			big_recv_len[1] = temp2[1];
			big_recv_len[2] = temp2[2];
			big_recv_len[3] = temp2[3];
			big_recv_len[4] = temp2[4];
			big_recv_len[5] = temp2[5];
			big_recv_len[6] = temp2[6];
			big_recv_len[7] = temp2[7];
			mask_key[0] = temp2[8];
			mask_key[1] = temp2[9];
			mask_key[2] = temp2[10];
			mask_key[3] = temp2[11];
		}
		header_flag = 1;
	}
	
//	unsigned char rBuf[length];
	unsigned char rBuf[recv_len];

	memset(rBuf, 0, sizeof(recv_len));
	int result = 0;
	if(recv_len > 0){
		int t3 = 0;

		while(t3 < recv_len){
			if((result = recv(sock, rBuf+t3, recv_len-t3, 0)) < 0) return -1;
			t3 += result;
		}

		if(result <= 0) return result;
		for(i=0; i<recv_len; i++){
			if(i >= length) break;
			recv_buf[i] = rBuf[i] ^ mask_key[key_idx++%4];
		}

		recv_len -= result;
	}

	if(recv_len <= 0){
		header_flag = 0;
		key_idx = 0;
	}

	return result;


}

int base64Encode(const unsigned char* buffer, size_t length, char** b64text) { 
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;

	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	BIO_write(bio, buffer, length);
	BIO_flush(bio);
	BIO_get_mem_ptr(bio, &bufferPtr);
	BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free_all(bio);

	*b64text=(*bufferPtr).data;

	return (0);
}

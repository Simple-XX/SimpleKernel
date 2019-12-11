
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// string.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"

// 获取字符串长度
size_t strlen(const char* str){
	size_t len = 0;
	while(str[len])
		len++;
	return len;
}

// 如果 src > dest, 则返回值大于 0，如果 src = dest, 则返回值等于 0，
// 如果 srd  < dest ,则返回值小于 0。
int8_t strcmp(const char * src, const char * dest){
	while ( *src && *dest && (*src == *dest) ) {
		src++;
		dest++;
	}
	return *src - *dest;
}

char * strcpy(char * dest, const char * src){
	char * address = dest;
	while( (*dest++ = *src++) != '\0' );
	return address;
}

void backspace(char * src){
	size_t len = strlen(src);
	src[len - 1] = '\0';
}

void append(char * src, char dest){
	size_t len = strlen(src);
	src[len] = dest;
	src[len + 1] = '\0';
}

char * strcat(char * dest, const char * src){
	uint8_t * add_d = (uint8_t *)dest;
	if(dest != NULL && src != NULL) {
		while(*add_d)
			add_d++;
		while(*src)
			*add_d++ = *src++;
	}
	// size_t len = strlen(dest);
	// dest[len+1]='\0';
	return dest;
}

void memcpy(void * dest, void * src, uint32_t len){
	uint8_t * sr = (uint8_t *)src;
	uint8_t * dst = (uint8_t *)dest;
	while (len != 0) {
		*dst++ = *sr++;
		len--;
	}
}
void memset(void * dest, uint8_t val, uint32_t len){
	for (uint8_t * dst = (uint8_t *)dest; len != 0; len--) {
		*dst++ = val;
	}
}

void bzero(void * dest, uint32_t len) {
	memset(dest, 0, len);
}

#ifdef __cplusplus
}
#endif

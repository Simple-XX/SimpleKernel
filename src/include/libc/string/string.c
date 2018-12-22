
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// string.c for MRNIU/SimpleKernel.


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
		while (*src && *dest && (*src == *dest)) {
				src++;
				dest++;
		}
		return *src - *dest;
}

char *strcpy(char *dest, const char *src){
		char *address = dest;
		while((*dest++ = *src++) != '\0');
		return address;
}

void backspace(char *src){
		int32_t len = strlen(src);
		src[len-1] = '\0';
}

void append(char *src, char dest){
		int len = strlen(src);
		src[len] = dest;
		src[len+1] = '\0';
}

char *strcat(char *dest, const char *src){
		int8_t * add_d= (int8_t *)dest;
		if(dest!=NULL && src !=NULL) {
				while(*add_d)
						add_d++;
				while(*src)
						*add_d++ = *src++;
		}
		return dest;
}


// /* Convert the integer D to a string and save the string in BUF. If
//    BASE is equal to 'd', interpret that D is decimal, and if BASE is
//    equal to 'x', interpret that D is hexadecimal. */
// static void
// itoa (char *buf, int base, int d)
// {
//   char *p = buf;
//   char *p1, *p2;
//   unsigned long ud = d;
//   int divisor = 10;
//
//   /* If %d is specified and D is minus, put `-' in the head. */
//   if (base == 'd' && d < 0)
//   {
//     *p++ = '-';
//     buf++;
//     ud = -d;
//   }
//   else if (base == 'x')
//     divisor = 16;
//
//   /* Divide UD by DIVISOR until UD == 0. */
//   do
//   {
//     int remainder = ud % divisor;
//
//     *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
//   }
//   while (ud /= divisor);
//
//   /* Terminate BUF. */
//   *p = 0;
//
//   /* Reverse BUF. */
//   p1 = buf;
//   p2 = p - 1;
//   while (p1 < p2)
//   {
//     char tmp = *p1;
//     *p1 = *p2;
//     *p2 = tmp;
//     p1++;
//     p2--;
//   }
// }

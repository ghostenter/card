#pragma once
#include "stdafx.h"
#define UCHAR unsigned char

typedef unsigned long err_Type;
typedef unsigned char uchar;
typedef unsigned long sysobj_LType;	


//Ñ¹Ëõ×Ö·û´®
template <class T> void StrToEBCD(T* Buf_IN, T* Buf_OUT, long len_Buf_IN)
{
	T temp1,temp2;
	long Len = len_Buf_IN/2, i;

	for (i=0; i<Len; i++)
	{	
		temp1 = Buf_IN[i*2];
		if (temp1 >= 'a')
			temp1 = temp1 - 'a' + 10;
		if (temp1 >= 'A')
			temp1 = temp1 - 'A' + 10;			
		if (temp1 >= '0')
			temp1 = temp1-'0';

		temp2 = Buf_IN[i*2 + 1];
		if (temp2 >= 'a')
			temp2 = temp2 - 'a' + 10;
		if (temp2 >= 'A')
			temp2 = temp2 - 'A' + 10;			
		if (temp2 >= '0')
			temp2 = temp2-'0';
		Buf_OUT[i] = ((temp1&0x0f)<<4)|(temp2&0x0f);
	}
}


//½âÑ¹Ëõ×Ö·û´®
template <class T> void EBCDToStr(T *Buf_IN, T *Buf_OUT, long len_Buf_IN)
{
	T temp1,temp2;
	long Len = len_Buf_IN,i;

	for (i=0; i<Len; i++)
	{	
		temp1 = (Buf_IN[i]&0xF0)>>4;
		if (temp1>9)
			temp1 = temp1 + 'A' - 10;
		else
			temp1 = temp1+'0';

		temp2 = Buf_IN[i]&0x0F;
		if (temp2>9)
			temp2 = temp2 + 'A' - 10;			
		else
			temp2 = temp2+'0';

		Buf_OUT[2*i] = temp1;
		Buf_OUT[2*i+1] = temp2;
	}
}

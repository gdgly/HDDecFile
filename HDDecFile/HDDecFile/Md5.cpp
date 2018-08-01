#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <Winbase.h>
#include <time.h>
#include "md5.h"

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
#define ONCE_READ_BLOCK   8192

static VOID
MD5Transform(
    ULONG		 *state,
    UCHAR        *block
	);

static VOID
Encode (
    UCHAR	 *output,
    ULONG    *input,
    ULONG    len
	);
	
static VOID
Decode (
    ULONG		 *output,
    UCHAR        *input,
    ULONG        len
	);

static VOID
MD5_memcpy (
    UCHAR           *output,
    UCHAR           *input,
    ULONG           len
	);
	
static VOID
MD5_memset (
    UCHAR           *output,
    ULONG           value,
    ULONG		    len
	);

static unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (ULONG)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (ULONG)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (ULONG)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (ULONG)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }


//对pContext结构进行初始化
static  VOID
MD5Init(
	 MD5_CTX	*pContext
	)
{
    pContext->count[0] = pContext->count[1] = 0;
    /*
     * Load magic initialization constants.
     */
    pContext->state[0] = 0x67452301;
    pContext->state[1] = 0xefcdab89;
    pContext->state[2] = 0x98badcfe;
    pContext->state[3] = 0x10325476;
}


/* MD5 block update operation. Continues an MD5 message-digest
   operation, processing another message block, and updating the
   context.
   */
static  VOID
MD5Update(
	 MD5_CTX  *pContext, 
	 UCHAR    *pInputData, 
	 ULONG    InputLen 
	)
{
    ULONG		i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (ULONG) ((pContext->count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((pContext->count[0] += ((ULONG) InputLen << 3))
	< ((ULONG) InputLen << 3))
	pContext->count[1]++;
    pContext->count[1] += ((ULONG) InputLen >> 29);

    partLen = 64 - index;

    /*
     * Transform as many times as possible.
     */
    if (InputLen >= partLen) 
	{
		MD5_memcpy (
			(UCHAR *)&pContext->buffer[index],
			pInputData,
			partLen);
 		MD5Transform(pContext->state, pContext->buffer);

		for (i = partLen; i + 63 < InputLen; i += 64)
			MD5Transform(pContext->state, &pInputData[i]);

		index = 0;
    }
    else
		i = 0;

    /* Buffer remaining input */
    MD5_memcpy(
		(UCHAR *)&pContext->buffer[index], 
		(UCHAR *)&pInputData[i],
		InputLen - i);
}


/*************************************************************************** 
   完成MD5数字签名操作 ，将签名写到pDigest里面
   重新初始化context 
****************************************************************************/
static  VOID
MD5Final(
	 UCHAR		*pDigest, 
	  MD5_CTX		*pContext
	)
{
    unsigned char   bits[8];
    unsigned int    index, padLen;

    /* Save number of bits */
    Encode(bits, pContext->count, 8);

    /*
     * Pad out to 56 mod 64.
     */
    index = (unsigned int) ((pContext->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(pContext, PADDING, padLen);

    /* Append length (before padding) */
    MD5Update(pContext, bits, 8);

    /* Store state in digest */
    Encode(pDigest, pContext->state, 16);

    /*
     * Zeroize sensitive information.
     */
    MD5_memset((UCHAR *) pContext, 0, sizeof(*pContext));
}


// MD5 基本转移函数. 按块转移
static VOID
MD5Transform (
    ULONG		 *state,
    UCHAR        *block
	)
{
    ULONG           a = state[0], b = state[1], c = state[2], d = state[3],
                    x[16];

    Decode(x, block, 64);

    /* Round 1 */
    FF(a, b, c, d, x[0], S11, 0xd76aa478);	/* 1 */
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);	/* 2 */
    FF(c, d, a, b, x[2], S13, 0x242070db);	/* 3 */
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);	/* 4 */
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);	/* 5 */
    FF(d, a, b, c, x[5], S12, 0x4787c62a);	/* 6 */
    FF(c, d, a, b, x[6], S13, 0xa8304613);	/* 7 */
    FF(b, c, d, a, x[7], S14, 0xfd469501);	/* 8 */
    FF(a, b, c, d, x[8], S11, 0x698098d8);	/* 9 */
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);	/* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);	/* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be);	/* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122);	/* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193);	/* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e);	/* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821);	/* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[1], S21, 0xf61e2562);	/* 17 */
    GG(d, a, b, c, x[6], S22, 0xc040b340);	/* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51);	/* 19 */
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);	/* 20 */
    GG(a, b, c, d, x[5], S21, 0xd62f105d);	/* 21 */
    GG(d, a, b, c, x[10], S22, 0x2441453);	/* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);	/* 23 */
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);	/* 24 */
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);	/* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6);	/* 26 */
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);	/* 27 */
    GG(b, c, d, a, x[8], S24, 0x455a14ed);	/* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);	/* 29 */
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);	/* 30 */
    GG(c, d, a, b, x[7], S23, 0x676f02d9);	/* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);	/* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[5], S31, 0xfffa3942);	/* 33 */
    HH(d, a, b, c, x[8], S32, 0x8771f681);	/* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);	/* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c);	/* 36 */
    HH(a, b, c, d, x[1], S31, 0xa4beea44);	/* 37 */
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);	/* 38 */
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);	/* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);	/* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);	/* 41 */
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);	/* 42 */
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);	/* 43 */
    HH(b, c, d, a, x[6], S34, 0x4881d05);	/* 44 */
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);	/* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);	/* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);	/* 47 */
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);	/* 48 */

    /* Round 4 */
    II(a, b, c, d, x[0], S41, 0xf4292244);	/* 49 */
    II(d, a, b, c, x[7], S42, 0x432aff97);	/* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7);	/* 51 */
    II(b, c, d, a, x[5], S44, 0xfc93a039);	/* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3);	/* 53 */
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);	/* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d);	/* 55 */
    II(b, c, d, a, x[1], S44, 0x85845dd1);	/* 56 */
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);	/* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);	/* 58 */
    II(c, d, a, b, x[6], S43, 0xa3014314);	/* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1);	/* 60 */
    II(a, b, c, d, x[4], S41, 0xf7537e82);	/* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235);	/* 62 */
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);	/* 63 */
    II(b, c, d, a, x[9], S44, 0xeb86d391);	/* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /*
     * Zeroize sensitive information.
     */
    MD5_memset((UCHAR *)x, 0, sizeof(x));
}


//将输入(input)编码为输出(output)，len为4的整数倍 
static VOID
Encode (
    UCHAR	 *output,
    ULONG    *input,
    ULONG    len
	)
{
    ULONG    i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
	output[j] = (unsigned char) (input[i] & 0xff);
	output[j + 1] = (unsigned char) ((input[i] >> 8) & 0xff);
	output[j + 2] = (unsigned char) ((input[i] >> 16) & 0xff);
	output[j + 3] = (unsigned char) ((input[i] >> 24) & 0xff);
    }
}


//将输入(input)解码为输出(output)，len为4的整数倍 
static VOID
Decode (
    ULONG		 *output,
    UCHAR        *input,
    ULONG        len
	)
{
    ULONG    i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
	output[i] = ((ULONG) input[j]) | (((ULONG) input[j + 1]) << 8) |
	    (((ULONG) input[j + 2]) << 16) | (((ULONG) input[j + 3]) << 24);
}


//内存拷贝
static VOID
MD5_memcpy (
    UCHAR           *output,
    UCHAR           *input,
    ULONG           len
	)
{
    ULONG    i;
    for (i = 0; i < len; i++)
	output[i] = input[i];
}


// 内存预置
static VOID
MD5_memset (
    UCHAR           *output,
    ULONG           value,
    ULONG		    len
	)
{
    ULONG    i;

    for (i = 0; i < len; i++)
	((char *) output)[i] = (char) value;
}


//数据摘要模块初始化
short Cisi_DigestFile(char *FileName,char *Digest)
{
	return Hztc_DigestFile(FileName,Digest);
}

int Hztc_DigestFile(char *FileName,char *Digest)
{
	
	static MD5_CTX DigestContext;
	int FileLen=0,loop=0,tailLen=0;
	char m_digest[16];
	DWORD NumberOfBytesRead;
	char tempBuf[ONCE_READ_BLOCK];
	int indx = 0;
	HANDLE fpSrc;
	fpSrc = CreateFile 
	(FileName,GENERIC_READ,FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
	
	
	 if(fpSrc==INVALID_HANDLE_VALUE)
	{
		
		return(-1);
	}
	
	//读出文件、一次读出4K
	MD5Init(&DigestContext);  //初始化
	
	FileLen = GetFileSize(
 			 fpSrc,  // handle of file to get size of
 			 NULL 
                 			// pointer to high-order word for file size
			);
	
	
	SetFilePointer( fpSrc, 0,NULL,FILE_BEGIN );
	loop = FileLen / ONCE_READ_BLOCK;
	tailLen = FileLen % ONCE_READ_BLOCK;
	for(indx=0;indx<loop;indx++)
	{
		
		if(!ReadFile( fpSrc,tempBuf, ONCE_READ_BLOCK,&NumberOfBytesRead,NULL ) || NumberOfBytesRead != ONCE_READ_BLOCK)
		{
		  	
			CloseHandle(fpSrc); 
			
			return(-4);
		}
		
		
		MD5Update(&DigestContext,(UCHAR *)tempBuf,ONCE_READ_BLOCK);
	}
	if(tailLen)
	{
		if(!ReadFile( fpSrc,tempBuf, tailLen,&NumberOfBytesRead,NULL ) || NumberOfBytesRead != tailLen)
		{
		  	
			CloseHandle(fpSrc); 
			
			return(-4);
		}
		
		MD5Update(&DigestContext,(UCHAR *)tempBuf,tailLen);
	}
	
	MD5Final((UCHAR *)m_digest,&DigestContext);
	memcpy(Digest,m_digest,16);
	CloseHandle(fpSrc); 
	return 0;
}


//数据摘要模块初始化
int Hztc_DigestBuffer(char *Buffer,int BuffLen,char *Digest)
{
	static MD5_CTX DigestContext;
	int FileLen=0,loop=0,tailLen=0;
	char m_digest[16]={0x00};
	int indx = 0;
	MD5Init(&DigestContext);  //初始化
	//移到文件开头位置
	///获取文件长度 
	FileLen = BuffLen;	
	loop = FileLen / ONCE_READ_BLOCK;
	tailLen = FileLen % ONCE_READ_BLOCK;
	for(indx=0;indx<loop;indx++)
	{
		MD5Update(&DigestContext,(UCHAR *)Buffer+indx*ONCE_READ_BLOCK,ONCE_READ_BLOCK);
	}
	if(tailLen)
	{
		
		
		MD5Update(&DigestContext,(UCHAR *)Buffer+loop*ONCE_READ_BLOCK,tailLen);
	}
	

	MD5Final((UCHAR *)m_digest,&DigestContext);
	memcpy(Digest,m_digest,16);
	return 0;
}

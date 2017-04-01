#ifndef _DATATYPE_H
#define _DATATYPE_H

//基本数据类型定义
typedef char TChar;
typedef unsigned char TUChar;

typedef signed short TInt2;
typedef unsigned short TUInt2;

typedef signed int TInt;
typedef unsigned int TUInt;

typedef signed long TInt4;
typedef unsigned long TUInt4;

#ifdef _SYS_WINDOWS

typedef __int64 TInt8;
typedef unsigned __int64 TUInt8;

#else

typedef signed long long TInt8;
typedef unsigned long long TUInt8;

#endif

typedef float TFloat;
typedef double TDouble;

#endif


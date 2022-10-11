#ifndef _QAQ_HEAD_H_
#define _QAQ_HEAD_H_
#include "QAQ_enum.h"
#include <cstring>
#include <string>

#define atoInt atoi
#define atoDouble atof
#define atoChar atoc

#define _MyAssert(_Expression,_ExceptionType,_ExceptionMsg) \
	(void)((!!(_Expression))|| \
    (printf("�ļ�����%s\n",__FILE__)),\
    printf("�кţ�%d\n",__LINE__),\
    printf("��������%s\n",__FUNCTION__),\
    exit(1))
//	throw err##_ExceptionType##Exception(_ExceptionMsg)
//	throw err##_ExceptionType##Exception(#_ExceptionMsg)

#define MyAssert(_Expression,_ExceptionType,_ExceptionMsg) _MyAssert(_Expression,_ExceptionType,_ExceptionMsg)


std::string atoc(const char* _String);
#endif

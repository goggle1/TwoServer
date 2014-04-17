#ifndef __STRPTRLEN_H__
#define __STRPTRLEN_H__

#include <sys/types.h>

class StrPtrLen
{
public:
	StrPtrLen();
	StrPtrLen(char* str);
	StrPtrLen(char* str, int len);
	virtual ~StrPtrLen();
	void 	Set(char* inPtr, int inLen);
	bool 	Equal(const StrPtrLen &compare) const;
	bool 	Equal(const char* compare) const;
	bool 	EqualIgnoreCase(const char* compare, const int len) const;
    bool 	EqualIgnoreCase(const StrPtrLen &compare) const { return EqualIgnoreCase(compare.Ptr, compare.Len); }
	// convert to a "NEW'd" zero terminated char array
    char*   GetAsCString() const;

	char* 	Ptr;
	int   	Len;

protected:
	static u_int8_t    sCaseInsensitiveMask[];
    static u_int8_t    sNonPrintChars[];
	
};

#endif
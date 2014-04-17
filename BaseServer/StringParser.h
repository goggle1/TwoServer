#ifndef __STRINGPARSER_H__
#define __STRINGPARSER_H__

#include <sys/types.h>
#include <string.h>

#include "StrPtrLen.h"

// EOL: end of line
class StringParser
{
public:
	StringParser(StrPtrLen* inStream);
	virtual ~StringParser();

	bool 	ParserIsEmpty(StrPtrLen* outString);
	void 	AdvanceMark();
	void 	ConsumeUntil(StrPtrLen* outString, char inStop);
	void 	ConsumeUntil(StrPtrLen* outString, u_int8_t *inMask);
	bool  	Expect(char stopChar);
	bool 	ExpectEOL();
	bool 	GetThru(StrPtrLen* outString, char inStopChar);
	bool	GetThruEOL(StrPtrLen* outString);	
	void    ConsumeWord(StrPtrLen* outString = NULL);  
	void    ConsumeWhitespace();         
	void	ConsumeLength(StrPtrLen* spl, int32_t numBytes);		
	char*   GetCurrentPosition() { return m_StartGet; }
	int     GetDataParsedLen();
	int		GetDataReceivedLen()
            {  return (int)(m_EndGet - m_Stream.Ptr); }
    inline char     PeekFast() { if (m_StartGet) return *m_StartGet; else return '\0'; }
    	
public:
	static u_int8_t 	sEOLMask[];	
	static u_int8_t 	sNonWordMask[];
	static u_int8_t 	sWhitespaceMask[]; // skip over whitespace	
protected:
	char*       m_StartGet;
    char*       m_EndGet;    
    StrPtrLen   m_Stream;	
};

#endif

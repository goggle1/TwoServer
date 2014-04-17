
#include <string.h>

#include "StringParser.h"

u_int8_t StringParser::sEOLMask[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9   
    1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //30-39 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69  
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
    0, 0, 0, 0, 0, 0             //250-255
};

u_int8_t StringParser::sNonWordMask[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //0-9 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //10-19 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //30-39 
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, //40-49 - is a word
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //50-59
    1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //60-69 //stop on every character except a letter
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
    0, 1, 1, 1, 1, 0, 1, 0, 0, 0, //90-99 _ is a word
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, //120-129
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //130-139
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //140-149
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //150-159
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //160-169
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //170-179
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
    1, 1, 1, 1, 1, 1             //250-255
};

u_int8_t StringParser::sWhitespaceMask[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, //0-9      // stop on '\t'
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, //10-19    // '\r', \v', '\f' & '\n'
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //20-29
    1, 1, 0, 1, 1, 1, 1, 1, 1, 1, //30-39   //  ' '
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //40-49
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //50-59
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //60-69
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //70-79
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //80-89
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //90-99
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //100-109
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //110-119
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //120-129
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //130-139
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //140-149
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //150-159
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //160-169
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //170-179
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //180-189
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //190-199
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //200-209
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //210-219
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //220-229
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //230-239
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //240-249
    1, 1, 1, 1, 1, 1             //250-255
};

StringParser::StringParser(StrPtrLen * inStream)
{
	m_StartGet 		= inStream->Ptr;
	m_EndGet 		= inStream->Ptr + inStream->Len;
	m_Stream.Ptr 	= inStream->Ptr;
	m_Stream.Len 	= inStream->Len;
}

StringParser::~StringParser()
{
}

bool StringParser::ParserIsEmpty(StrPtrLen* outString)
{
    if (NULL == m_StartGet || NULL == m_EndGet)
    {
        if (NULL != outString)
        {   outString->Ptr = NULL;
            outString->Len = 0;
        }
        
        return true;
    }
    
    return false; // parser ok to parse
}

void StringParser::AdvanceMark()
{
     if (this->ParserIsEmpty(NULL))
        return;

   if ((*m_StartGet == '\n') || ((*m_StartGet == '\r') && (m_StartGet[1] != '\n')))
    {
        // we are progressing beyond a line boundary (don't count \r\n twice)
        // fCurLineNumber++;
    }
    m_StartGet++;
}

void StringParser::ConsumeUntil(StrPtrLen* outString, char inStop)
{
    if (this->ParserIsEmpty(outString))
    {
        return;
    }

    char *originalStartGet = m_StartGet;

    while ((m_StartGet < m_EndGet) && (*m_StartGet != inStop))
    {
        AdvanceMark();
    }
        
    if (outString != NULL)
    {
        outString->Ptr = originalStartGet;
        outString->Len = m_StartGet - originalStartGet;
    }
}

void StringParser::ConsumeUntil(StrPtrLen* outString, u_int8_t *inMask)
{
    if (this->ParserIsEmpty(outString))
    {
        return;
    }
        
    char *originalStartGet = m_StartGet;

    while ((m_StartGet < m_EndGet) && (!inMask[(unsigned char) (*m_StartGet)]))//make sure inMask is indexed with an unsigned char
    {
        AdvanceMark();
    }

    if (outString != NULL)
    {
        outString->Ptr = originalStartGet;
        outString->Len = m_StartGet - originalStartGet;
    }
}

bool StringParser::ExpectEOL()
{
    if (this->ParserIsEmpty(NULL))
    {
        return false;
    }

    //This function processes all legal forms of HTTP / RTSP eols.
    //They are: \r (alone), \n (alone), \r\n
    bool retVal = false;
    if ((m_StartGet < m_EndGet) && ((*m_StartGet == '\r') || (*m_StartGet == '\n')))
    {
        retVal = true;
        AdvanceMark();
        //check for a \r\n, which is the most common EOL sequence.
        if ((m_StartGet < m_EndGet) && ((*(m_StartGet - 1) == '\r') && (*m_StartGet == '\n')))
            AdvanceMark();
    }
    return retVal;
}

bool StringParser::GetThruEOL(StrPtrLen* outString)
{
    ConsumeUntil(outString, sEOLMask);
    return ExpectEOL();
}


int StringParser::GetDataParsedLen()
{
	return (int)(m_StartGet - m_Stream.Ptr); 
}

void StringParser::ConsumeWord(StrPtrLen* outString)
{ 
	ConsumeUntil(outString, sNonWordMask); 
}

void StringParser::ConsumeWhitespace()
{ 
	ConsumeUntil(NULL, sWhitespaceMask); 
}


void StringParser::ConsumeLength(StrPtrLen* spl, int32_t inLength)
{
    if (this->ParserIsEmpty(spl))
        return;

    //sanity check to make sure we aren't being told to run off the end of the
    //buffer
    if ((m_EndGet - m_StartGet) < inLength)
        inLength = m_EndGet - m_StartGet;
    
    if (spl != NULL)
    {
        spl->Ptr = m_StartGet;
        spl->Len = inLength;
    }
    if (inLength > 0)
    {
        for (short i=0; i<inLength; i++)
            AdvanceMark();
    }
    else
        m_StartGet += inLength;  // ***may mess up line number if we back up too much
}


bool  StringParser::Expect(char stopChar)
{
    if (this->ParserIsEmpty(NULL))
        return false;

    if (m_StartGet >= m_EndGet)
        return false;
    if(*m_StartGet != stopChar)
        return false;
    else
    {
        AdvanceMark();
        return true;
    }
}


bool StringParser::GetThru(StrPtrLen* outString, char inStopChar)
{
    ConsumeUntil(outString, inStopChar);
    return Expect(inStopChar);
}



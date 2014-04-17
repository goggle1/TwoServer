
//#include "StringTranslator.h"
#include "HttpProtocol.h"
#include "HttpRequest.h"

static StrPtrLen sCloseString("close", 5);
static StrPtrLen sKeepAliveString("keep-alive", 10);

u_int8_t HttpRequest::sURLStopConditions[] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9      //'\t' is a stop condition
  1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39    //' '
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
  0, 0, 0, 0, 0, 0                         //250-255
};

HttpRequest::HttpRequest()
{
	fRequestPath = NULL;
}

HttpRequest::~HttpRequest()
{
	if(fRequestPath != NULL)
    {
    	delete [] fRequestPath;
    	fRequestPath = NULL;
    }
}

void HttpRequest::SetKeepAlive(StrPtrLen *keepAliveValue)
{
    if ( sCloseString.EqualIgnoreCase(keepAliveValue->Ptr, keepAliveValue->Len) )
    {
    	fRequestKeepAlive = false;
    }
    else
    {        
        fRequestKeepAlive = true;
    }
}

// Parses the Connection header and makes sure that request is properly terminated
int HttpRequest::ParseHeaders(StringParser* parser)
{
    StrPtrLen theKeyWord;
    bool isStreamOK;
  
    //Repeat until we get a \r\n\r\n, which signals the end of the headers
    while ((parser->PeekFast() != '\r') && (parser->PeekFast() != '\n'))
    {
        //First get the header identifier
    
        isStreamOK = parser->GetThru(&theKeyWord, ':');
        if (!isStreamOK)
        {       // No colon after header!
            fStatusCode = httpBadRequest;
            return -1;
        }
    
        if (parser->PeekFast() == ' ') 
        {        // handle space, if any
            isStreamOK = parser->Expect(' ');
        }
     
        //Look up the proper header enumeration based on the header string.
        HTTPHeader theHeader = HttpProtocol::GetHeader(&theKeyWord);
      
        StrPtrLen theHeaderVal;
        isStreamOK = parser->GetThruEOL(&theHeaderVal);
      
        if (!isStreamOK)
        {       // No EOL after header!
            fStatusCode = httpBadRequest;
            return -1;
        }
      
        // If this is the connection header
        if ( theHeader == httpConnectionHeader )
        { // Set the keep alive boolean based on the connection header value
            SetKeepAlive(&theHeaderVal);
        }
      
        // Have the header field and the value; Add value to the array
        // If the field is invalid (or unrecognized) just skip over gracefully
        if ( theHeader != httpIllegalHeader )
            fFieldValues[theHeader] = theHeaderVal;
            
    }
  
    isStreamOK = parser->ExpectEOL();
  
    return 0;
}

int HttpRequest::ParseURI(StringParser* parser)
{
    // read in the complete URL into fRequestAbsURI
    parser->ConsumeUntil(&fAbsoluteURI, sURLStopConditions);
  
    StringParser urlParser(&fAbsoluteURI);
  
    // we always should have a slash before the URI
    // If not, that indicates this is a full URI
    if (fAbsoluteURI.Ptr[0] != '/')
    {
	    //if it is a full URL, store the scheme and host name
	    urlParser.ConsumeLength(&fAbsoluteURIScheme, 7); //consume "http://"
	    urlParser.ConsumeUntil(&fHostHeader, '/');
    }
  
    // whatever is in this position is the relative URI
    StrPtrLen relativeURI(urlParser.GetCurrentPosition(), urlParser.GetDataReceivedLen() - urlParser.GetDataParsedLen());
    // read this URI into fRequestRelURI
    fRelativeURI = relativeURI;

    urlParser.ConsumeUntil(&fURIPath, '?');
    
    // Allocate memory for fRequestPath
    u_int32_t len = fRelativeURI.Len;
#if 0
    
    len++;    
    char* relativeURIDecoded = new char[len];
    
    int32_t theBytesWritten = StringTranslator::DecodeURL(fRelativeURI.Ptr, fRelativeURI.Len,
                                                       relativeURIDecoded, len);
     
    //if negative, an error occurred, reported as an QTSS_Error
    //we also need to leave room for a terminator.
    if ((theBytesWritten < 0) || ((u_int32_t)theBytesWritten == len))
    {
        fStatusCode = httpBadRequest;
        return -1;
    }
    
    fRequestPath = new char[theBytesWritten + 1];
    //::memcpy(fRequestPath, relativeURIDecoded + 1, theBytesWritten); 
    ::memcpy(fRequestPath, relativeURIDecoded, theBytesWritten); 
    fRequestPath[theBytesWritten] = '\0';
    
    delete []relativeURIDecoded;
    
#else    
    fRequestPath = new char[len+1];
    ::memcpy(fRequestPath, fRelativeURI.Ptr, len); 
    fRequestPath[len] = '\0';    
#endif

	{
		StrPtrLen request_path(fRequestPath);
		StringParser urlParser(&request_path);
		urlParser.ConsumeUntil(&fURIPath, '?');
		
	    StrPtrLen UriParams(urlParser.GetCurrentPosition(), urlParser.GetDataReceivedLen() - urlParser.GetDataParsedLen());
	    fURIParams = UriParams;

		if(fURIParams.Len > 0)
		{
			// todo:
	    	//ParseParams(&urlParser);    
	    }
    }
    
    return 0;
}

int HttpRequest::ParseFirstLine(StringParser* parser)
{   
    // Get the method - If the method is not one of the defined methods
    // then it doesn't return an error but sets fMethod to httpIllegalMethod
    StrPtrLen theParsedData;
    parser->ConsumeWord(&theParsedData);
    fMethod = HttpProtocol::GetMethod(&theParsedData);
    
    // Consume whitespace
    parser->ConsumeWhitespace();
  
    // Parse the URI - If it fails returns an error after setting 
    // the fStatusCode to the appropriate error code
    int err = ParseURI(parser);
    if (err != 0)
            return err;
  
    // Consume whitespace
    parser->ConsumeWhitespace();
  
    // If there is a version, consume the version string
    StrPtrLen versionStr;
    parser->ConsumeUntil(&versionStr, StringParser::sEOLMask);
    // Check the version
    if (versionStr.Len > 0)
    	fVersion = HttpProtocol::GetVersion(&versionStr);
  
    // Go past the end of line
    if (!parser->ExpectEOL())
    {
        fStatusCode = httpBadRequest;
        return -1;     // Request line is not properly formatted!
    }

    return 0;
}

int  HttpRequest::Parse(StrPtrLen* str)
{
    int ret = 0;

    fFullRequest.Set(str->Ptr, str->Len);
    
    StringParser parser(str); 
    //parse status line.
    ret = this->ParseFirstLine(&parser);
    //handle any errors that come up    
    if (ret < 0)
    {
        return ret;
    }   

    ret = this->ParseHeaders(&parser);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
};

void HttpRequest::Clear()
{
    fFullRequest.Set(NULL, 0);    
    fMethod = httpGetMethod;
    fVersion = http10Version;    
    fAbsoluteURI.Set(NULL, 0);    
    fRelativeURI.Set(NULL, 0);    
    fAbsoluteURIScheme.Set(NULL, 0);    
    fHostHeader.Set(NULL, 0);
    if(fRequestPath != NULL)
    {
    	delete [] fRequestPath;
    	fRequestPath = NULL;
    }
    fStatusCode = httpOK; 
    fURIParams.Set(NULL, 0);
    #if 0
    if(fParamPairs != NULL)
    {
    	deque_release(fParamPairs, UriParam_release);    	
		fParamPairs = NULL;
	}
	#endif
}


#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "StringParser.h"
#include "HttpProtocol.h"
#include "StrPtrLen.h"

class HttpRequest
{
public:
	HttpRequest();
	virtual ~HttpRequest();
	int		Parse(StrPtrLen* str);
	void 	Clear();

public:
	int 	ParseFirstLine(StringParser* parser);
	int 	ParseURI(StringParser* parser);
	int     ParseHeaders(StringParser* parser);
	void 	SetKeepAlive(StrPtrLen *keepAliveValue);
	
	StrPtrLen			fFullRequest;
	HTTPMethod  		fMethod;     
	HTTPVersion     	fVersion;
    // For the URI (fAbsoluteURI and fRelativeURI are the same if the URI is of the form "/path")
	StrPtrLen       	fAbsoluteURI;       // If it is of the form "http://foo.bar.com/path?params"
	StrPtrLen       	fRelativeURI;       // If it is of the form "/path?params"
    StrPtrLen           fAbsoluteURIScheme;
    StrPtrLen           fHostHeader;        // If the full url is given in the request line
    char*               fRequestPath;       // Also contains the query string
    StrPtrLen           fURIPath;       	// If it is of the form "/path"
    StrPtrLen			fURIParams;			// ?key1=value&key2=value2
    HTTPStatusCode		fStatusCode;
    bool	            fRequestKeepAlive;  // Keep-alive information in the client request
    StrPtrLen           fFieldValues[httpNumHeaders];   // Array of header field values parsed from the request

    static u_int8_t        sURLStopConditions[];
};

#endif
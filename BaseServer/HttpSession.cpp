#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "common.h"
#include "StringParser.h"
#include "TaskThread.h"
#include "HttpSession.h"

#define BASE_SERVER_NAME		"OneServer"
#define BASE_SERVER_VERSION		"0.1.0"
#define MAX_TIME_LEN			64
#define HTML_HOME				"/home/html"
#define CHARSET_UTF8			"utf-8"

#define CONTENT_TYPE_TEXT_PLAIN					"text/plain"
#define CONTENT_TYPE_TEXT_HTML					"text/html"
#define CONTENT_TYPE_TEXT_CSS					"text/CSS"
//#define CONTENT_TYPE_TEXT_XML					"text/xml"
#define CONTENT_TYPE_TEXT_XSL					"text/xsl"

#define CONTENT_TYPE_APPLICATION_JAVASCRIPT		"application/javascript"
#define CONTENT_TYPE_APPLICATION_JSON			"application/json"
#define CONTENT_TYPE_APPLICATION_XML			"application/xml" // text/xml ?
#define CONTENT_TYPE_APPLICATION_SWF			"application/x-shockwave-flash" 
// audio/video
//#define CONTENT_TYPE_APPLICATION_M3U8			"application/x-mpegURL"
#define CONTENT_TYPE_APPLICATION_M3U8			"application/vnd.apple.mpegurl"
#define CONTENT_TYPE_VIDEO_FLV					"video/x-flv" 
#define CONTENT_TYPE_VIDEO_MP4					"video/mp4"
#define CONTENT_TYPE_AUDIO_MPEG					"audio/mpeg"
#define CONTENT_TYPE_VIDEO_QUICKTIME			"video/quicktime"
#define CONTENT_TYPE_VIDEO_MP2T					"video/MP2T"
// images
#define CONTENT_TYPE_IMAGE_PNG					"image/png"
#define CONTENT_TYPE_IMAGE_JPEG					"image/jpeg"
#define CONTENT_TYPE_IMAGE_GIF					"image/gif"
#define CONTENT_TYPE_IMAGE_BMP					"image/bmp"
#define CONTENT_TYPE_IMAGE_ICO					"image/vnd.microsoft.icon"
#define CONTENT_TYPE_IMAGE_TIFF					"image/tiff"
#define CONTENT_TYPE_IMAGE_SVG					"image/svg+xml"
// archives
#define CONTENT_TYPE_APPLICATION_ZIP			"application/zip"
#define CONTENT_TYPE_APPLICATION_RAR			"application/x-rar-compressed"
#define CONTENT_TYPE_APPLICATION_MSDOWNLOAD		"application/x-msdownload"
#define CONTENT_TYPE_APPLICATION_CAB			"application/vnd.ms-cab-compressed"
// adobe
#define CONTENT_TYPE_APPLICATION_PDF			"application/pdf"
#define CONTENT_TYPE_IMAGE_PHOTOSHOP			"image/vnd.adobe.photoshop"
#define CONTENT_TYPE_APPLICATION_POSTSCRIPT		"application/postscript"
// ms office
#define CONTENT_TYPE_APPLICATION_MSWORD         "application/msword"
#define CONTENT_TYPE_APPLICATION_RTF	        "application/rtf"
#define CONTENT_TYPE_APPLICATION_EXCEL	        "application/vnd.ms-excel"
#define CONTENT_TYPE_APPLICATION_POWERPOINT     "application/vnd.ms-powerpoint"
// open office
#define CONTENT_TYPE_APPLICATION_ODT      		"application/vnd.oasis.opendocument.text"
#define CONTENT_TYPE_APPLICATION_ODS            "application/vnd.oasis.opendocument.spreadsheet"

// octet-stream
#define CONTENT_TYPE_APPLICATION_OCTET_STREAM	"application/octet-stream"


#if USE_FILE_BUFFER
bool file_exist(char* abs_path)
{
	int ret = 0;
	ret = g_files_master->AccessFile(abs_path);
	if(ret == 0)
	{
		return true;
	}
	
	return false;
}
#else
bool file_exist(char* abs_path)
{
	int ret = 0;
	ret = access(abs_path, R_OK);
	if(ret == 0)
	{
		return true;
	}
	
	return false;
}
#endif

char* file_suffix(char* abs_path)
{
	char* suffix = rindex(abs_path, '.');
	return suffix;
}

char* content_type_by_suffix(char* suffix)
{
	/*
	     'txt' => 'text/plain',
            'htm' => 'text/html',
            'html' => 'text/html',
            'php' => 'text/html',
            'css' => 'text/css',
            'js' => 'application/javascript',
            'json' => 'application/json',
            'xml' => 'application/xml',
            'swf' => 'application/x-shockwave-flash',
            'flv' => 'video/x-flv',

            // images
            'png' => 'image/png',
            'jpe' => 'image/jpeg',
            'jpeg' => 'image/jpeg',
            'jpg' => 'image/jpeg',
            'gif' => 'image/gif',
            'bmp' => 'image/bmp',
            'ico' => 'image/vnd.microsoft.icon',
            'tiff' => 'image/tiff',
            'tif' => 'image/tiff',
            'svg' => 'image/svg+xml',
            'svgz' => 'image/svg+xml',

            // archives
            'zip' => 'application/zip',
            'rar' => 'application/x-rar-compressed',
            'exe' => 'application/x-msdownload',
            'msi' => 'application/x-msdownload',
            'cab' => 'application/vnd.ms-cab-compressed',

            // audio/video
            'mp3' => 'audio/mpeg',
            'qt' => 'video/quicktime',
            'mov' => 'video/quicktime',

            // adobe
            'pdf' => 'application/pdf',
            'psd' => 'image/vnd.adobe.photoshop',
            'ai' => 'application/postscript',
            'eps' => 'application/postscript',
            'ps' => 'application/postscript',

            // ms office
            'doc' => 'application/msword',
            'rtf' => 'application/rtf',
            'xls' => 'application/vnd.ms-excel',
            'ppt' => 'application/vnd.ms-powerpoint',

            // open office
            'odt' => 'application/vnd.oasis.opendocument.text',
            'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
        */
        
	if(suffix == NULL)
	{
		return CONTENT_TYPE_TEXT_HTML;
	}

	if(strcasecmp(suffix, ".txt") == 0)
	{
		return CONTENT_TYPE_TEXT_PLAIN;
	}
	else if(strcasecmp(suffix, ".htm") == 0)
	{
		return CONTENT_TYPE_TEXT_HTML;
	}
	else if(strcasecmp(suffix, ".html") == 0)
	{
		return CONTENT_TYPE_TEXT_HTML;
	}
	else if(strcasecmp(suffix, ".php") == 0)
	{
		return CONTENT_TYPE_TEXT_HTML;
	}
	else if(strcasecmp(suffix, ".css") == 0)
	{
		return CONTENT_TYPE_TEXT_CSS;
	}
	else if(strcasecmp(suffix, ".xsl") == 0)
	{
		return CONTENT_TYPE_TEXT_XSL;
	}
	else if(strcasecmp(suffix, ".js") == 0)
	{
		return CONTENT_TYPE_APPLICATION_JAVASCRIPT;
	}
	else if(strcasecmp(suffix, ".json") == 0)
	{
		return CONTENT_TYPE_APPLICATION_JSON;
	}
	else if(strcasecmp(suffix, ".xml") == 0)
	{
		return CONTENT_TYPE_APPLICATION_XML;
	}
	else if(strcasecmp(suffix, ".swf") == 0)
	{
		return CONTENT_TYPE_APPLICATION_SWF;
	}
	else if(strcasecmp(suffix, ".m3u8") == 0)
	{
		return CONTENT_TYPE_APPLICATION_M3U8;
	}
	else if(strcasecmp(suffix, ".flv") == 0)
	{
		return CONTENT_TYPE_VIDEO_FLV;
	}
	else if(strcasecmp(suffix, ".mp4") == 0)
	{
		return CONTENT_TYPE_VIDEO_MP4;
	}
	else if(strcasecmp(suffix, ".mp3") == 0)
	{
		return CONTENT_TYPE_AUDIO_MPEG;
	}
	else if(strcasecmp(suffix, ".qt") == 0)
	{
		return CONTENT_TYPE_VIDEO_QUICKTIME;
	}
	else if(strcasecmp(suffix, ".mov") == 0)
	{
		return CONTENT_TYPE_VIDEO_QUICKTIME;
	}
	else if(strcasecmp(suffix, ".ts") == 0)
	{
		return CONTENT_TYPE_VIDEO_MP2T;
	}
    else if(strcasecmp(suffix, ".png") == 0)
	{
		return CONTENT_TYPE_IMAGE_PNG;
	}
	else if(strcasecmp(suffix, ".jpe") == 0)
	{
		return CONTENT_TYPE_IMAGE_JPEG;
	}
	else if(strcasecmp(suffix, ".jpeg") == 0)
	{
		return CONTENT_TYPE_IMAGE_JPEG;
	}
	else if(strcasecmp(suffix, ".jpg") == 0)
	{
		return CONTENT_TYPE_IMAGE_JPEG;
	}
	else if(strcasecmp(suffix, ".gif") == 0)
	{
		return CONTENT_TYPE_IMAGE_GIF;
	}
	else if(strcasecmp(suffix, ".bmp") == 0)
	{
		return CONTENT_TYPE_IMAGE_BMP;
	}
	else if(strcasecmp(suffix, ".ico") == 0)
	{
		return CONTENT_TYPE_IMAGE_ICO;
	}
	else if(strcasecmp(suffix, ".tiff") == 0)
	{
		return CONTENT_TYPE_IMAGE_TIFF;
	}
	else if(strcasecmp(suffix, ".tif") == 0)
	{
		return CONTENT_TYPE_IMAGE_TIFF;
	}
	else if(strcasecmp(suffix, ".svg") == 0)
	{
		return CONTENT_TYPE_IMAGE_SVG;
	}
	else if(strcasecmp(suffix, ".svgz") == 0)
	{
		return CONTENT_TYPE_IMAGE_SVG;
	}
    else if(strcasecmp(suffix, ".zip") == 0)
	{
		return CONTENT_TYPE_APPLICATION_ZIP;
	}
	else if(strcasecmp(suffix, ".rar") == 0)
	{
		return CONTENT_TYPE_APPLICATION_RAR;
	}
	else if(strcasecmp(suffix, ".exe") == 0)
	{
		return CONTENT_TYPE_APPLICATION_MSDOWNLOAD;
	}
	else if(strcasecmp(suffix, ".msi") == 0)
	{
		return CONTENT_TYPE_APPLICATION_MSDOWNLOAD;
	}
	else if(strcasecmp(suffix, ".cab") == 0)
	{
		return CONTENT_TYPE_APPLICATION_CAB;
	}
    else if(strcasecmp(suffix, ".pdf") == 0)
	{
		return CONTENT_TYPE_APPLICATION_PDF;
	}
	else if(strcasecmp(suffix, ".psd") == 0)
	{
		return CONTENT_TYPE_IMAGE_PHOTOSHOP;
	}
	else if(strcasecmp(suffix, ".ai") == 0)
	{
		return CONTENT_TYPE_APPLICATION_POSTSCRIPT;
	}
	else if(strcasecmp(suffix, ".eps") == 0)
	{
		return CONTENT_TYPE_APPLICATION_POSTSCRIPT;
	}
	else if(strcasecmp(suffix, ".ps") == 0)
	{
		return CONTENT_TYPE_APPLICATION_POSTSCRIPT;
	}
    else if(strcasecmp(suffix, ".doc") == 0)
	{
		return CONTENT_TYPE_APPLICATION_MSWORD;
	}
	else if(strcasecmp(suffix, ".rtf") == 0)
	{
		return CONTENT_TYPE_APPLICATION_RTF;
	}
	else if(strcasecmp(suffix, ".xls") == 0)
	{
		return CONTENT_TYPE_APPLICATION_EXCEL;
	}
	else if(strcasecmp(suffix, ".ppt") == 0)
	{
		return CONTENT_TYPE_APPLICATION_POWERPOINT;
	}
	else if(strcasecmp(suffix, ".odt") == 0)
	{
		return CONTENT_TYPE_APPLICATION_ODT;
	}
	else if(strcasecmp(suffix, ".ods") == 0)
	{
		return CONTENT_TYPE_APPLICATION_ODS;
	}
	else
	{
		return CONTENT_TYPE_TEXT_HTML;
	}
}

HttpSession::HttpSession(int fd, struct sockaddr_in * addr) : 
	TcpSession(fd, addr),     
    fResponse(NULL, 0)    
{	
	fprintf(stdout, "%s[%p]: fd=%d, 0x%08X:%u\n", __PRETTY_FUNCTION__, this, 
		m_SockFd, m_SockAddr.sin_addr.s_addr, m_SockAddr.sin_port);
#if USE_FILE_BUFFER
	m_CFile 	= NULL;
#else
	fFd		= -1;
#endif
	m_ReadCount = 0;
	fHaveRange 	= false;
	fRangeStart = 0;
	fRangeStop 	= -1;
}

HttpSession::~HttpSession()
{
	fprintf(stdout, "%s[%p][%p]: fd=%d, 0x%08X:%u\n", __PRETTY_FUNCTION__, this, m_task_thread,
		m_SockFd, m_SockAddr.sin_addr.s_addr, m_SockAddr.sin_port);	
#if USE_FILE_BUFFER
	if(m_CFile != NULL)
	{
		delete m_CFile;
		m_CFile = NULL;
	}
#else
	if(fFd != -1)
	{		
		close(fFd);
		fFd = -1;
	}
#endif
}

bool HttpSession::IsFullRequest()
{
	m_StrRequest.Ptr = m_StrReceived.Ptr;
    m_StrRequest.Len = 0;
    
    StringParser headerParser(&m_StrReceived);
    while (headerParser.GetThruEOL(NULL))
    {        
        if (headerParser.ExpectEOL())
        {
            m_StrRequest.Len = headerParser.GetDataParsedLen();
            return true;
        }
    }
    
    return false;
}

void HttpSession::MoveOnRequest()
{
    StrPtrLen   strRemained;
    strRemained.Set(m_StrRequest.Ptr+m_StrRequest.Len, m_StrReceived.Len-m_StrRequest.Len);
        
    ::memmove(m_RequestBuffer, strRemained.Ptr, strRemained.Len);
    m_StrReceived.Set(m_RequestBuffer, strRemained.Len);
    m_StrRequest.Set(m_RequestBuffer, 0);
}


int HttpSession::ResponseError(HTTPStatusCode status_code)
{
	char	buffer[1024];
	StringFormatter content(buffer, sizeof(buffer));
	content.Put("<HTML>\n");
	content.Put("<BODY>\n");
	content.Put("<TABLE border=0>\n");

	content.Put("<TR>\n");	
	content.Put("<TD>\n");
	content.Put("url:\n");
	content.Put("</TD>\n");
	content.Put("<TD>\n");
	content.PutFmtStr("%s\n", m_Request.fRequestPath);
	content.Put("</TD>\n");	
	content.Put("</TR>\n");
	
	content.Put("<TR>\n");	
	content.Put("<TD>\n");
	content.Put("status:\n");
	content.Put("</TD>\n");
	content.Put("<TD>\n");
	content.PutFmtStr("%s\n", HttpProtocol::GetStatusCodeAsString(status_code)->Ptr);
	content.Put("</TD>\n");	
	content.Put("</TR>\n");

	content.Put("<TR>\n");	
	content.Put("<TD>\n");
	content.Put("reason:");
	content.Put("</TD>\n");
	content.Put("<TD>\n");
	content.PutFmtStr("%s\n", HttpProtocol::GetStatusCodeString(status_code)->Ptr);
	content.Put("</TD>\n");	
	content.Put("</TR>\n");

	content.Put("<TR>\n");	
	content.Put("<TD>\n");
	content.Put("server:\n");
	content.Put("</TD>\n");
	content.Put("<TD>\n");
	content.PutFmtStr("%s/%s\n", BASE_SERVER_NAME, BASE_SERVER_VERSION);	
	content.Put("</TD>\n");	
	content.Put("</TR>\n");

	content.Put("<TR>\n");	
	content.Put("<TD>\n");
	content.Put("time:");
	content.Put("</TD>\n");
	content.Put("<TD>\n");
	time_t now = time(NULL);
	char str_now[MAX_TIME_LEN] = {0};
	ctime_r(&now, str_now);
	content.PutFmtStr("%s", str_now);
	content.Put("</TD>\n");	
	content.Put("</TR>\n");

	content.Put("</TABLE>\n");		
	content.Put("</BODY>\n");
	content.Put("</HTML>\n");

    fContentLen = content.GetBytesWritten();
    
    fResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);
    fResponse.PutFmtStr("%s %s %s\r\n", 
    	HttpProtocol::GetVersionString(http11Version)->Ptr,
    	HttpProtocol::GetStatusCodeAsString(status_code)->Ptr,
    	HttpProtocol::GetStatusCodeString(status_code)->Ptr);
    fResponse.PutFmtStr("Server: %s/%s\r\n", BASE_SERVER_NAME, BASE_SERVER_VERSION);
    fResponse.PutFmtStr("Content-Length: %d\r\n", fContentLen);
    fResponse.PutFmtStr("Content-Type: %s;charset=%s\r\n", CONTENT_TYPE_TEXT_HTML, CHARSET_UTF8);
    fResponse.Put("\r\n"); 
    fResponse.Put(content.GetBufPtr(), content.GetBytesWritten());

    m_StrResponse.Set(fResponse.GetBufPtr(), fResponse.GetBytesWritten());
    //append to m_StrRemained
    m_StrRemained.Len += m_StrResponse.Len;  
    //clear previous response.
    m_StrResponse.Set(m_ResponseBuffer, 0);
    
	return 0;

}


bool HttpSession::SendDone()
{
	bool ret = false;

	if(m_ReadCount == fContentLen)
	{
	#if USE_FILE_BUFFER
		if(m_CFile != NULL)
		{
			delete m_CFile;
			m_CFile = NULL;
		}
	#else
		if(fFd != -1)
		{
			close(fFd);
			fFd = -1;
		}
	#endif
		return true;
	}

	return ret;
}

#if USE_FILE_BUFFER
int HttpSession::ResponseFile(char* abs_path)
{
	int ret = 0;

	m_CFile = g_files_master->OpenFile(abs_path);	
	if(m_CFile == NULL)
	{
		fHttpStatus = httpInternalServerError;
		ret = ResponseError(fHttpStatus);
		return ret;
	}	
	m_ReadCount = 0;
	
	off_t file_len = m_CFile->GetFileLength();	
	if(fRangeStop == -1)
	{
		fRangeStop = file_len - 1;
	}	

	fHttpStatus = httpOK;	
	if(fHaveRange)
	{
		if(fRangeStart > fRangeStop)
		{
			fHttpStatus = httpRequestRangeNotSatisfiable;
			ret = ResponseError(fHttpStatus);
			delete m_CFile;
			m_CFile = NULL;
			return ret;
		}
		if(fRangeStop > file_len - 1)
		{
			fHttpStatus = httpRequestRangeNotSatisfiable;
			ret = ResponseError(fHttpStatus);
			delete m_CFile;
			m_CFile = NULL;
			return ret;
		}
		
		fprintf(stdout, "%s: range=%ld-%ld", __PRETTY_FUNCTION__, fRangeStart, fRangeStop);
		fHttpStatus = httpPartialContent;
	}
	
	char* suffix = file_suffix(abs_path);
	char* content_type = content_type_by_suffix(suffix);
	fContentLen = fRangeStop+1-fRangeStart;
	
	fResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);		
	fResponse.PutFmtStr("%s %s %s\r\n", 
			HttpProtocol::GetVersionString(http11Version)->Ptr,
			HttpProtocol::GetStatusCodeAsString(fHttpStatus)->Ptr,			
			HttpProtocol::GetStatusCodeString(fHttpStatus)->Ptr);
    fResponse.PutFmtStr("Server: %s/%s\r\n", BASE_SERVER_NAME, BASE_SERVER_VERSION);
    fResponse.PutFmtStr("Accept-Ranges: bytes\r\n");	
    fResponse.PutFmtStr("Content-Length: %ld\r\n", fContentLen);    
    if(fHaveRange)
    {
    	//Content-Range: 1000-3000/5000
    	fResponse.PutFmtStr("Content-Range: bytes %ld-%ld/%ld\r\n", fRangeStart, fRangeStop, file_len);    
    }
    //fResponse.PutFmtStr("Content-Type: %s; charset=utf-8\r\n", content_type);
    fResponse.PutFmtStr("Content-Type: %s", content_type);  
    if(strcmp(content_type, CONTENT_TYPE_TEXT_HTML) == 0)
    {
    	fResponse.PutFmtStr(";charset=%s\r\n", CHARSET_UTF8);
    }
    else
    {
    	fResponse.Put("\r\n");
    }    
    fResponse.Put("\r\n"); 
        
    m_StrResponse.Set(fResponse.GetBufPtr(), fResponse.GetBytesWritten());
    //append to m_StrRemained
    m_StrRemained.Len += m_StrResponse.Len;  
    //clear previous response.
    m_StrResponse.Set(m_ResponseBuffer, 0);
   
	return 0;
}
#else
int HttpSession::ResponseFile(char* abs_path)
{
	int ret = 0;
	
	fFd = open(abs_path, O_RDONLY);
	if(fFd == -1)
	{
		fHttpStatus = httpInternalServerError;
		ret = ResponseError(fHttpStatus);
		return ret;
	}
	m_ReadCount = 0;
	
	off_t file_len = lseek(fFd, 0L, SEEK_END);	
	if(fRangeStop == -1)
	{
		fRangeStop = file_len - 1;
	}	

	fHttpStatus = httpOK;	
	if(fHaveRange)
	{
		if(fRangeStart > fRangeStop)
		{
			fHttpStatus = httpRequestRangeNotSatisfiable;
			ret = ResponseError(fHttpStatus);
			close(fFd);
			fFd = -1;
			return ret;
		}
		if(fRangeStop > file_len - 1)
		{
			fHttpStatus = httpRequestRangeNotSatisfiable;
			ret = ResponseError(fHttpStatus);
			close(fFd);
			fFd = -1;
			return ret;
		}
		
		fprintf(stdout, "%s: range=%ld-%ld", __PRETTY_FUNCTION__, fRangeStart, fRangeStop);
		fHttpStatus = httpPartialContent;
	}
	
	lseek(fFd, fRangeStart, SEEK_SET);
	
	char* suffix = file_suffix(abs_path);
	char* content_type = content_type_by_suffix(suffix);
	fContentLen = fRangeStop+1-fRangeStart;
	
	fResponse.Set(m_StrRemained.Ptr+m_StrRemained.Len, RESPONSE_BUFF_SIZE-m_StrRemained.Len);		
	fResponse.PutFmtStr("%s %s %s\r\n", 
			HttpProtocol::GetVersionString(http11Version)->Ptr,
			HttpProtocol::GetStatusCodeAsString(fHttpStatus)->Ptr,			
			HttpProtocol::GetStatusCodeString(fHttpStatus)->Ptr);
    fResponse.PutFmtStr("Server: %s/%s\r\n", BASE_SERVER_NAME, BASE_SERVER_VERSION);
    fResponse.PutFmtStr("Accept-Ranges: bytes\r\n");	
    fResponse.PutFmtStr("Content-Length: %ld\r\n", fContentLen);    
    if(fHaveRange)
    {
    	//Content-Range: 1000-3000/5000
    	fResponse.PutFmtStr("Content-Range: bytes %ld-%ld/%ld\r\n", fRangeStart, fRangeStop, file_len);    
    }
    //fResponse.PutFmtStr("Content-Type: %s; charset=utf-8\r\n", content_type);
    fResponse.PutFmtStr("Content-Type: %s", content_type);  
    if(strcmp(content_type, CONTENT_TYPE_TEXT_HTML) == 0)
    {
    	fResponse.PutFmtStr(";charset=%s\r\n", CHARSET_UTF8);
    }
    else
    {
    	fResponse.Put("\r\n");
    }    
    fResponse.Put("\r\n"); 
        
    m_StrResponse.Set(fResponse.GetBufPtr(), fResponse.GetBytesWritten());
    //append to m_StrRemained
    m_StrRemained.Len += m_StrResponse.Len;  
    //clear previous response.
    m_StrResponse.Set(m_ResponseBuffer, 0);
   
	return 0;
}
#endif

int HttpSession::DoGet()
{
	int ret = 0;
	fprintf(stdout, "%s[%p]: %s\n", __PRETTY_FUNCTION__, this, m_Request.fRequestPath);
	
	char abs_path[PATH_MAX];
	snprintf(abs_path, PATH_MAX, "%s%s", HTML_HOME, m_Request.fRequestPath);
	abs_path[PATH_MAX-1] = '\0';		
	if(file_exist(abs_path))
	{
		ret = ResponseFile(abs_path);
	}
	else
	{
		fHttpStatus = httpNotFound;
		ret = ResponseError(fHttpStatus);		
	}	
	
	return ret;
}

int HttpSession::DoRequest()
{
	int ret = 0;
	
	m_Request.Clear();
	m_Request.Parse(&m_StrRequest);
	MoveOnRequest();
	
	if(m_Request.fMethod == httpGetMethod)
	{
		ret = DoGet();
	}
	else if(m_Request.fMethod == httpPostMethod)
	{
		//ret = DoPost();
	}
	else if(m_Request.fMethod == httpHeadMethod)
	{
		//ret = DoHead();
	}
	
	return ret;
}

int HttpSession::DoRead()
{
	int ret = 0;
	
	ret = RecvData();
	if(ret < 0)
	{
		return ret;
	}

	while(1)
	{
		if(IsFullRequest())
		{
			ret = DoRequest();
			if(ret < 0)
			{
				return ret;
			}

			ret = SendData();
			if(ret != 0)
			{
				return ret;
			}

			while(!SendDone())
			{
				ret = DoContinue();
				if(ret != 0)
				{
					return ret;
				}
			}
			
		}	
		else
		{
			break;
		}
	}

	return ret;
}

#if USE_FILE_BUFFER
int HttpSession::DoContinue()
{
	int ret = 0;
	ret = SendData();
	if(ret != 0)
	{
		return ret;
	}

	if(m_CFile != NULL)
	{
		StrPtrLen buffer_availiable;
		buffer_availiable.Ptr = m_StrRemained.Ptr + m_StrRemained.Len;
		buffer_availiable.Len = RESPONSE_BUFF_SIZE - m_StrRemained.Len;
		if(buffer_availiable.Len <= 0)
		{
			return SEND_TO_BE_CONTINUE;
		}
		
		ssize_t read_len = m_CFile->Read((u_int8_t*)buffer_availiable.Ptr, buffer_availiable.Len);
		if(read_len < 0)
		{
			fprintf(stderr, "%s[%p]: read ret=%ld, errno=%d, %s\n", 
				__PRETTY_FUNCTION__, this, read_len, errno, strerror(errno));
			delete m_CFile;
			m_CFile = NULL;
			return -1;
		}
		else if(read_len == 0)
		{
			fprintf(stdout, "%s[%p]: read want=%d, ret=%ld, [end of file]\n", 
				__PRETTY_FUNCTION__, this, buffer_availiable.Len, read_len);
			delete m_CFile;
			m_CFile = NULL;
			// normal
			//return -1;			
		}
		else if(read_len < buffer_availiable.Len)
		{
			fprintf(stdout, "%s[%p]: read want=%d, ret=%ld, [read -> end_of_file]\n", 
				__PRETTY_FUNCTION__, this, buffer_availiable.Len, read_len);
			m_StrRemained.Len += read_len;
			m_ReadCount += read_len;
		}
		else
		{
			m_StrRemained.Len += read_len;
			m_ReadCount += read_len;
		}
		
		ret = SendData();
		if(ret != 0)
		{
			return ret;
		}
	}

	if(!SendDone())
	{		
		return SEND_TO_BE_CONTINUE;
	}
		
	return 0;
}
#else
int HttpSession::DoContinue()
{
	int ret = 0;
	ret = SendData();
	if(ret != 0)
	{
		return ret;
	}

	if(fFd != -1)
	{
		StrPtrLen buffer_availiable;
		buffer_availiable.Ptr = m_StrRemained.Ptr + m_StrRemained.Len;
		buffer_availiable.Len = RESPONSE_BUFF_SIZE - m_StrRemained.Len;
		if(buffer_availiable.Len <= 0)
		{
			return 1;
		}
		
		ssize_t read_len = read(fFd, buffer_availiable.Ptr, buffer_availiable.Len);
		if(read_len < 0)
		{
			fprintf(stderr, "%s[%p]: read ret=%d, errno=%d, %s\n", 
				__PRETTY_FUNCTION__, this, read_len, errno, strerror(errno));
			close(fFd);
			fFd = -1;
			return -1;
		}
		else if(read_len == 0)
		{
			fprintf(stdout, "%s[%p]: read want=%d, ret=%d, [end of file or ?]\n", 
				__PRETTY_FUNCTION__, this, buffer_availiable.Len, read_len);
			close(fFd);
			fFd = -1;
			// normal
			//return -1;			
		}
		else if(read_len < buffer_availiable.Len)
		{
			fprintf(stdout, "%s[%p]: read want=%d, ret=%d, [?]\n", 
				__PRETTY_FUNCTION__, this, buffer_availiable.Len, read_len);
			m_StrRemained.Len += read_len;
			m_ReadCount += read_len;
		}
		else
		{
			m_StrRemained.Len += read_len;
			m_ReadCount += read_len;
		}
		
		ret = SendData();
		if(ret != 0)
		{
			return ret;
		}
	}

	if(!SendDone())
	{		
		return SEND_TO_BE_CONTINUE;
	}
		
	return 0;
}
#endif

int HttpSession::DoEvents(u_int32_t events, TaskThread* threadp)
{
	int ret = 0;

	if(events & EPOLLERR)
	{
		delete this;
		return 0;
	}
	if(events & EPOLLHUP)
	{
		delete this;
		return 0;
	}
	if(events & EVENT_READ)
	{
		ret = DoRead();	
		if(ret < 0)
		{
			delete this;
			return 0;
		}
		else if(ret == 0)
		{
			// do nothing.
		}
		else
		{
			m_task_thread->m_EventsMaster.ModifyWatch(m_SockFd, EVENT_READ|EVENT_WRITE, this);
		}
	}
	if(events & EVENT_WRITE)
	{
		ret = DoContinue();
		if(ret < 0)
		{
			delete this;
			return 0;
		}
		else if(ret == 0)
		{
			m_task_thread->m_EventsMaster.ModifyWatch(m_SockFd, EVENT_READ, this);
		}
		else
		{
			// do nothing.
		}
	}
	
	return 0;
}


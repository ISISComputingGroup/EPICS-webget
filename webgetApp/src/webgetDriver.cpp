/*************************************************************************\ 
* Copyright (c) 2013 Science and Technology Facilities Council (STFC), GB. 
* All rights reverved. 
* This file is distributed subject to a Software License Agreement found 
* in the file LICENSE.txt that is included with this distribution. 
\*************************************************************************/ 

/// @file webgetDriver.cpp Implementation of #webgetDriver class and webgetConfigure() iocsh command
/// @author Freddie Akeroyd, STFC ISIS Facility, GB

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <iostream>
#include <map>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <errlog.h>
#include <iocsh.h>
#include <epicsExit.h>

// CURL
#include <curl/curl.h>
#include <curl/easy.h>

// HTMLtidy
#include <tidy.h>
#include <buffio.h>
#include <stdio.h>
#include <errno.h>

// pugixml
#include "pugixml.hpp"

#include "asynPortDriver.h"

#include <epicsExport.h>

#include "webgetDriver.h"

static const char *driverName="webgetDriver"; ///< Name of driver for use in message printing 

/// EPICS driver report function for iocsh dbior command
void webgetDriver::report(FILE* fp, int details)
{
	asynPortDriver::report(fp, details);
}

struct WriteCallbackData 
{
  std::string& data;
  WriteCallbackData(std::string& data_) : data(data_) { }
};
 
static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct WriteCallbackData *mem = (struct WriteCallbackData *)userp;
    mem->data.append(std::string((const char*)contents, realsize));
    return realsize;
}

/// Constructor for the webgetDriver class.
/// Calls constructor for the asynPortDriver base class and sets up driver parameters.
///
/// \param[in] portName @copydoc initArg0
webgetDriver::webgetDriver(const char *portName, unsigned options) 
	: asynPortDriver(portName, 
	0, /* maxAddr */ 
	NUM_WEBGET_PARAMS,
	asynInt32Mask | asynInt32ArrayMask | asynFloat64Mask | asynFloat64ArrayMask | asynOctetMask | asynDrvUserMask, /* Interface mask */
	asynInt32Mask | asynInt32ArrayMask | asynFloat64Mask | asynFloat64ArrayMask | asynOctetMask,  /* Interrupt mask */
	ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
	1, /* Autoconnect */
	0, /* Default priority */
	0),	/* Default stack size*/
	m_shutdown(false), m_options(options)
{
	const char *functionName = "webgetDriver";
//  curl_global_init(CURL_GLOBAL_ALL);
	createParam(P_URL0String, asynParamOctet, &P_URL0);
	createParam(P_Data0String, asynParamOctet, &P_Data0);
	createParam(P_IData0String, asynParamInt32, &P_IData0);
	createParam(P_FData0String, asynParamFloat64, &P_FData0);
	createParam(P_PollTimeString, asynParamFloat64, &P_PollTime);
	createParam(P_XPath0String, asynParamOctet, &P_XPath0);
    setStringParam(P_URL0, "");
    setStringParam(P_Data0, "");
    setIntegerParam(P_IData0, 0);
    setDoubleParam(P_FData0, 0.0);
    setDoubleParam(P_PollTime, 0.0);
    setStringParam(P_XPath0, "//*"); // return everything
//  curl_global_cleanup();
    if (epicsThreadCreate("webgetDriverPoller",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)pollerTaskC, this) == 0)
    {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }
}

void webgetDriver::pollerTask()
{
    double poll_time = 0.0;
    while(!m_shutdown)
	{
        getDoubleParam(P_PollTime, &poll_time);
        if (poll_time > 0.0)
        {
	        lock();
            processURL();
		    unlock();
		    epicsThreadSleep(poll_time);
        }
        else
        {
		    epicsThreadSleep(5.0);
        }
	}
	m_shutdown = false;
}

void webgetDriver::processURL()
{
	char url0[256], xpath0[256];
	std::string data, value;
	    getStringParam(P_URL0, sizeof(url0), url0);
	    getStringParam(P_XPath0, sizeof(xpath0), xpath0);
		if (strlen(url0) > 0)
		{
		    readURL(url0, data);
			value = runXPath(data, xpath0);
			setStringParam(P_Data0, value.c_str());
			setIntegerParam(P_IData0, atol(value.c_str()));
			setDoubleParam(P_FData0, atof(value.c_str()));
			callParamCallbacks();
		}
}

void webgetDriver::readURL(const char* url, std::string& data)
{
    data.clear();
	std::string raw_data;
	CURL *curl = curl_easy_init();
    if(curl) 
	{
        CURLcode res;
        WriteCallbackData* cd = new WriteCallbackData(raw_data);
        curl_easy_setopt(curl, CURLOPT_URL, url);
	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)cd);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
		{
            errlogPrintf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
        curl_easy_cleanup(curl);
		delete cd;
		tidyHTML2XHTML(raw_data , data, checkOption(TidyWarnings));
	}
}

asynStatus webgetDriver::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
    double poll_time = 0.0;
    getDoubleParam(P_PollTime, &poll_time);
    if ( poll_time <= 0.0 && function == P_Data0 )
    {
        processURL();
    }
    return asynPortDriver::readOctet(pasynUser, value, maxChars, nActual, eomReason);
}

asynStatus webgetDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
    int function = pasynUser->reason;
    double poll_time = 0.0;
    getDoubleParam(P_PollTime, &poll_time);
    if ( poll_time <= 0.0 && function == P_IData0 )
    {
        processURL();
    }
    return asynPortDriver::readInt32(pasynUser, value);
}

asynStatus webgetDriver::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
    int function = pasynUser->reason;
    double poll_time = 0.0;
    getDoubleParam(P_PollTime, &poll_time);
    if ( poll_time <= 0.0 && function == P_FData0 )
    {
        processURL();
    }
    return asynPortDriver::readFloat64(pasynUser, value);
}

int webgetDriver::tidyHTML2XHTML(const std::string& html_in, std::string& xhtml_out, bool warnings)
{
  TidyBuffer output = {0};
  TidyBuffer errbuf = {0};
  int rc = -1;
  Bool ok;

  TidyDoc tdoc = tidyCreate();                     // Initialize "document"

  ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
  if ( ok )
    rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
  if ( rc >= 0 )
    rc = tidyParseString( tdoc, html_in.c_str() ); // Parse the input
  if ( rc >= 0 )
    rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
  if ( rc >= 0 )
    rc = tidyRunDiagnostics( tdoc );               // Kvetch
  if ( rc > 1 )                                    // If error, force output.
    rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
  if ( rc >= 0 )
    rc = tidySaveBuffer( tdoc, &output );          // Pretty Print

  if ( rc >= 0 )
  {
    if ( rc > 0 )
	{
	    if (warnings)
		{
		    errlogPrintf("HTMLTidy: %s\n", errbuf.bp);
		}
	}
    xhtml_out = (const char*)output.bp;
  }
  else
  {
	errlogPrintf("HTMLTidy failed: error code %d\n", rc);
	xhtml_out = "";
  }
  tidyBufFree( &output );
  tidyBufFree( &errbuf );
  tidyRelease( tdoc );
  return rc;
}


std::string webgetDriver::runXPath(const std::string& xml_str, const std::string& xpath_str)
{
    pugi::xml_document doc;
	if (xpath_str.size() == 0)
	{
		return "";
	}
	try
	{
	    pugi::xml_parse_result result = doc.load_buffer(xml_str.c_str(), xml_str.size());
        if (!result)
		{
			std::cerr << "webgetDriver::runXPath " << result << std::endl;
		    return "";
		}
		pugi::xpath_query query(xpath_str.c_str());
		return query.evaluate_string(doc);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "webgetDriver::runXPath " << ex.what() << std::endl;
		return "";
	}
}
	
extern "C" {

	/// The function is registered via webgetRegister().
	///
	/// @param[in] portName @copydoc initArg0
	int webgetConfigure(const char *portName, int options)		
	{
		try
		{
			new webgetDriver(portName, options);
			return(asynSuccess);
		}
		catch(const std::exception& ex)
		{
			errlogSevPrintf(errlogFatal, "webgetConfigure failed: %s\n", ex.what());
			return(asynError);
		}
	}

	// EPICS iocsh shell commands 

	static const iocshArg initArg0 = { "portName", iocshArgString};			///< A name for the asyn driver instance we will create - used to refer to it from EPICS DB files
	static const iocshArg initArg1 = { "options", iocshArgInt};			///< A name for the asyn driver instance we will create - used to refer to it from EPICS DB files

	static const iocshArg * const initArgs[] = { &initArg0, &initArg1 };

	static const iocshFuncDef initFuncDef = {"webgetConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

	static void initCallFunc(const iocshArgBuf *args)
	{
		webgetConfigure(args[0].sval, args[1].ival);
	}
	
	/// Register new commands with EPICS IOC shell
	static void webgetRegister(void)
	{
		iocshRegister(&initFuncDef, initCallFunc);
	}

	epicsExportRegistrar(webgetRegister);

}


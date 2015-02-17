/*************************************************************************\ 
* Copyright (c) 2013 Science and Technology Facilities Council (STFC), GB. 
* All rights reverved. 
* This file is distributed subject to a Software License Agreement found 
* in the file LICENSE.txt that is included with this distribution. 
\*************************************************************************/ 

/// @file lvDCOMDriver.cpp Implementation of #lvDCOMDriver class and lvDCOMConfigure() iocsh command
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

#include "webgetDriver.h"
#include <epicsExport.h>

#include <curl/curl.h>
#include <curl/easy.h>

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

/// Constructor for the lvDCOMDriver class.
/// Calls constructor for the asynPortDriver base class and sets up driver parameters.
///
/// \param[in] dcomint DCOM interface pointer created by lvDCOMConfigure()
/// \param[in] portName @copydoc initArg0
webgetDriver::webgetDriver(const char *portName) 
	: asynPortDriver(portName, 
	0, /* maxAddr */ 
	10,
	asynInt32Mask | asynInt32ArrayMask | asynFloat64Mask | asynFloat64ArrayMask | asynOctetMask | asynDrvUserMask, /* Interface mask */
	asynInt32Mask | asynInt32ArrayMask | asynFloat64Mask | asynFloat64ArrayMask | asynOctetMask,  /* Interrupt mask */
	ASYN_CANBLOCK, /* asynFlags.  This driver can block but it is not multi-device */
	1, /* Autoconnect */
	0, /* Default priority */
	0)	/* Default stack size*/
{
	int i;
	const char *functionName = "webgetDriver";
//  curl_global_init(CURL_GLOBAL_ALL);
	createParam(P_URL0String, asynParamOctet, &P_URL0);
	createParam(P_Data0String, asynParamOctet, &P_Data0);
	createParam(P_PollTimeString, asynParamFloat64, &P_PollTime);
    setStringParam(P_URL0, "");
    setDoubleParam(P_PollTime, 5.0);
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
    double poll_time = 5.0;
	char url0[256];
	std::string data;
    while(true)
	{
	    lock();
	    getStringParam(P_URL0, sizeof(url0), url0);
		if (strlen(url0) > 0)
		{
		    readURL(url0, data);
			setStringParam(P_Data0, data.c_str());
			callParamCallbacks();
		}
		getDoubleParam(P_PollTime, &poll_time);
		unlock();
		if (poll_time <= 0.0)
		{
		    poll_time = 5.0;
		}
		epicsThreadSleep(poll_time);
	}
}

void webgetDriver::readURL(const char* url, std::string& data)
{
	data.clear();
	CURL *curl = curl_easy_init();
    if(curl) 
	{
        CURLcode res;
        WriteCallbackData* cd = new WriteCallbackData(data);
        curl_easy_setopt(curl, CURLOPT_URL, url);
	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)cd);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = curl_easy_perform(curl);
//  if(res != CURLE_OK) {
//    fprintf(stderr, "curl_easy_perform() failed: %s\n",
//            curl_easy_strerror(res));
        curl_easy_cleanup(curl);
		delete cd;
	}
}

asynStatus webgetDriver::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason)
{
    return asynPortDriver::readOctet(pasynUser, value, maxChars, nActual, eomReason);
}


extern "C" {

	/// EPICS iocsh callable function to call constructor of lvDCOMInterface().
	/// The function is registered via lvDCOMRegister().
	///
	/// @param[in] portName @copydoc initArg0
	/// @param[in] configSection @copydoc initArg1
	/// @param[in] configFile @copydoc initArg2
	/// @param[in] host @copydoc initArg3
	/// @param[in] options @copydoc initArg4
	/// @param[in] progid @copydoc initArg5
	/// @param[in] username @copydoc initArg6
	/// @param[in] password @copydoc initArg7
	int webgetConfigure(const char *portName, const char* configSection, const char *configFile, const char *host, int options, 
		const char* progid, const char* username, const char* password)
	{
		try
		{
				new webgetDriver(portName);
				return(asynSuccess);

		}
		catch(const std::exception& ex)
		{
			errlogSevPrintf(errlogFatal, "lvDCOMConfigure failed: %s\n", ex.what());
			return(asynError);
		}
	}

	// EPICS iocsh shell commands 

	static const iocshArg initArg0 = { "portName", iocshArgString};			///< A name for the asyn driver instance we will create - used to refer to it from EPICS DB files
	static const iocshArg initArg1 = { "configSection", iocshArgString};	///< section name of \a configFile we will load settings from
	static const iocshArg initArg2 = { "configFile", iocshArgString};		///< Path to the XML input file to load configuration information from
	static const iocshArg initArg3 = { "host", iocshArgString};				///< host name where LabVIEW is running ("" for localhost) 
	static const iocshArg initArg4 = { "options", iocshArgInt};			    ///< options as per #lvDCOMOptions enum
	static const iocshArg initArg5 = { "progid", iocshArgString};			///< (optional) DCOM ProgID (required if connecting to a compiled LabVIEW application)
	static const iocshArg initArg6 = { "username", iocshArgString};			///< (optional) remote username for \a host
	static const iocshArg initArg7 = { "password", iocshArgString};			///< (optional) remote password for \a username on \a host

	static const iocshArg * const initArgs[] = { &initArg0,
		&initArg1,
		&initArg2,
		&initArg3,
		&initArg4,
		&initArg5,
		&initArg6,
		&initArg7 };

	static const iocshFuncDef initFuncDef = {"webgetConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

	static void initCallFunc(const iocshArgBuf *args)
	{
		webgetConfigure(args[0].sval, args[1].sval, args[2].sval, args[3].sval, args[4].ival, args[5].sval, args[6].sval, args[7].sval);
	}
	
	/// Register new commands with EPICS IOC shell
	static void webgetRegister(void)
	{
		iocshRegister(&initFuncDef, initCallFunc);
	}

	epicsExportRegistrar(webgetRegister);

}


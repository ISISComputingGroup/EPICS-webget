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

#if 0
/// Function to translate a Win32 structured exception into a standard C++ exception. 
/// This is registered via registerStructuredExceptionHandler()
static void seTransFunction(unsigned int u, EXCEPTION_POINTERS* pExp)
{
	throw Win32StructuredException(u, pExp);
}

/// Register a handler for Win32 strcutured exceptions. This needs to be done on a per thread basis.
static void registerStructuredExceptionHandler()
{
	_set_se_translator(seTransFunction);
}

template<typename T>
asynStatus lvDCOMDriver::writeValue(asynUser *pasynUser, const char* functionName, T value)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName = NULL;
	registerStructuredExceptionHandler();
	getParamName(function, &paramName);
	try
	{
		if (m_lvdcom == NULL)
		{
			throw std::runtime_error("m_lvdcom is NULL");
		}
		m_lvdcom->setLabviewValue(paramName, value);
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s: function=%d, name=%s, value=%s\n", 
			driverName, functionName, function, paramName, convertToString(value).c_str());
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
			"%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
			driverName, functionName, status, function, paramName, convertToString(value).c_str(), ex.what());
		return asynError;
	}
}

template<typename T>
asynStatus lvDCOMDriver::readValue(asynUser *pasynUser, const char* functionName, T* value)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName = NULL;
	registerStructuredExceptionHandler();
	getParamName(function, &paramName);
	try
	{
		if (m_lvdcom == NULL)
		{
			throw std::runtime_error("m_lvdcom is NULL");
		}
		m_lvdcom->getLabviewValue(paramName, value);
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s: function=%d, name=%s, value=%s\n", 
			driverName, functionName, function, paramName, convertToString(*value).c_str());
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
			"%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
			driverName, functionName, status, function, paramName, convertToString(*value).c_str(), ex.what());
		return asynError;
	}
}

template<typename T>
asynStatus lvDCOMDriver::readArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements, size_t *nIn)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName = NULL;
	registerStructuredExceptionHandler();
	getParamName(function, &paramName);

	try
	{
		if (m_lvdcom == NULL)
		{
			throw std::runtime_error("m_lvdcom is NULL");
		}
		m_lvdcom->getLabviewValue(paramName, value, nElements, *nIn);
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s: function=%d, name=%s\n", 
			driverName, functionName, function, paramName);
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		*nIn = 0;
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
			"%s:%s: status=%d, function=%d, name=%s, error=%s", 
			driverName, functionName, status, function, paramName, ex.what());
		return asynError;
	}
}

asynStatus lvDCOMDriver::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
	return writeValue(pasynUser, "writeFloat64", value);
}

asynStatus lvDCOMDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	return writeValue(pasynUser, "writeInt32", value);
}

asynStatus lvDCOMDriver::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
	return readArray(pasynUser, "readFloat64Array", value, nElements, nIn);
}

asynStatus lvDCOMDriver::readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
{
	return readArray(pasynUser, "readInt32Array", value, nElements, nIn);
}

asynStatus lvDCOMDriver::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
	return readValue(pasynUser, "readFloat64", value);
}

asynStatus lvDCOMDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
	return readValue(pasynUser, "readInt32", value);
}

asynStatus lvDCOMDriver::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason)
{
	int function = pasynUser->reason;
	int status=0;
	const char *functionName = "readOctet";
	const char *paramName = NULL;
	registerStructuredExceptionHandler();
	getParamName(function, &paramName);
	std::string value_s;
	try
	{
		if (m_lvdcom == NULL)
		{
			throw std::runtime_error("m_lvdcom is NULL");
		}
		m_lvdcom->getLabviewValue(paramName, &value_s);
		if ( value_s.size() > maxChars ) // did we read more than we have space for?
		{
			*nActual = maxChars;
			if (eomReason) { *eomReason = ASYN_EOM_CNT | ASYN_EOM_END; }
			asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
				"%s:%s: function=%d, name=%s, value=\"%s\" (TRUNCATED from %d chars)\n", 
				driverName, functionName, function, paramName, value_s.substr(0,*nActual).c_str(), value_s.size());
		}
		else
		{
			*nActual = value_s.size();
			if (eomReason) { *eomReason = ASYN_EOM_END; }
			asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
				"%s:%s: function=%d, name=%s, value=\"%s\"\n", 
				driverName, functionName, function, paramName, value_s.c_str());
		}
		strncpy(value, value_s.c_str(), maxChars); // maxChars  will NULL pad if possible, change to  *nActual  if we do not want this
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
			"%s:%s: status=%d, function=%d, name=%s, value=\"%s\", error=%s", 
			driverName, functionName, status, function, paramName, value_s.c_str(), ex.what());
		*nActual = 0;
		if (eomReason) { *eomReason = ASYN_EOM_END; }
		value[0] = '\0';
		return asynError;
	}
}

asynStatus lvDCOMDriver::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
	int function = pasynUser->reason;
	asynStatus status = asynSuccess;
	const char *paramName = NULL;
	registerStructuredExceptionHandler();
	getParamName(function, &paramName);
	const char* functionName = "writeOctet";
	std::string value_s(value, maxChars);
	try
	{
		if (m_lvdcom == NULL)
		{
			throw std::runtime_error("m_lvdcom is NULL");
		}
		m_lvdcom->setLabviewValue(paramName, value_s);
		asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
			"%s:%s: function=%d, name=%s, value=%s\n", 
			driverName, functionName, function, paramName, value_s.c_str());
		*nActual = value_s.size();
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
			"%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
			driverName, functionName, status, function, paramName, value_s.c_str(), ex.what());
		*nActual = 0;
		return asynError;
	}
}

/// EPICS driver report function for iocsh dbior command
void lvDCOMDriver::report(FILE* fp, int details)
{
//	fprintf(fp, "lvDCOM report\n");
	for(std::map<std::string,std::string>::const_iterator it=m_params.begin(); it != m_params.end(); ++it)
	{
		fprintf(fp, "Asyn param \"%s\" lvdcom type \"%s\"\n", it->first.c_str(), it->second.c_str());
	}
	if (m_lvdcom != NULL)
	{
		m_lvdcom->report(fp, details);
	}
	else
	{
		fprintf(fp, "DCOM pointer is NULL\n");
	}
	asynPortDriver::report(fp, details);
}

#endif

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
	const char *functionName = "lvDCOMDriver";
	CURL *curl = curl_easy_init();
    if(curl) {
   CURLcode res;
   curl_easy_setopt(curl, CURLOPT_URL, "http://www.isis.stfc.ac.uk");
   res = curl_easy_perform(curl);
   curl_easy_cleanup(curl);
   }

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


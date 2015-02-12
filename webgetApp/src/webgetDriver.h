/*************************************************************************\ 
* Copyright (c) 2013 Science and Technology Facilities Council (STFC), GB. 
* All rights reverved. 
* This file is distributed subject to a Software License Agreement found 
* in the file LICENSE.txt that is included with this distribution. 
\*************************************************************************/ 

/// @file lvDCOMDriver.h Header for #lvDCOMDriver class.
/// @author Freddie Akeroyd, STFC ISIS Facility, GB

#ifndef LVDCOMDRIVER_H
#define LVDCOMDRIVER_H

#include "asynPortDriver.h"

/// EPICS Asyn port driver class. 
class webgetDriver : public asynPortDriver 
{
public:
	webgetDriver(const char *portName);
#if 0
	// These are the methods that we override from asynPortDriver
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
	virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);
	virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);
	virtual asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
	virtual asynStatus readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);
	virtual asynStatus readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn);
	virtual void report(FILE* fp, int details);

private:
	lvDCOMInterface* m_lvdcom;
	std::map<std::string,std::string> m_params;

	template<typename T> asynStatus writeValue(asynUser *pasynUser, const char* functionName, T value);
	template<typename T> asynStatus readValue(asynUser *pasynUser, const char* functionName, T* value);
	template<typename T> asynStatus readArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements, size_t *nIn);

	static void lvDCOMTask(void* arg);
#endif
};

#endif /* LVDCOMDRIVER_H */

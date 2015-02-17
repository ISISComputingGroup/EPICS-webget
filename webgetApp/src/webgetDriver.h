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
	virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);
	virtual void report(FILE* fp, int details);

private:
	int P_PollTime; // double
    int P_URL0; // string
    int P_Data0; // string
	
	void readURL(const char* url, std::string& data);

	static void pollerTaskC(void* arg)
	{
	    webgetDriver* driver = static_cast<webgetDriver*>(arg);
		driver->pollerTask();	    
	}
	void pollerTask();
};

#define P_URL0String "URL0"
#define P_Data0String "DATA0"
#define P_PollTimeString "POLLTIME"

#endif /* LVDCOMDRIVER_H */

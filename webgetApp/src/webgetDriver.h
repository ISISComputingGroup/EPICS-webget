/*************************************************************************\ 
* Copyright (c) 2013 Science and Technology Facilities Council (STFC), GB. 
* All rights reverved. 
* This file is distributed subject to a Software License Agreement found 
* in the file LICENSE.txt that is included with this distribution. 
\*************************************************************************/ 

/// @file webgetDriver.h Header for #webgetDriver class.
/// @author Freddie Akeroyd, STFC ISIS Facility, GB

#ifndef WEBGETDRIVER_H
#define WEBGETDRIVER_H

/// EPICS Asyn port driver class. 
class epicsShareClass webgetDriver : public asynPortDriver 
{
public:
	webgetDriver(const char *portName, unsigned options);
	virtual asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);
	virtual void report(FILE* fp, int details);
    enum WebgetOptions { TidyWarnings = 0x1 };
	
private:

#define FIRST_WEBGET_PARAM P_PollTime

	int P_PollTime; // double
    int P_URL0; // string
    int P_Data0; // string
    int P_XPath0; // string

#define LAST_WEBGET_PARAM 	P_XPath0
#define NUM_WEBGET_PARAMS	(&LAST_WEBGET_PARAM - &FIRST_WEBGET_PARAM + 1)

	bool m_shutdown;
	unsigned m_options;
	
	bool checkOption(WebgetOptions option) { return (m_options & (unsigned)option) != 0; }
	
	void readURL(const char* url, std::string& data);
	static int tidyHTML2XHTML(const std::string& html_in, std::string& xhtml_out, bool warnings);
	static std::string runXPath(const std::string& xml_str, const std::string& xpath_str);

	static void pollerTaskC(void* arg)
	{
	    webgetDriver* driver = static_cast<webgetDriver*>(arg);
		driver->pollerTask();	    
	}
	void pollerTask();
	void processURL();
};

#define P_URL0String "URL0"
#define P_Data0String "DATA0"
#define P_XPath0String "XPATH0"
#define P_PollTimeString "POLLTIME"

#endif /* WEBGETDRIVER_H */

/** @file webPOSTRequest.cpp
 *  @ingroup asub_functions
 *
 * Post a mesage using CURL
 */
#include <string>
#include <iostream>

#include <registryFunction.h>
#include <aSubRecord.h>
#include <menuFtype.h>
#include <errlog.h>
#include <dbLock.h>
#include <recSup.h>
#include <epicsThread.h>
#include <callback.h>

// CURL
#include <curl/curl.h>
#include <curl/easy.h>

#include <epicsExport.h>

#include "webUtils.h"

// nmemb can be 0 and data may not be NULL terminated
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	aSubRecord *prec = (aSubRecord *)userdata;
	std::string message(ptr, nmemb * size);
	std::cerr << prec->name << ": Reply: " << message << std::endl;
	return nmemb;
}

/**
 *  Post a message using CURL
 *  @ingroup asub_functions
 *  @param[in] prec Pointer to aSub record
 *  if url says "test" the request is not posted
 */
static int webPOSTRequestThreadImp(aSubRecord* prec) 
{
	const char* url = getString(prec->a, prec->fta, prec->noa);
	const char* urlEncodedFormData = getString(prec->b, prec->ftb, prec->nob); /* from webFormURLEncode() */
	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
         errlogPrintf("%s curl init error", prec->name);
		 return 1;
	}
	if (url == NULL || urlEncodedFormData == NULL)
	{
         errlogPrintf("%s input args A or B are invalid", prec->name);
		 return 1;
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, urlEncodedFormData);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, prec);	
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	std::cerr << prec->name << ": POSTing \"" << urlEncodedFormData << "\" to " << url << std::endl;
	CURLcode res;
	if (!strcmp(url, "test"))
	{
	    res = curl_easy_perform(curl);
	}
	else
	{
	    std::cerr << prec->name << ": TESTING: ignoring url" << std::endl;
		res = CURLE_OK;
	}
	if (res != CURLE_OK)
	{
		errlogSevPrintf(errlogMajor, "%s curl_easy_perform() failed: %s\n", prec->name, curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl);
	return (res == CURLE_OK ? 0 : 1); /* only process output link on success */
}

typedef long (*rset_process_t)(dbCommon *);

/// post data and then notify epics when done 
static void webPOSTRequestThread(void* arg) 
{
	
	aSubRecord *prec =  (aSubRecord *)arg;
	dbCommon *pcomrec =  (dbCommon *)arg;
	int ret;
	try {
	    ret = webPOSTRequestThreadImp(prec);
	}
	catch(...) {
		errlogSevPrintf(errlogMajor, "%s webPOSTRequestThread failed\n", prec->name);
		ret = 1;
	}
    struct rset *prset =(struct rset *)(prec->rset);
    dbScanLock(pcomrec);
	prec->dpvt = reinterpret_cast<void*>(ret); // store return val in driver private area
    (*(rset_process_t)prset->process)(pcomrec); // needed until USE_TYPED_RSET available
    dbScanUnlock(pcomrec);
}

/// create thread to do asynchronous posting and return
/// data to post is in asub arg A, it has been urlencoded elsewhere
/// see sendAlert.db for calling details
static long webPOSTRequest(aSubRecord *prec) 
{
	if (prec->pact == 0)
	{
	    if (epicsThreadCreate("webPOSTRequest", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), webPOSTRequestThread, prec) != 0)
		{
			prec->pact = 1;
			return 0;
		}
		else
		{
		    errlogSevPrintf(errlogMajor, "%s epicsThreadCreate failed\n", prec->name);
	        return -1;
		}
	}
	else
	{
		if (prec->dpvt == 0) /* dpvt contains return status */
		{
			return 0;
		}
		else
		{
		    return -1;
		}
	}
}

extern "C" {
    epicsRegisterFunction(webPOSTRequest); /* must also be mentioned in dbd file */
}

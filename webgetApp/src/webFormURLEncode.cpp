/** @file webFormURLEncode.cpp
 *
 *  @ingroup asub_functions
 *
 *  create a urlEncoded string ready to send to a web server
 */
#include <string>
#include <cstring>
#include <sstream>

#include <registryFunction.h>
#include <aSubRecord.h>
#include <menuFtype.h>
#include <errlog.h>

// CURL
#include <curl/curl.h>
#include <curl/easy.h>

#include "webUtils.h"

#include <epicsExport.h>

/// encode a string as part of building a form urlencoded request
static std::string getEncodedString(CURL *curl, void* arg1, short type1, int len1, void* arg2, short type2, int len2)
{
	std::string s1 = getString(arg1, type1, len1);
	std::string s2 = getString(arg2, type2, len2);
	if (s1.size() == 0 || s2.size() == 0)
	{
		return "";
	}
	char* es1 = curl_easy_escape(curl, s1.c_str(), 0);
	char* es2 = curl_easy_escape(curl, s2.c_str(), 0);
	if (es1 == NULL || es2 == NULL)
	{
		return "";
	}
	std::ostringstream oss;
	if (strlen(es1) > 0)
	{
	    oss << es1 << "=" << es2;
	}
	curl_free(es1);
	curl_free(es2);
	return oss.str();
}

static void addEncodedString(CURL *curl, std::string& result, void* arg1, short type1, int len1, void* arg2, short type2, int len2)
{
    std::string item = getEncodedString(curl, arg1, type1, len1, arg2, type2, len2);
	if (item.size() > 0)
	{
		if (result.size() > 0)
		{
			result += "&";
		}
		result += item;
	}
}

/**
 *  URL encode a set of string ready for a form post request
 *  constructs name=value&name=value with appropriate escaping of characters
 *  @ingroup asub_functions
 *  @param[in] prec Pointer to aSub record
 *
 * args alternate, result if A=B&C=D etc.
 * "name" must always be a string, but value can be string or waveform
 * see sendAlert.db for further examples
 */
long webFormURLEncode(aSubRecord *prec)
{
    if (prec->ftva != menuFtypeCHAR)
	{
         errlogPrintf("%s incorrect output type. VALA (CHAR)", prec->name);
		 return -1;
	}
    std::string result;
	CURL *curl = curl_easy_init();
	if (curl == NULL)
	{
         errlogPrintf("%s curl init error", prec->name);
		 return -1;		
	}
	addEncodedString(curl, result, prec->a , prec->fta, prec->nea, prec->b, prec->ftb, prec->neb);
	addEncodedString(curl, result, prec->c , prec->ftc, prec->nec, prec->d, prec->ftd, prec->ned);
	addEncodedString(curl, result, prec->e , prec->fte, prec->nee, prec->f, prec->ftf, prec->nef);
	addEncodedString(curl, result, prec->g , prec->ftg, prec->neg, prec->h, prec->fth, prec->neh);
	addEncodedString(curl, result, prec->i , prec->fti, prec->nei, prec->j, prec->ftj, prec->nej);
	addEncodedString(curl, result, prec->k , prec->ftk, prec->nek, prec->l, prec->ftl, prec->nel);
	addEncodedString(curl, result, prec->m , prec->ftm, prec->nem, prec->n, prec->ftn, prec->nen);
	addEncodedString(curl, result, prec->o , prec->fto, prec->neo, prec->p, prec->ftp, prec->nep);
	curl_easy_cleanup(curl);
	strncpy((char*)prec->vala, result.c_str(), prec->nova);
	prec->neva = (result.size() < prec->nova ? result.size() + 1: prec->nova); // +1 to include NULL
    return 0; /* process output links */
}

extern "C" {
    epicsRegisterFunction(webFormURLEncode); /* must also be mentioned in asubFunctions.dbd */
}


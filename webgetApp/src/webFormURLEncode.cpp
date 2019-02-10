/** @file charToStringWaveform.c
 *  @author Freddie Akeroyd, STFC (freddie.akeroyd@stfc.ac.uk)
 *  @ingroup asub_functions
 *
 *  Copy a CHAR waveform record into a STRING waveform record. If this is done by
 *  a normal CAPUT the character byte codes are not preserved
 *
 *  It expect the A input to be the waveform data and B to be "NORD" (number of elements)
 *  it write its output to VALA
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

static std::string getEncodedString(CURL *curl, void* arg1, short type1, int len1, void* arg2, short type2, int len2)
{
	const char* s1 = getString(arg1, type1, len1);
	const char* s2 = getString(arg2, type2, len2);
	if (s1 == NULL || s2 == NULL)
	{
		return "";
	}
	char* es1 = curl_easy_escape(curl, s1, 0);
	char* es2 = curl_easy_escape(curl, s2, 0);
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
 */
static long webFormURLEncode(aSubRecord *prec) 
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
	addEncodedString(curl, result, prec->a , prec->fta, prec->noa, prec->b, prec->ftb, prec->nob);
	addEncodedString(curl, result, prec->c , prec->ftc, prec->noc, prec->d, prec->ftd, prec->nod);
	addEncodedString(curl, result, prec->e , prec->fte, prec->noe, prec->f, prec->ftf, prec->nof);
	addEncodedString(curl, result, prec->g , prec->ftg, prec->nog, prec->h, prec->fth, prec->noh);
	addEncodedString(curl, result, prec->i , prec->fti, prec->noi, prec->j, prec->ftj, prec->noj);
	addEncodedString(curl, result, prec->k , prec->ftk, prec->nok, prec->l, prec->ftl, prec->nol);
	addEncodedString(curl, result, prec->m , prec->ftm, prec->nom, prec->n, prec->ftn, prec->non);
	addEncodedString(curl, result, prec->o , prec->fto, prec->noo, prec->p, prec->ftp, prec->nop);
	curl_easy_cleanup(curl);
    char* str_out = (char*)prec->vala;
	strncpy((char*)prec->vala, result.c_str(), prec->nova);
	prec->neva = (result.size() < prec->nova ? result.size() : prec->nova);
    return 0; /* process output links */
}

extern "C" {
    epicsRegisterFunction(webFormURLEncode); /* must also be mentioned in asubFunctions.dbd */
}
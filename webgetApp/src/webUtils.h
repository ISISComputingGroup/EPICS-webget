#include <epicsString.h>

static std::string getString(void* arg, short type, int len)
{
    // use strnlen to avoid embedded nulls
    if (type == menuFtypeSTRING && len == 1)
    {
        size_t n = epicsStrnLen(*(epicsOldString*)arg, sizeof(epicsOldString));
        return std::string(*(epicsOldString*)arg, n);
    }
    else if (type == menuFtypeCHAR)
    {
        size_t n = epicsStrnLen((const char*)arg, len);
        return std::string((const char*)arg, n);
    }
    else
    {
        return "";
    }
}

extern long webFormURLEncode(aSubRecord *prec);
extern long webPOSTRequest(aSubRecord *prec);

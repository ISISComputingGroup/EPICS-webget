static std::string getString(void* arg, short type, int len)
{
    if (type == menuFtypeSTRING && len == 1)
    {
        return std::string(*(epicsOldString*)arg, sizeof(epicsOldString));
    }
    else if (type == menuFtypeCHAR)
    {
        return std::string((const char*)arg, len);
    }
    else
    {
        return "";
    }
}

extern long webFormURLEncode(aSubRecord *prec);
extern long webPOSTRequest(aSubRecord *prec);

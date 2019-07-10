
static const char* getString(void* arg, short type, int len)
{
    if (type == menuFtypeSTRING && len == 1)
    {
        return *(epicsOldString*)arg;
    }
    else if (type == menuFtypeCHAR)
    {
        return (const char*)arg;
    }
    else
    {
        return NULL;
    }
}

extern long webFormURLEncode(aSubRecord *prec);

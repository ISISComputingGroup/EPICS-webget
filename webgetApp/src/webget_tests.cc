#include <gtest/gtest.h>

#include <epicsString.h>
#include <epicsThread.h>
#include <aSubRecord.h>
#include <menuFtype.h>

#include "webUtils.h"

// setup asub record, assumes a,b,c etc fields are consecutive in structure
// so "b" can be accessed using pointer arithmetic from "a"  
static void setupStringArg(aSubRecord *prec, int index, const char* val)
{
    *(&(prec->a) + index * (&(prec->b) - &(prec->a))) = strdup(val);
	*(&(prec->fta) + index * (&(prec->ftb) - &(prec->fta))) = menuFtypeSTRING;
	*(&(prec->noa) + index * (&(prec->nob) - &(prec->noa))) = 1;
}

static void setupWaveformArg(aSubRecord *prec, int index, const char* val)
{
    *(&(prec->a) + index * (&(prec->b) - &(prec->a))) = strdup(val);
	*(&(prec->fta) + index * (&(prec->ftb) - &(prec->fta))) = menuFtypeCHAR;
	*(&(prec->noa) + index * (&(prec->nob) - &(prec->noa))) = strlen(val);
}

namespace {
    TEST(Webget, test_GIVEN_data_THEN_check_encoded_ok){
        // GIVEN
		aSubRecord rec;
		rec.ftva = menuFtypeCHAR;
		rec.nova = 256;
		rec.vala = new char[rec.nova];
		
        // WHEN
		setupStringArg(&rec, 0, "a");
		setupStringArg(&rec, 1, "b");
		setupStringArg(&rec, 2, "c");
		setupStringArg(&rec, 3, " ");
		
		int ret = webFormURLEncode(&rec);

        // THEN
		EXPECT_EQ(ret, 0);
		EXPECT_EQ(rec.neva, strlen((const char*)rec.vala) + 1);
        EXPECT_STREQ((const char*)rec.vala, "a=b&c=%20");
    }
    
    TEST(Webget, test_GIVEN_encoded_data_and_test_url_THEN_check_send_ok){
        // GIVEN
		aSubRecord rec;
        memset(&rec, 0, sizeof(rec));
		
        // WHEN
		setupWaveformArg(&rec, 0, "test");
		setupWaveformArg(&rec, 1, "a=b&c=%20");
		
		int ret = webPOSTRequest(&rec);
        while(rec.pact != 0)
        {
            epicsThreadSleep(0.1); // wait for background thread to finish
        }

        // THEN
		EXPECT_EQ(ret, 0);
        EXPECT_EQ((uintptr_t)rec.dpvt, 0); // background thread return status, 0 on success
    }

    TEST(Webget, test_GIVEN_encoded_data_and_silly_url_THEN_check_send_fails){
        // GIVEN
		aSubRecord rec;
        memset(&rec, 0, sizeof(rec));
		
        // WHEN
		setupWaveformArg(&rec, 0, "http://idonot.exist/dummy");
		setupWaveformArg(&rec, 1, "a=b&c=%20");
		
		int ret = webPOSTRequest(&rec);
        while(rec.pact != 0)
        {
            epicsThreadSleep(0.1); // wait for background thread to finish
        }

        // THEN
		EXPECT_EQ(ret, 0);
        EXPECT_NE((uintptr_t)rec.dpvt, 0); // background thread return status, non zero on error
    }

} // namespace

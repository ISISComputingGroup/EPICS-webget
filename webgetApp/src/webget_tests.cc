#include <gtest/gtest.h>

#include <epicsString.h>
#include <aSubRecord.h>
#include <menuFtype.h>

#include "webUtils.h"

// setup asub record, assumes a,b,c etc fields are consecutive in structure
// so "b" can be accessed using pointer arithmetic from "a"  
static void setupArg(aSubRecord *prec, int index, const char* val)
{
    *(&(prec->a) + index * (&(prec->b) - &(prec->a))) = strdup(val);
	*(&(prec->fta) + index * (&(prec->ftb) - &(prec->fta))) = menuFtypeSTRING;
	*(&(prec->noa) + index * (&(prec->nob) - &(prec->noa))) = 1;
}

namespace {
    TEST(Webget, test_GIVEN_data_THEN_check_encoded_ok){
        // GIVEN
		aSubRecord rec;
		rec.ftva = menuFtypeCHAR;
		rec.nova = 256;
		rec.vala = new char[rec.nova];
		
        // WHEN
		setupArg(&rec, 0, "a");
		setupArg(&rec, 1, "b");
		setupArg(&rec, 2, "c");
		setupArg(&rec, 3, " ");
		
		int ret = webFormURLEncode(&rec);

        // THEN
		EXPECT_EQ(ret, 0);
		EXPECT_EQ(rec.neva, strlen((const char*)rec.vala));
        EXPECT_STREQ((const char*)rec.vala, "a=b&c=%20");
    }
} // namespace

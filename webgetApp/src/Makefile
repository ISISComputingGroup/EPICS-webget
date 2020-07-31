TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += webget

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install webget.dbd into <top>/dbd
DBD += webget.dbd

ifeq ($(STATIC_BUILD),YES)
USR_CPPFLAGS += -DCURL_STATICLIB 
USE_CPPFLAGS_WIN32 += -DTIDY_EXPORT=extern
endif

ifeq ($(SHARED_LIBRARIES),YES)
USR_CPPFLAGS_WIN32 += -DTIDY_EXPORT=__declspec(dllimport)
endif

# specify all source files to be compiled and added to the library
webget_SRCS += webgetDriver.cpp webFormURLEncode.cpp webPOSTRequest.cpp

webget_LIBS += asyn htmltidy pugixml
webget_LIBS_WIN32 += libcurl
webget_SYS_LIBS_Linux += curl
webget_LIBS += $(EPICS_BASE_IOC_LIBS)

# googleTest Runner

ifeq ($(findstring 10.0,$(VCVERSION)),)

GTESTPROD_HOST += runner
runner_SRCS += webget_tests.cpp webFormURLEncode.cpp webPOSTRequest.cpp
runner_LIBS += $(EPICS_BASE_IOC_LIBS)
runner_LIBS_WIN32 += libcurl zlib
runner_SYS_LIBS_Linux += curl
runner_SYS_LIBS_WIN32 += wldap32 crypt32 Normaliz

endif

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


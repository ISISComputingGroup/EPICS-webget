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

# specify all source files to be compiled and added to the library
webget_SRCS += webgetDriver.cpp

webget_LIBS += asyn libcurl
webget_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


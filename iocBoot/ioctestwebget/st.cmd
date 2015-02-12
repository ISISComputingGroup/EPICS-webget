#!../../bin/win32-x86/testwebget

## You may have to change testwebget to something else
## everywhere it appears in this file

#< envPaths

## Register all support components
dbLoadDatabase("../../dbd/testwebget.dbd",0,0)
testwebget_registerRecordDeviceDriver(pdbbase) 

## Load record instances
dbLoadRecords("../../db/testwebget.db","user=faa59")

iocInit()

## Start any sequence programs
#seq snctestwebget,"user=faa59"

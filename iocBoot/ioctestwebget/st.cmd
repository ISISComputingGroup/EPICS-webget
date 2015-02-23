#!../../bin/win32-x86/testwebget

## You may have to change testwebget to something else
## everywhere it appears in this file

< envPaths

## Register all support components
dbLoadDatabase("../../dbd/testwebget.dbd",0,0)
testwebget_registerRecordDeviceDriver(pdbbase) 

## port, options - pass 0x1 as option to see html tidy warnings
#webgetConfigure("T1", 1)
webgetConfigure("T1")

## Load record instances
dbLoadRecords("../../db/testwebget.db","P=$(MYPVPREFIX),PORT=T1")

iocInit()

## Start any sequence programs
#seq snctestwebget,"user=faa59"

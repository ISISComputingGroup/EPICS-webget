#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))

define DIR_template
 $(1)_DEPEND_DIRS = configure
endef
$(foreach dir, $(filter-out configure,$(DIRS)),$(eval $(call DIR_template,$(dir))))

iocBoot_DEPEND_DIRS += $(filter %App,$(DIRS))
testwebgetApp_DEPEND_DIRS += webgetApp

TEST_RUNNER = $(TOP)/webgetApp/src/O.$(EPICS_HOST_ARCH)/runner

# Add any additional dependency rules here:

include $(TOP)/configure/RULES_TOP

.PHONY: test
test:
ifneq ($(wildcard $(TEST_RUNNER)*),)
	$(TEST_RUNNER) --gtest_output=xml:$(TOP)/test-reports/TEST-Webget.xml
endif

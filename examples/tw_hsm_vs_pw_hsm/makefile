PRJ_ROOT := ../../
PROGRAMS := variant_state_machine tw_state_machine

include ../common.mk

#-------------------------------------------------------------------------------

TEST_OUTPUTS := $(foreach p,$(PROGRAMS),$(p).out.txt)

#-------------------------------------------------------------------------------

.PHONY: test
test: $(TEST_OUTPUTS)
	diff $^ && printf "\033[1;32mPASSED\033[0;39m"
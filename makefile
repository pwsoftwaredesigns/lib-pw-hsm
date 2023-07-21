CXXFLAGS := -Os -fno-rtti -std=c++17 -fdata-sections -ffunction-sections -fno-exceptions
MAP_TO_CSV := map_to_csv.exe

EXTENSIONS := map size.txt map.csv out.txt
PROGRAMS := variant_state_machine tw_state_machine
INCLUDE_PATHS += etl/include

#-------------------------------------------------------------------------------

OUTPUTS += $(PROGRAMS)
OUTPUTS += $(foreach p,$(PROGRAMS),$(foreach e,$(EXTENSIONS), $(p).$(e)))
TEST_OUTPUTS := $(foreach p,$(PROGRAMS),$(p).out.txt)
INCLUDES := $(foreach i,$(INCLUDE_PATHS),-I$(i))

#-------------------------------------------------------------------------------

.PHONY: all
all: $(OUTPUTS)

#In order to get wildcard rules to work
$(OUTPUTS):

#Rule to compile a .cpp file into an executable
%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -Xlinker -Map=$@.map $^

#Rule to make .map files depend on executables	
%.map: %

#Rule to output the executable size to a file
%.size.txt: %
	size.exe $^ | tee $@

#Rule to convert the .map file into a .csv file	
%.map.csv: %.map
	$(MAP_TO_CSV) $^ $@

#Rule to run the executable and save its output to an .out.txt file		
%.out.txt: %
	./$^ > $@
	
.PHONY: clean
clean:
	rm -f $(OUTPUTS)
	
.PHONY: test
test: $(TEST_OUTPUTS)
	diff $^ && printf "\033[1;32mPASSED\033[0;39m"
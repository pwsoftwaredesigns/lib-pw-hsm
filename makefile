CXXFLAGS := -Os -fno-rtti -std=c++17 -fdata-sections -ffunction-sections -fno-exceptions
MAP_TO_CSV := map_to_csv.exe

EXTENSIONS := map size.txt map.csv out.txt
PROGRAMS := variant_state_machine tw_state_machine

OUTPUTS := $(foreach p,$(PROGRAMS),$(foreach e,$(EXTENSIONS), $(p).$(e)))

.PHONY: all
all: $(OUTPUTS)

$(OUTPUTS):

%: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -Xlinker -Map=$@.map $^
	
%.map: %

%.size.txt: %
	size.exe $^ > $@
	
%.map.csv: %.map
	$(MAP_TO_CSV) $^ $@
		
%.out.txt: %
	./$^ > $@
	
.PHONY: clean
clean:
	rm -f $(OUTPUTS)
	
.PHONY: test
test: variant_state_machine.txt tw_state_machine.txt
	diff $^ && echo "PASSED"
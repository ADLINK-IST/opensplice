
all link: sources

.PHONY: code_clean
code_clean:
	@rm -rf code
 
clean: code_clean
 
DDSI2E_SOURCES := $(wildcard $(addprefix ../ddsi2e/code/, *.[ch] *.template *.odl)) $(wildcard $(addprefix ../ddsi2e/core/, *.[ch] *.template *.odl))

.PHONY: sources
sources: code/.STAMP

code/.STAMP: ../ddsi2e/sanitize.sh ../ddsi2e/sanitize.pl $(DDSI2E_SOURCES)
	../ddsi2e/sanitize.sh ../ddsi2e/sanitize.pl ../ddsi2e/code ../ddsi2e/core code
	touch $@




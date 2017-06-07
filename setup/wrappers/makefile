ifneq (,$(or $(findstring win32,$(SPLICE_HOST)),$(findstring win64,$(SPLICE_HOST))))
all link clean:
	@$(MAKE) -C wincmd $@
	@$(MAKE) -C winln $@
else
all link clean:
endif

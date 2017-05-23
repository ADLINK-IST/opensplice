# Copy OpenSplice Streams IDL to $(OSPL_HOME)/etc/

# Source
STREAMS_IDL := streams_dcps.idl
SRC_PATH		:= $(OSPL_HOME)/src/api/streams/idl

# Destination
DEST_PATH := $(OSPL_HOME)/etc/idl
DEST_IDL := $(DEST_PATH)/$(STREAMS_IDL)

all link: $(DEST_IDL)

$(DEST_IDL): $(SRC_PATH)/$(STREAMS_IDL)
	cp $? $(DEST_PATH)

clean:
	rm -f $(DEST_IDL)


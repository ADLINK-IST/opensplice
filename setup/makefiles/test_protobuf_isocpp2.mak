# default values for directory and idl-files to process
PROTOC_DIR     ?= ../../code
PROTOC_OUTPUT  ?= $(PROTOC_DIR)
#PROTOC_FILES   ?= $(shell find $(PROTOC_DIR) -name '*.proto')
PROTOC_FILES   ?= $(notdir $(wildcard $(IDL_DIR)/*.proto))
vpath %.proto     $(PROTOC_DIR)

# protoc compiler settings and files.
ifeq (,$(or $(findstring win32,$(SPLICE_TARGET)), $(findstring win64,$(SPLICE_TARGET)), $(findstring wince,$(SPLICE_TARGET))))
  PROTOC_FLAGS  = --cpp_out=$(PROTOC_OUTPUT)
  PROTOC_FLAGS += --ddscpp_out=$(PROTOC_OUTPUT)
  PROTOC_FLAGS += --proto_path=$(PROTOC_OUTPUT)
  PROTOC_FLAGS += --proto_path=$(PROTOC_DIR)
  PROTOC_FLAGS += --proto_path=$(PROTOBUF_HOME)/src
  PROTOC_FLAGS += --proto_path=$(OSPL_HOME)/src/tools/protobuf/protos
else
  PROTOC_FLAGS  = --cpp_out='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOC_OUTPUT))'
  PROTOC_FLAGS += --ddscpp_out='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOC_OUTPUT))'
  PROTOC_FLAGS += --proto_path='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOC_OUTPUT))'
  PROTOC_FLAGS += --proto_path='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOC_DIR))'
  PROTOC_FLAGS += --proto_path='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(PROTOBUF_HOME)/src)'
  PROTOC_FLAGS += --proto_path='$(shell $(OSPL_HOME)/bin/ospl_normalizePath $(OSPL_HOME)/src/tools/protobuf/protos)'
endif
PROTOC_FILES += $(OSPL_HOME)/src/tools/protobuf/protos/omg/dds/descriptor.proto
PROTOC_FILES += $(PROTOBUF_HOME)/src/google/protobuf/descriptor.proto
PROTOC_CPP   += google/protobuf/descriptor.pb.cc
PROTOC_CPP   += omg/dds/descriptor.pb.cc
PROTOC_CPP   += ospl_protobuf_common.cpp
PROTOC_CPP   += ospl_protobuf_commonSplDcps.cpp

# protoc command
PROTOC  = $(WINCMD) protoc

# protoc output
PROTOC_HDR     = $(PROTOC_FILES:%.proto=%.h) $(PROTOC_FILES:%.proto=%_DCPS.hpp) $(PROTOC_FILES:%.proto=%SplDcps.h)
PROTOC_OBJ_TMP = $(PROTOC_CPP:%.cc=%$(OBJ_POSTFIX))
PROTOC_OBJ     = $(PROTOC_OBJ_TMP:%.cpp=%$(OBJ_POSTFIX))
PROTOC_OBJ    += ospl_protobuf_common$(OBJ_POSTFIX)
PROTOC_OBJ    += ospl_protobuf_commonSplDcps$(OBJ_POSTFIX)

# This determines what/how it will be processed
# PROTO_H will be generated before the actual compile  (may even include C-file like ..SplLoad.c)
# PROTO_O will be linked into the final target
PROTO_H       = $(PROTOC_HDR)
PROTO_C       = $(PROTOC_CPP)
PROTO_O       = $(PROTOC_OBJ)

# This cpp file doesn't have a specific related proto file.
# Having these empty rules makes sure that make knows about these
# files and will replace these empty rules with the implicit ones.
ospl_protobuf_common.cpp:
ospl_protobuf_commonSplDcps.cpp:

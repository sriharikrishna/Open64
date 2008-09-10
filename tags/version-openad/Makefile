#############################################################################

# WARNING: exporting these causes problems with the Open64 build system
#export CC
#export CXX
#export CFLAGS
#export CXXFLAGS

#############################################################################

ifndef OPEN64ROOT
  $(error "Error: OPEN64ROOT not set!")
endif 

all: open64_fe_build open64_be_build open64_tools_build

open64_fe_build: 
	cd $(OPEN64ROOT)/crayf90/sgi && $(MAKE) 

open64_be_build: 
	cd $(OPEN64ROOT)/whirl2f && $(MAKE)

open64_tools_build: 
	cd $(OPEN64ROOT)/ir_tools && $(MAKE)


clean: 
	cd $(OPEN64ROOT) && $(MAKE) clean 
	cd osprey1.0/tests && $(MAKE) clean

veryclean: clean
	cd $(OPEN64ROOT) && $(MAKE) clobber 

.PHONY : open64_fe_build open64_be_build open64_tools_build clean veryclean 

include $(OSPL_HOME)/setup/x86.win32-default.mak

EXES=ServiceUtil.exe ServiceUtil_DEBUG.exe
CLEAN=$(EXES) ServiceUtil.pdb ServiceUtil_DEBUG.pdb

.PHONY:
ifeq (,$(findstring win32,$(SPLICE_TARGET)))
all:
clean:
else
all: $(EXES)
clean: $(CLEAN)
	@rm $(CLEAN)
endif

ServiceUtil.exe: ServiceUtil.cs
	$(CSC) -noconfig -nowarn:1701,1702 -errorreport:prompt -warn:4 -define:DEBUG\;TRACE -main:SvcInstaller.ServiceUtil -debug+ -debug:full -optimize- -out:$@ -target:exe $^

ServiceUtil_DEBUG.exe: ServiceUtil.cs
	$(CSC) -noconfig -nowarn:1701,1702 -errorreport:prompt -warn:4 -define:TRACE -main:SvcInstaller.ServiceUtil -debug:pdbonly -optimize+ -out:$@ -target:exe $^


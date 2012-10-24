ifeq ("$(VS_VER)","14")
VS_INCLUDE = -I"$(VS_HOME)/VC/INCLUDE" -I"$(VS_HOME)/VC/PlatformSDK/include"
VS_LIBS = -L"$(VS_HOME)/VC/lib" -L"$(VS_HOME)/VC/PlatformSDK/lib"
endif

ifeq ("$(VS_VER)","15")
VS_INCLUDE += -I"$(VS_HOME)/VC/include" -I"$(VS_HOME)/../Microsoft SDKs/Windows/v6.0A/Include"  -I"$(WINDOWSSDKDIR)/Include"
VS_LIBS = -L"$(VS_HOME)/VC/lib" -L"$(VS_HOME)/../Microsoft SDKs/Windows/v6.0A/lib"  -L"$(WINDOWSSDKDIR)/lib"
endif

$(OSPL_HOME)/bin/ospl_winln.exe: ospl_winln.exe
	cp `pwd`/ospl_winln.exe $(OSPL_HOME)/bin/ospl_winln.exe

ospl_winln.exe: ospl_winln.c
	$(OSPL_HOME)/bin/ospl_wincmd cl -DWIN32 -nologo -TC -Z7 -Od -W3 $(VS_INCLUDE) -c ospl_winln.c
	$(OSPL_HOME)/bin/ospl_wincmd link -incremental:no -machine:IX86 -subsystem:console -NODEFAULTLIB:MSVCRT $(VS_LIBS) ospl_winln.obj -lkernel32 -o ospl_winln.exe

clean:
	rm -f $(OSPL_HOME)/bin/ospl_winln.exe
	rm -f ospl_winln.exe

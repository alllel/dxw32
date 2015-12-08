SOURCES=$(wildcard *.cpp)

DEPS=$(patsubst %.cpp,%.d,$(SOURCES))
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES))

MINGW=i686-w64-mingw32

CXX=$(MINGW)-g++-win32
WINDRES=$(MINGW)-windres
DXW32LIBS=-lwinmm -lgdi32 -llz32 -lcomdlg32
LDFLAGS=-mwindows -static-libgcc -static-libstdc++
CXXFLAGS=-Wall -O3


all: dxw32.exe

include $(DEPS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

%.d: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $< -MF $@

%.o: %.rc
	$(WINDRES) $< $@

dxw.o: dxw_rc.h

dxw32.exe: $(OBJECTS) dxw.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(DXW32LIBS)

clean:
	@rm dxw32.exe *.o

distclean: clean
	@rm *.d


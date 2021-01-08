TARGET   := spcinject

CXXFLAGS := -march=native -std=gnu++20 -O2 -s
EXEFLAGS := -fwhole-program
DLLFLAGS := -shared -L "C:\\Program Files (x86)\\BH\\SPCImage" -l base

CXX      := i686-w64-mingw32-c++

.PHONY: all clean

all: $(TARGET).exe $(TARGET).dll

clean:
	@rm -f $(TARGET).exe $(TARGET).dll

$(TARGET).exe: src/main.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(EXEFLAGS)

$(TARGET).dll: dll/main.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(DLLFLAGS)

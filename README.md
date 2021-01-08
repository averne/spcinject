# SPCInject

SPCImage (FLIM analysis software) doesn't support easily exporting calculated lifetime/amplitudes values (who would need that, right).

This is a little tool that use DLL injection to either query their (proprietary) library for those values, or reads them from memory.

## Building

Requires mingw32. Run `make`.

## Running

Start spcinject.exe while SPCImage is running. Make sure spcinject.dll is in the same directory.

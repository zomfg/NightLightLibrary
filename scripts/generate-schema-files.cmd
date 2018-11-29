REM pre-build script

REM cd to $(ProjectDir)
cd  %1

REM compile .bond file to c++
REM call $(VcpkgRoot)tools\gbc.exe
%2 c++ --apply=compact  nightlight_schema.bond

REM prepend stdafx.h to bond schema .cpp files
(echo #include "stdafx.h") > bond.cpp.tmp
type nightlight_schema_apply.cpp >> bond.cpp.tmp
move /y bond.cpp.tmp nightlight_schema_apply.cpp

(echo #include "stdafx.h") > bond.cpp.tmp
type nightlight_schema_types.cpp >> bond.cpp.tmp
move /y bond.cpp.tmp nightlight_schema_types.cpp

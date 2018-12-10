REM post-build script

REM cd to $(ProjectDir)
cd  %1

echo // file generated automatically, you are probably looking for NightLight class at the bottom > NightLightLibrary.h
git log --pretty=format:"// Revision : %%h %%ad%%x0a" -n 1 >> NightLightLibrary.h
set remote=git remote get-url origin
FOR /F "tokens=*" %%g IN ('git remote get-url origin') do (SET VAR=%%g)
echo // %VAR% >> NightLightLibrary.h
echo // ----------------------------------------------------------------------------------------- >> NightLightLibrary.h
echo. >> NightLightLibrary.h
echo #ifndef INCLUDE_BOND >> NightLightLibrary.h
echo. >> NightLightLibrary.h
(type NightLightWrapper.h) >> NightLightLibrary.h
echo. >> NightLightLibrary.h
echo #else // INCLUDE_BOND >> NightLightLibrary.h
echo. >> NightLightLibrary.h
(type *schema_types.h) >> NightLightLibrary.h
echo. >> NightLightLibrary.h

(echo #pragma region Registry.h) >>  NightLightLibrary.h
(type Registry.h | find /V "#pragma once") >> NightLightLibrary.h
(echo #pragma endregion Registry.h) >>  NightLightLibrary.h
echo. >> NightLightLibrary.h

(echo #pragma region Settings.h) >>  NightLightLibrary.h
(type Settings.h | find /V "#pragma once" | find /V "#include") >> NightLightLibrary.h
(echo #pragma endregion Settings.h) >>  NightLightLibrary.h
echo. >> NightLightLibrary.h

(echo #pragma region State.h) >>  NightLightLibrary.h
(type State.h | find /V "#pragma once" | find /V "#include") >> NightLightLibrary.h
(echo #pragma endregion State.h) >>  NightLightLibrary.h
echo. >> NightLightLibrary.h

(echo #pragma region NightLight.h) >>  NightLightLibrary.h
(type NightLight.h | find /V "#pragma once" | find /V "#include") >> NightLightLibrary.h
(echo #pragma endregion NightLight.h) >>  NightLightLibrary.h
echo. >> NightLightLibrary.h

echo #endif // INCLUDE_BOND >> NightLightLibrary.h

REM copy to $(OutDir)
copy NightLightLibrary.h %2

REM post-build script

REM cd to $(ProjectDir)
cd  %1

echo // Windows Night Light wrapper > NightLightLibrary.h
git log --pretty=format:"// Revision : %%h %%ad%%x0a" -n 1 >> NightLightLibrary.h
set remote=git remote get-url origin
FOR /F "tokens=*" %%g IN ('git remote get-url origin') do (SET VAR=%%g)
echo // %VAR% >> NightLightLibrary.h
echo // ----------------------------------------------------------------------------------------- >> NightLightLibrary.h
echo. >> NightLightLibrary.h

(type NightLightWrapper.h) >> NightLightLibrary.h

REM copy to $(OutDir)
copy NightLightLibrary.h %2

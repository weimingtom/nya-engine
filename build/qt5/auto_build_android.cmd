set NYA_PRO=%cd%\nya_engine.pro
rmdir ..\..\bin\build-nya_engine-Android-armeabi-v7a-auto /s /q
md ..\..\bin\build-nya_engine-Android-armeabi-v7a-auto
pushd ..\..\bin\build-nya_engine-Android-armeabi-v7a-auto
call %QMAKE_ANDROID% -spec android-g++ %NYA_PRO%
call %PLATFORM_MAKE% --jobs
popd

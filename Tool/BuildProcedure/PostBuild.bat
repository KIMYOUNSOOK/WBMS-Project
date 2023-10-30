:: Post-Build Procedure for TSW project ::
@echo off

echo^ ::::::::::: 1.Copy output hex and elf files from Debug folder :::::::::::
copy  /y *.hex ..\Tool\T32\
copy  /y *.elf ..\Tool\T32\
exit
@echo off
dir %1 |find "%1" > resnul.txt
for /f "tokens=3" %%i in (resnul.txt) do set SIZE=%%i
echo size:%SIZE%
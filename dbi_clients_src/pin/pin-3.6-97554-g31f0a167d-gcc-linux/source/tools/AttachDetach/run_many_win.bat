@echo off
for /l %%x in (1, 1, 145) do (
	echo Welcome %%x times
	echo Line to run: %*
	%* || goto error
)
:error
if %errorlevel% neq 0 echo Command terminated with error %errorlevel%
exit /b %errorlevel%
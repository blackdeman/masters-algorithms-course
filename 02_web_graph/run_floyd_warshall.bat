@echo off
setlocal enabledelayedexpansion
set /a counter=0
for /r %%i in ("graphs_from_seed_2000_scale_12\output*.txt") do (
	set /a counter+=1
	echo !counter!
	echo !time!
	REM set output=%%i.fw_results
	echo %%~ni
	echo Test %%~ni >> graphs_from_seed_2000_scale_12_results.txt
	call blocked-fw-cuda.exe < %%i >> graphs_from_seed_2000_scale_12_results.txt
)
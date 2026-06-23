cd C:\Users\Mitch\Documents\GitHub\emsdk
call emsdk_env.bat
cd C:\Users\Mitch\Documents\GitHub\MitchMitch42-second-movement
REM rmdir build-sim /S /Q
call emmake make BOARD=sensorwatch_pro DISPLAY=classic
call "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" http://localhost:8000/firmware.html
call python3 -m http.server -d build-sim

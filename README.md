ResourceAccounting
==================

Resource accounting project Pi and android platform and kernel code changes
This is Userspace repository of ResourceAccouting project
Currently the project is divided into various small repositories on different machines.
List of projects:
1. Android Source Code: Framework level changes to Android source to find out applications requesting for sensor reads to 
  SensorService. Its difficult to take backup of entire Android source so I will take backup of only modified files and README to 
  explain source location.
2. Samsung Linux Kernel: Added device_event_log module to input subsystem. The module records event timestamps
3. Pi Linux Kernel: deveice_event_log for tty subsystem. Records eventlog for gps
4. Gpsd: We have modified the gpsd source code so that it can communicate with our resource accounting deamon. On the similar line
  of our modifications in SensorService in Android.

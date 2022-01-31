# sysmonitor
Simple system monitor utility for Linux written in C

<br>Usage:</br> ./sysmonitor [-s | -u | -g | -samples= | -tdelay=]

<br>Flags:</br>
  -system: Shows only system usage information
  -users: Shows only connected users
  -samples=: Specify the number of samples to be taken
  -tdelay=: The number of seconds between each sample
  
<br>Example usage:</br>
  - ./sysmonitor -g  ... Monitor with graphical representation
  - ./sysmonitor -samples=100 -tdelay=5 ... Take 100 samples with a delay of 5 seconds
  - ./sysmonitor 100 5 ... Take 100 samples with a delay of 5 seconds
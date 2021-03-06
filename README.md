# sysmonitor
Simple system monitor utility for Linux written in C

## Usage:
  ./sysmonitor [-s | -u | -g | -samples= | -tdelay=]

## Flags:
  * -graphics or -g: Displays a graphical representation of data
  * -system or -s: Shows only system usage information
  * -users or -u: Shows only connected users
  * -samples=: Specify the number of samples to be taken
  * -tdelay=: The number of seconds between each sample
  
## Example usage: 
  * ./sysmonitor -g  ... Monitor with graphical representation
  * ./sysmonitor -samples=100 -tdelay=5 ... Take 100 samples with a delay of 5 seconds
  * ./sysmonitor 100 5 ... Take 100 samples with a delay of 5 seconds

# CustomerCounter
Repo for Arduino Customer Counter Project 

Available in two flavours: WiFi/non-WiFi

The non-WiFi version currently compiles for Arduino MKR WiFi 1010, although it 
should be able to run perfectly well on most boards.

The WiFi version requires an arduino_secrets.h file creating in the directory containing the following:

#define SECRET_SSID "YourNetworkHere"
#define SECRET_PASS "YourPasswordHere"


# Simple alternative ESP8266 or ESP8265 firmware for Sonoff Basic switches

Note that this firmware does:

* Support OTA Updates using an firmware included password
* Includes wireless network information in the firmware image
* Does not perform any authentication on the webpages

This means that anybody who can extract firmware from one of the
deployed devices has access to wireless or OTA passwords. This also
means that changing these passwords requires a change to the firmware
and it means that 802.1x EAP-TLS is not supported by this sketch.

Of course it also means that anybody having access to the network
can toggle the switch (and cause potential damage)

This is one of the most basic possible siwtches. For production
environments one should consider using authentication on web interfaces,
move wireless configuration and flash passwords to SPIFFS - and
possibly add MQTT support (and drop HTTP support entirely)

There are 3 fields inside the sketch that have to be set before
flashing:

* ```{FILLIN_WIFI_SSID}``` is the SSID of the WiFi to connect to
* ```{FILLIN_WIFI_PASSWORD}``` is the preshared key for WPA-PSK
* ```{FILLIN_OTAPASSWORD}``` is the OTA password to be used
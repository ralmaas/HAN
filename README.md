HAN
===
Made a small project to read my energy usage by fetching data from the
HAN-connector on the electricity meter set up by my local provider, Hafslund.

The solution is based on a small board from AliExpress for interfacing with the HAN and a 
Wemos D1 mini to handle the serial data and submitting it for further use on MQTT

The AliExpress board is using a Texas Instrument TS721A Meter-Bus Tranceiver.

https://www.aliexpress.com/item/32771019981.html

Have added a complete schematics of the setup in the picture folder.

The Wemos D1 Mini is processing the data and will submitt MQTT data at the following intervals:

3 seconds

10 seconds

Hourly

Thanks to a lot of makers for source-code and inspiration:
<ul>
<li>steinjo
<li>roarf
</ul>

<a href="https://github.com/ralmaas/HAN/wiki">More information on the Wiki-page</a>

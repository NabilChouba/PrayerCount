-    Bluetooth gadget interacting with Android smartphone to help during Prayer

The solution uses an Android phone + Special Bluetooth kit
  - The embedded kit uses a 3D sensor to detect a change phase in prayer
  - Send information to the Android phone.
  - The GSM display : number of raka'at + the status of current raka'at
  - The Bluetooth kit should be wearied during the prayer  and put the phone at ground in front of you

I share code source for Android app & embedded C source code.

hardware and app in action on youtube
https://www.youtube.com/watch?v=zmb9fmdiMRg

full documentation on :
https://sites.google.com/site/nabilchouba2/home/bluetoothlowpower

It's good idea to control every think with your smartphone or ipad like gadget.
Smartphone have Powerful processor, CooL User Interface, many sensor ...
Bluetooth is the only LowPower solution available until now to wireless interact with GSM.

At beginning for the year 2011, Ti has made available the LowPower Bleutoth chip : CC2560
they also provide the started kit EZ430-RF2560,
At that time I find it the best solution to do some funny project
that include smartphone as user interface or as panel control for an embedded gadget.




The proposed project is :
During the Islamic Prayer people often forget the status of there Prayer.
The Islamic Prayer is composed from those steps :

The solution is to wear an gadget that will detect a change phase in prayer and by this way know that is your status.

as the kit EZ430-RF2560 has already a 3D sensor, no other hardware is needed ;)

We only need to detect the transition between every movement, and implement the C algorithm in the msp430.

Also we need to develop the android application that will get the change status information from the Bluetooth kit,
And show it on the smartphone screen with some information processing ;)

The smartphone is should be putted on the ground that you can see it during the Prayer.



The Demo of the final solution  with all source code :
The blue light in the video is the kit EZ430-RF2560 that I m wiring, for the purpose of the demo :
The phone is in front of webcam but it should be in front of me that I can see the status.

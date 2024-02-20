# Code provided by André Saischowa (and Mike Brüstle)

This is code for doing (nearly) serious things with your Transputer(s).
André ported all needed INMOS tools like iserver to run all the available development tools from back in the days (OCCAM, C, etc).  
He also ported the [Helios](https://www.geekdot.com/helios-the-transputer-os/) server, i.e. the software which runs on the host (i.e. your ATARI) and communicates with the Helios Kernel(s) running on your insane Transputer Farm!
This is a good 75% of what the ATW800 offered – the missing 25% are the graphics which ran on the Blossom chip and was only accessible by the Transputer.

* AUTO – the TBIOS driver and stuff needed during ATARI bootup
* BIN – the INMOS tools like `iserver` as well as the always-needed ispy utility and the even better `RSPY` tool created by Mike Brüstle.
* D72UNI.zip – contains the transputer hosted compiler environment based on d7205a (OCCAM) and d7214c (C-Compiler). [Visit transputer.net for plenty of documentation](https://www.geekdot.com/stgatw-programming-and-software/#programs:~:text=AUTO%20%E2%80%93%20the%20TBIOS,these%20steps.) on those. See the README in that folder.
* HELIOS11.zip – well, that’s the Helios v1.1 distribution. It’s way smaller than the v1.3 and good for an initial try. You can later switch to v1.3.1 following [these steps](https://www.geekdot.com/installation/).
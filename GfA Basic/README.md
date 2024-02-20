# Basic Testing

Yes, literally, we’re testing if your Transputer is working correctly using a BASIC program called T_TEST.GFA – i.e. it’s GfA Basic in this case. But in essence it’s nearly the same code used for [my C64 or Apple II interfaces](https://www.geekdot.com/1st-code-example/).  

This little Program checks if it can find a link-interface, a Transputer and if so, which kind (16 or 32 bit). If that went OK, it does a little coms-speed test by reading 4KB from the Transputer and times that.
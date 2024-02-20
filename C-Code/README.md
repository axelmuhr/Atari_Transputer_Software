# C code examples

There are two Mandelbrot fractal examples available

## TMANDEL.PRG

the evil, dirty, down-to-the-metal, direct-to-screen-writing version.
This is good for getting an idea of how fast data is being pushed to the Atari ST without much handling overhead.
As this writes to the Screen directly, it **only runs in “ST-High” resolution** (i.e. 640x400x1).

## GEMMAN.PRG

The well behaving GEM version. It opens a window max’ed to the current resolution and starts plotting the fractal in 16 colors. This takes longer than TMANDEL, as it has to do quite a bit of GEM juggling before plotting a pixel…
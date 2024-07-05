# GHE(Global Histogram Enhancement) library

Equalization techniques such as CDF(Cumulative Distribution Function)
is applied in this algorithm to enhance the low contrast images on reflective
panels under ambient light conditions.
Histogram is obtained from the display hardware. This histogram is then fed to
this algorithm. Upon applying the logic, enhanced pixel factor is generated
which is then fed back to the dispay hardware.

##Steps to compile
meson build
ninja -C build
## To install
ninja -C build install

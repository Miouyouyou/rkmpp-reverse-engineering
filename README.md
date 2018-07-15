About
-----

This a clone of RKMPP (Rockchip Media Process Platform) modified
to provide as much information as necessary to rewrite a new VPU
driver.

The main purpose is to dump the registers content and the frames
related to these registers, to the disk, so that I understand how
and why the registers are setup.

The whole idea is that the RKMPP contains the definitions and set up
all the VPU registers by itself.  
Now, writing the actual values to the VPU memory is done through the
VPU driver, but the driver writes most of the provided registers
content without any modification...

So basically, RKMPP *IS* the VPU driver. The kernel driver just
make sure that the IOMMU is setup to perform the DMA read/write
operations, and convert the provided file descriptors to actual
physical memory locations (as catter-gather lists most of the time).

Now, RKMPP is a pain in the ass to setup... I wonder if it wouldn't
be faster to generate an H264 frame by myself and try to decode it
manually...


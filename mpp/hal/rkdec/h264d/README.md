Notes to self :

* Input address is named `rlc_vlc_st_adr` in RKMPP . Maybe it's how you should name the raw encoded bitstream in H264 but... Yeah... thanks to the Chromium driver for putting me on the right track...

* There's two version of VDPU in RKMPP but I don't know which one is correct with RK3288 chips.

* Anyway, the VPDU1 seems correct at first glance... So we'll use this one. Now I just have to dump the registers and data buffers from a working RKMPP run and we should be good to go.

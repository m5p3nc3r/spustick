Spustick
========

This project was started after seeing Grant Likelys arcade controller
<https://github.com/glikely/stm32f3-discovery-arcade>, but I wanted a project
that would introduce me to the world of zephyr.

I am not sure how much time I will have to work on the project, as... well, you
know... life!  But I wanted to share what I have so far with people who may be
interested.

Currently working:
* HID gadget for joystick and buttons
* GPIO input for a single joystick and buttons

TODO:
* LED control for ws2812 RGB LEDs
 * I need some form of scope to see the timings, lets hope Christmas is kind to
   me :o)
* Enable control of multiple joysticks from a single controller
 * Most of the code is there, but there is an issue with the TX rate over USB, I
   need to understand the USB sub-system better to make this work


--------------------
These are notes to myself for setting up my development environment.
Your mileage may vary!

<from project directory>
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=/c/Program\ Files\ \(x86\)/GNU\ Tools\ Arm\ Embedded/7\ 2018-q2-update/
export PATH=$PATH:$HOME/projects/dfu
pushd $HOME/projects/zephyr
source zephyr-env.sh
popd
mkdir -p build && cd build
cmake -GNinja -DBOARD=96b_carbon ..
ninja flash

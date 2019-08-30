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
* LED control for ws2812 RGB LEDs

TODO:
* Enable control of multiple joysticks from a single controller
 * Most of the code is there, but there is an issue with the TX rate over USB, I
   need to understand the USB sub-system better to make this work


## Debug tools used
* hidviz

--------------------
These are notes to myself for setting up my development environment.
Your mileage may vary!
Windows (git bash shell)

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

--------------------
Building on linux (Fedora 30)

Follow the instructions to enable non-root access to usb devices with dfu-util
  https://wiki.yoctoproject.org/wiki/TipsAndTricks/BuildingZephyrImages
You need to ensure the idVendor and idProduct match your device

export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=$HOME/projects/zephyr-sdk/
pushd $HOME/projects/zephyr && source zephyr-env.sh && popd
mkdir -p build && cd build
cmake -GNinja -DBOARD=96b_carbon ..
ninja flash

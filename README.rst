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
# These are notes to myself for setting up my development environment.
# Building on Linux (Fedora 31)
Initialise the west tool as per instructions [here](https://docs.zephyrproject.org/latest/guides/west/install.html)
Install an SDK as per instructions [here](https://docs.zephyrproject.org/latest/getting_started/installation_linux.html#zephyr-sdk)

# get the latest version of Zephyr using the west tool
west init zerphyproject
cd zephyrproject
west update

# in the spustick project directory
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=$HOME/projects/zephyr-sdk-0.10.3/
pushd $HOME/projects/zephyrproject/zephyr
source zephyr-env.sh
popd

west build -b 96b_carbon
west flash

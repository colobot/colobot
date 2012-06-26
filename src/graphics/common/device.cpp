#include "graphics/common/device.h"

//! Sets the default values
Gfx::DeviceConfig::DeviceConfig()
{
    width = 800;
    height = 600;
    bpp = 16;
    fullScreen = false;
    resizeable = false;
    hardwareAccel = true;
    doubleBuf = true;
    noFrame = false;
}
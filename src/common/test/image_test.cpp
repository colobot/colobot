#include "../image.h"

#include <SDL/SDL.h>
#include <stdio.h>

/* For now, just a simple test: loading a file from image
 * and saving it to another in PNG. */

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s in_image out_image\n", argv[0]);
        return 0;
    }

    CImage image;

    if (! image.Load(argv[1]))
    {
        std::string err = image.GetError();
        printf("Error loading '%s': %s\n", err.c_str());
        return 1;
    }

    if (! image.SavePNG(argv[2]))
    {
        std::string err = image.GetError();
        printf("Error saving PNG '%s': %s\n", err.c_str());
        return 2;
    }

    return 0;
}

#include "common/image.h"

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
        printf("Error loading '%s': %s\n", argv[1], err.c_str());
        return 1;
    }
    Gfx::Color color;
    std::string str;

    color = image.GetPixel(Math::IntPoint(0, 0));
    str = color.ToString();
    printf("pixel @ (0,0): %s\n", str.c_str());

    color = image.GetPixel(Math::IntPoint(0, 1));
    str = color.ToString();
    printf("pixel @ (0,1): %s\n", str.c_str());

    color = image.GetPixel(Math::IntPoint(1, 0));
    str = color.ToString();
    printf("pixel @ (1,0): %s\n", str.c_str());

    color = image.GetPixel(Math::IntPoint(1, 1));
    str = color.ToString();
    printf("pixel @ (1,1): %s\n", str.c_str());

    image.SetPixel(Math::IntPoint(0, 0), Gfx::Color(0.1f, 0.2f, 0.3f, 0.0f));
    image.SetPixel(Math::IntPoint(1, 0), Gfx::Color(0.3f, 0.2f, 0.1f, 1.0f));
    image.SetPixel(Math::IntPoint(0, 1), Gfx::Color(1.0f, 1.0f, 1.0f, 1.0f));
    image.SetPixel(Math::IntPoint(1, 1), Gfx::Color(0.0f, 0.0f, 0.0f, 1.0f));

    if (! image.SavePNG(argv[2]))
    {
        std::string err = image.GetError();
        printf("Error saving PNG '%s': %s\n", argv[2], err.c_str());
        return 2;
    }

    return 0;
}

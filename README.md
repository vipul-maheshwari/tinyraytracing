# Understandable RayTracing in 333 lines of C++ code

Ray tracing is an advanced and lifelike process of rendering light and shadows in a scene using the advanced mathematical gemeotry. This technique is vividly used for creating the artifical environments for Games and CG Work for movies. However, because ray tracing works by simulating and tracking every ray of light produced by a source of lighting, it kind of takes a lot of horsepower to actually render the scenes.

Essentially, an algorithm is used which can trace the path of light, and then simulate the way that the light interacts with the virtual objects when it ultimately hits in the computer-generated world. In this project I have used the ray tracing algorithm created using C++ and pre-made image libraries to simulate the way that a light source interacts with the virtual world. If you are more interested in the mathematical theory behind ray tracing, you can read more about it [here](https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-ray-tracing/ray-tracing-practical-example)

The final output is a rendered image of the virtual world. Here it is...
![out](/out.jpg)

***Note: This whole project wouldn't be possible if it wasnt' for [sslov](https://github.com/ssloy/tinyraytracer/). It makes no sense just to look at the code, copy it and paste it for the completion of the project. Cheers and happy rendring!***

------------------------------

## Step 1: write an image to the disk

As the result of our final output will be a simple picture saved on disk. The first thing we need to be able to do is to save the picture to disk. This is done by creating an image of the virtual world and saving it to disk.

```cpp
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"

void render() {
    const int width    = 1024;
    const int height   = 768;
    std::vector<Vec3f> framebuffer(width*height);

    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            framebuffer[i+j*width] = Vec3f(j/float(height),i/float(width), 0);
        }
    }

    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
    render();
    return 0;
}
```

Only render() is called in the main function and nothing else. What is inside the render() function? First of all, I define the framebuffer as a one-dimensional array of Vec3f values, those are simple three-dimensional vectors that give us (r,g,b) values for each pixel. The class of vectors lives in the file geometry.h, I will not describe it here: it is really a trivial manipulation of two and three-dimensional vectors (addition, subtraction, assignment, multiplication by a scalar, scalar product).

I save the image in the ppm format. It is the easiest way to save images, though not always the most convenient way to view them further. If you want to save in other formats, I recommend that you link a third-party library, such as stb. This is a great library: you just need to include one header file stb_image_write.h in the project, and it will allow you to save images in most popular formats.

Warning: my code is full of bugs, I fix them in the upstream, but older commits are affected. Check this issue.

So, the goal of this step is to make sure that we can a) create an image in memory + assign different colors and b) save the result to disk. Then you can view it in a third-party software. Here is the result:

![prototype]()

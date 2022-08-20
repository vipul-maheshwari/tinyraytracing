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

I save the image in the [ppm](https://en.wikipedia.org/wiki/Netpbm#File_formats) format. It is the easiest way to save images, though not always the most convenient way to view them further. If you want to save in other formats, I recommend that you link a third-party library, such as stb. This is a great library: you just need to include one header file stb_image_write.h in the project, and it will allow you to save images in most popular formats.

Warning: my code is full of bugs, I fix them in the upstream, but older commits are affected. Check this issue.

So, the goal of this step is to make sure that we can a) create an image in memory + assign different colors and b) save the result to disk. Then you can view it in a third-party software. Here is the result:

![prototype](/prototype.jpg)

------------------------------

## Step 2: The crucial one: ray tracing

![2](/2.jpg)

This is the most important and difficult step of the whole chain. I want to define one sphere in my code and draw it without being obsessed with materials or lighting. This is how our result should look like:

For the sake of convenience, I have one commit per step in my repository; Github makes it very easy to view the changes made. Here, for instance, what was changed by the second commit.

To begin with, what do we need to represent the sphere in the computer's memory? Four numbers are enough: a three-dimensional vector for the center of the sphere and a scalar describing the radius:

```cpp
struct Sphere {
    Vec3f center;
    float radius;

    Sphere(const Vec3f &c, const float &r) : center(c), radius(r) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};
```

The only non-trivial thing in this code is a function that allows you to check if a given ray (originating from orig in the direction of dir) intersects with our sphere. A detailed description of the algorithm for the ray-sphere intersection can be found here, I highly recommend you to do this and check my code.

How does the ray tracing work? It is pretty simple. At the first step we just filled the picture with a gradient of colors:

```cpp
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            framebuffer[i+j*width] = Vec3f(j/float(height),i/float(width), 0);
        }
    }
```

Now for each pixel we will form a ray coming from the origin and passing through our pixel, and then check if this ray intersects with the sphere:

![3](/3.png)

If there is no intersection with sphere we draw the pixel with color1, otherwise with color2:

```cpp
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const Sphere &sphere) {
    float sphere_dist = std::numeric_limits<float>::max();
    if (!sphere.ray_intersect(orig, dir, sphere_dist)) {
        return Vec3f(0.2, 0.7, 0.8); // background color
    }
    return Vec3f(0.4, 0.4, 0.3);
}

void render(const Sphere &sphere) {
[...]
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
            float y = -(2*(j + 0.5)/(float)height - 1)*tan(fov/2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir, sphere);
        }
    }
[...]
}
```

At this point, I recommend you to take a pencil and check on paper all the calculations (the ray-sphere intersection and the sweeping of the picture with the rays). Just in case, our camera is determined by the following things:

picture width
picture height
field of view angle
camera location, Vec3f(0.0.0)
view direction, along the z-axis, in the direction of minus infinity
Let me illustrate how we compute the initial direction of the ray to trace. In the main loop we have this formula:

```cpp
    float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
    float y = -(2*(j + 0.5)/(float)height - 1)*tan(fov/2.);
```

Where it comes from? Pretty simple. Our camera is placed in the origin and it faces the -z direction. Let me illustrate the stuff, this image shows the camera from the top, the y axis points out of the screen:

![4](/4.png)

As i said, the camera is placed at the origin, and the scene is projected at the screen that lies in the plane z = -1. The field of view specifies what sector of the space will be visible at the screen. In our image the screen is 16 pixels wide; can you compute its length in world coordinates? It is pretty simple: let us focus on the triangle formed by the red, gray and gray dashed line. It is easy to see that

```cpp
tan(field of view / 2) = (screen width) * 0.5 / (screen-camera distance). We placed the screen at the distance of 1 from the camera, thus (screen width) = 2 * tan(field of view / 2).
```

Now let us say that we want to cast a vector through the center of the 12th pixel of the screen, i.e. we want to compute the blue vector. How can we do that? What is the distance from the left of the screen to the tip of the blue vector? First of all, it is 12+0.5 pixels. We know that 16 pixels of the screen correspond to

```cpp
2 * tan(fov/2) world units. Thus, tip of the vector is located at (12+0.5)/16 * 2*tan(fov/2) world units from the left edge, or at the distance of (12+0.5) * 2/16 * tan(fov/2) - tan(fov/2)
```

from the intersection between the screen and the -z axis. Add the screen aspect ratio to the computations and you will find exactly the formulas for the ray direction.

------------------------------

## Step 3: Add more spheres

The hardest part is over, and now our path is clear. If we know how to draw one sphere, it will not take us long to add few more.

![5](/5.jpg)

------------------------------

## Step 4: lighting

The image is perfect in all aspects, except for the lack of light. Throughout the rest of the article we will talk about lighting. Let's add few point light sources:

```cpp
struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};
```

Computing real global illumination is a very, very difficult task, so like everyone else, we will trick the eye by drawing completely non-physical, but visually plausible results. To start with: why is it cold in winter and hot in summer? Because the heating of the Earth's surface depends on the angle of incidence of the Sun's rays. The higher the sun rises above the horizon, the brighter the surface is. Conversely, the lower it is above the horizon, the dimmer it is. And after the sun sets over the horizon, photons don't even reach us at all.

Back our spheres: we emit a ray from the camera (no relation to photons!) at it stops at a sphere. How do we know the intensity of the intersection point illumination? In fact, it suffices to check the angle between a normal vector in this point and the vector describing a direction of light. The smaller the angle, the better the surface is illuminated. Recall that the scalar product between two vectors a and b is equal to product of norms of vectors times the cosine of the angle between the vectors: a*b = |a| |b| cos(alpha(a,b)). If we take vectors of unit length, the dot product will give us the intensity of surface illumination.

Thus, in the cast_ray function, instead of a constant color we will return the color taking into account the light sources:

```cpp
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const Sphere &sphere) {
    [...]
    float diffuse_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        Vec3f light_dir      = (lights[i].position - point).normalize();
        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, light_dir*N);
    }
    return material.diffuse_color * diffuse_light_intensity;
}
```

![6](/6.jpg)

------------------------------

## Step 5: specular lighting

The dot product trick gives a good approximation of the illumination of matt surfaces, in the literature it is called diffuse illumination. What should we do if we want to draw shiny surfaces? I want to get a picture like this:

![7](/7.jpg)

This trickery with illumination of matt and shiny surfaces is known as Phong reflection model. The wiki has a fairly detailed description of this lighting model. It can be nice to read it side-by-side with the source code. Here is the key picture to understanding the magic:

![8](/8.png)

------------------------------

## Step 6: shadows

Why do we have the light, but no shadows? It's not okay! I want this picture:

![9](/9.jpg)

Mere six lines of code allow us to achieve this: when drawing each point, we just make sure that the segment between the current point and the light source does not intersect the objects of our scene. If there is an intersection, we skip the current light source. There is only a small subtlety: I perturb the point by moving it in the direction of normal:

```cpp
Vec3f shadow_orig = light_dir*N < 0 ? point - N*1e-3 : point + N*1e-3;
```

Why is that? It's just that our point lies on the surface of the object, and (except for the question of numerical errors) any ray from this point will intersect the object itself.

------------------------------

## Step 7: reflections

It's incredible, but to add reflections to our render, we just need to add three lines of code:

```cpp
    Vec3f reflect_dir = reflect(dir, N).normalize();
    Vec3f reflect_orig = reflect_dir*N < 0 ? point - N*1e-3 : point + N*1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
```

![10](/10.jpg)

------------------------------

## Step 8: reflections

If we know to do reflections, refractions are easy. We need to add one function to compute the refracted ray (using Snell's law), and three more lines of code in our recursive function cast_ray. Here is the result where the closest ball is "made of glass", it reflects and refracts the light at the same time:

![11](/11.jpg)

------------------------------

## Step 9: Beyond the Spheres

Till this moment we rendered only spheres because it is one of the simplest nontrivial mathematical objects. Let us add a plane. The chessboard is a classic choice. For this purpose, it is quite enough to add a dozen lines. And here is the result:

![out](/out.jpg)

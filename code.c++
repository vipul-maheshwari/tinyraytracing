#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "HeaderFiles\stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "HeaderFiles\stb_image.h"
#include "HeaderFiles\geometry.h"


/*Some points to remember
#Reading the wiki is must to understand this project
1. For every new property, we are creating a new structure having that property details i.e Material color or lighting
2. normalize() normalize the vector [unit vector]
3. The diffuse term is not affected by the viewer direction (V (cap)). The specular term is large only 
when the viewer direction (V (cap)) is aligned with the reflection direction (R(cap)) 
Their alignment is measured by the alpha  power of the cosine of the angle between them. 
The cosine of the angle between the normalized vectors (R(cap))  and (V(cap))  is equal to their dot product. When alpha  is large, 
in the case of a nearly mirror-like reflection, the specular highlight will be small, 
because any viewpoint not aligned with the reflection will have a cosine less than one 
which rapidly approaches zero when raised to a high power.
*/


//For illumination purposes
struct Light {
    Light(const Vec3f &p, const float i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

//Material is the structure which provides the color information for the rendering purposes
struct Material {
    
    Material(const float r, const Vec4f &a, const Vec3f &color, const float spec) : refractive_index(r), albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : refractive_index(1), albedo(1,0,0,0), diffuse_color(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    /*Albedo is a non-dimensional, unitless quantity that indicates how well a surface reflects solar energy. ... 
    Albedo commonly refers to the "whiteness" of a surface, with 0 meaning black and 1 meaning white. 
    A value of 0 means the surface is a "perfect absorber" that absorbs all incoming energy.
    */
    Vec3f diffuse_color;
    float specular_exponent;
};


//A user defind datatype for creating the spheres on our screen
struct Sphere {
    Vec3f center;       //Center of the sphere
    float radius;       //Radius of the sphere
    Material material;  //Material of the sphere

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

    //A method to find out if the ray intersect the sphere or not which will help us to know if our sphere is in our field of view or not
    //Names of the coordinates and other parameters are taken considering the reference from the below link.
    //http://www.lighthouse3d.com/tutorials/maths/ray-sphere-intersection/
    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &intersection_point) const {

        Vec3f L = center - orig;   
        float dist_bw_pc_p = L*dir;                       
        float dist_bw_c_pc = L*L - dist_bw_pc_p*dist_bw_pc_p;  

        if (dist_bw_c_pc > radius*radius) return false;

        float dist_bw_pc_i1 = sqrtf(radius*radius - dist_bw_c_pc); //Distance between pc to i1
        intersection_point = dist_bw_pc_p - dist_bw_pc_i1;
        float t1 = dist_bw_pc_p + dist_bw_pc_i1;

        //If the origin of the ray is inside the sphere.
        if (intersection_point < 0) intersection_point = t1;
        
        //Q: If the origin of the ray is on the spehere then ? A: If that's the case then the intersection point distance would be zero
        //As the distance between the the p and the pc is equal to the distance between the pc and i1
        
        //Even if the intersection point distance value is less than 0, then there is no such kind of intersection point
        if (intersection_point < 0) return false;
        return true;
    }
};


//Ilumintaion total(Phong) = Illumination(ambient) + Illumination(diffuse) + Illumination(specular)
//I -> Incident vector ( Vector from the light source to the hitting surface)
//N -> Normal vector   ( Vector between the directed ray and the reflected ray)
//The formulation in the below method is based on the 
Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}


//You need to find the refracted vector in terms of I and the N where I is the casted ray and the N is the normal
//See the formula in the wiki https://en.wikipedia.org/wiki/Snell%27s_law
Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    float r_of_air = 1, r_of_medium = refractive_index;
    Vec3f n = N;
    if (cosi < 0) { // if the ray is inside the object, swap the indices and invert the normal to get the correct result
        cosi = -cosi;
        std::swap(r_of_air, r_of_medium); n = -N;
    }
    float ratio = r_of_air / r_of_medium;
    float k = 1 - ratio*ratio*(1 - cosi*cosi);
    return k < 0 ? Vec3f(0,0,0) : I*ratio + n*(ratio * cosi - sqrtf(k));
}



bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    //spheres_dist: 3.40282e+038
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        //dist_i < spheres_dist means the ray is coming from the front of the sphere and then intersection is happening(in case if it happens)

        //NOTE: If there are two same spheres ( exactly same at the same depth and the same position) then the priority order will be given according to the first come first serve basis
        //As we have mentioned that the material color will change only if the dist_i < spheres_dist, and as for two same spheres
        //the spehere_dist will be equal to the dist_i calculated and then for the second one the dist_i so calculated will be equal to the sphere_dist(dist_i of previous sphere)
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            
            //After finding the intersection point, it's actually the distance of the sphere from the origin
            spheres_dist = dist_i; 
            //hit point tells us about the total distance the sphere is from the origin
            hit = orig + dir*dist_i;

            //N gives normal between the hit point and the center of the sphere.
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material; 
        }
    }
    
    //If the spehere_dist < 1000 that means we make some changes in the spheres_dist and 
    //it is only possible if any of our ray intersect at any of our sphere which will give us true 
    //And if not then the spehere_dist (3.40282e+038) will not be changed and we will get false for that purpose



    //Note for anyone stupid like me, dist_i < spheres_dist in line 62 and spheres_dist = dist_i in line 65 make sure that the sphere closest to the camera will be drawn. 
    //Also, return spheres_dist<1000 in line 82 guarantees that the sphere 
    //a) father than 1000 from the camera, b) doesn't intersect with the ray will not be drawn


    //https://github.com/ssloy/tinyraytracer/commit/c19c430151cb659372b4988876173b022164e371

    //Creating a checkboard object
    float checkerboard_dist = std::numeric_limits<float>::max();
    //dir.y can be positive as well as negative, this line is here to prevent a division by zero, therefore i use fabs
    if (fabs(dir.y)>1e-3)  {
        //orig.y was 0 ( see the casted ray source in the render method)
        //As we want to create a cardboard up
        float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
        //pt is the hitting point
        Vec3f pt = orig + dir*d;
        //fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 -> They all are checking if the cardboard is in the viewer space or not
        //You can change the conditions if you want
        //z coordinte defy depth of the cardboard (more negative means more distance from the source)
        //fabs return the absolute value ( you can use the abs in place of it) { there was fabs at the place of abs}
        if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
            checkerboard_dist = d;
            hit = pt;
            N = Vec3f(0,1,0);
            material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(1,1,1) : Vec3f(0, 0, 0);
        }

    }
    return std::min(spheres_dist, checkerboard_dist)<1000;
}


//A ray is casted from the origin to the direction mentioned for every pixel in the space
//Ray is originated from the origin and ended at the pixel mentioned
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights,size_t depth=0)  {
    Vec3f hit, N;
    Material material;


    //If the ray (pixel) is at the place of the sphere, then the colour of that pixel is the colour mentioned of that sphere.
    //material = spheres[i].colour;
    //In other case, if the ray (pixel) is not at the place of the sphere, then the colour of that pixel is default mentioned
    //Scene intersects method checks for any ray(mentioned with the reference of the origin and it's direction with the object of our scene) speheres in our case
    if (depth>4 || !scene_intersect(orig, dir, spheres, hit, N, material)) {
        return Vec3f{0.2, 0.7, 0.8}; // background color
    }
    
    //First we calculated the reflect_dir using the same method we used for specular lighting
    //Then we created a reflection_origin using the same way we created the shadow_orig

    Vec3f reflect_dir = reflect(dir, N);
    Vec3f refract_dir = refract(dir, N, material.refractive_index).normalize();
    Vec3f reflect_orig = reflect_dir*N < 0 ? hit - N*1e-3 : hit + N*1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f refract_orig = refract_dir*N < 0 ? hit - N*1e-3 : hit + N*1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    Vec3f refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);
    
    //The illumination on the spheres is determined by position of the light vector
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {

        //Creating the light_dir vector
        //Cos product between the light_dir and N(the vector between the hit point and the center of the sphere)
        Vec3f light_dir      = (lights[i].position - hit).normalize();
        float light_distance = (lights[i].position - hit).norm();


        //everytime when there is an intersection between the casted ray from the source and the object, we are creating an another light source,
        //but this time this is from the point where the previous (original light source intersected with the object which we call hit point)
        //and then we are checking if that source intersects another object or not, if it is then every point where shadow source intersected with the other object behind it
        //we will make that point as no color ( no light source for that point )
        
        Vec3f shadow_orig = light_dir*N < 0 ? hit - N*1e-1 : hit + N*1e-2; 
        Vec3f shadow_hit, shadow_N;

        //A dummy material
        Material tmpmaterial;
        
        //(shadow_pt-shadow_orig).norm() < light_distance -> for the given spheres, the distance between them should be less than light_distance as if they are much farther, then 
        //their shadows will not be seen as such. So it's necessary to have the distance between them

        //for a particular sphere, we are more into creating a different vector ( shadow vector ) which is originated from the hit point (some distance from the hit point)
        //because as the hit point is on the surface of the sphere, any ray from that hit point will intersect the object itself, that's why we need to
        //shift that hit point a little bit away towards and away the normal.

        //For the shadow case, there is light source from the hit point to the backwards side which will check for the other object in its way.
        //See Image:D:\Projects\Image_Rendering_in_C++\Resources\shadow.jpeg
        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_hit, shadow_N, tmpmaterial) && (shadow_hit-shadow_orig).norm() < light_distance)
            continue;

        //Diffuse light intensity will be maximum if the light is falling on the pixel parallelly that is at 0 degrees
        //Diffuse illumination is not effected by the viewer angle, as it is only affected by the light direction and it's intersection with the hitting surface
        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, light_dir*N);

        //All the abbrevations in the equation of the diffuse and the specular illumination is applied to the formula in this wiki.
        //https://en.wikipedia.org/wiki/Phong_reflection_model
        //reflect(-light_dir, N)*dir -> Rm(cap)
        //material.specular_exponent -> alpha value (shininess constant of the material (spheres in our case))
        //dir -> V(cap) [viewer direction]
        
        /*----------------
        float powf( float base, float exponent );
        double pow( double base, double exponent );
        ------------------*/

        //dir -> V(cap) [viewer direction], as in our case we are looking at the image from the same direction as we casting the ray, 
        //the V(cap) vector is the same as the direction vector for the light
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*dir), material.specular_exponent)*lights[i].intensity;

    }
    
    //material.diffuse_color is kd (Diffusion constant)
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2] + refract_color*material.albedo[3];
}



void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights){
    const int width    = 1024; //picture width
    const int height   = 768;  //picture height

    //More the fov is, the small will be the spheres will be as they will be present in much larger space
    const int fov      = M_PI/3; //M_PI: 3.14159265358979323846, fov: The field of view specifies what sector of the space will be visible at the screen

    //Creating a Vector of type <Vect3f> which consists of RGB values of each of the pixels    
    std::vector<Vec3f> framebuffer(width*height); 


    #pragma omp parallel for
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            //tan(field of view / 2) = (screen width) * 0.5 / (screen-camera distance).
            //Add the screen aspect ratio to the computations and you will find exactly the formulas for the ray direction. i.e width/(float)height
            /*
            Now let us say that we want to cast a vector through the center of the 12th pixel of the screen, i.e. 
            we want to compute the blue vector. How can we do that? What is the distance from the left of the screen to the 
            tip of the blue vector? First of all, it is 12+0.5 pixels. We know that 16 pixels of the screen correspond to 2 * tan(fov/2) 
            world units. Thus, tip of the vector is located at (12+0.5)/16 * 2*tan(fov/2) world units from the left edge, 
            or at the distance of (12+0.5) * 2/16 * tan(fov/2) - tan(fov/2) from the intersection between the screen and the -z axis. 
            Add the screen aspect ratio to the computations and you will find exactly the formulas for the ray direction.
            */
            // float x =  (2*(i + 0.5)/(float)width  - 1)*tan(fov/2.)*width/(float)height;
            // float y =  (2*(j + 0.5)/(float)height - 1)*tan(fov/2.);

            //Spheres are at constant position for the space, the main thing is the creationg of the view ray.
            //If you change the denominator let's say increase the number from 2 to 4 then 
            //then you are moving your view ray source towards right ( +ve x axis as dir_X will be more positive)
            //( less aligned to the viewing space) and vice versa. (Make some changes in the dir_x by changing the denominator)



            float dir_x =  (i + 0.5) -  width/2.;
            float dir_y = -(j + 0.5) + height/2.;    // this flips the image at the same time
            float dir_z = -height/(2.*tan(fov/2.)); //width*0.6 -> this is for make space a little bit smaller
            framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), Vec3f(dir_x, dir_y, dir_z).normalize(), spheres, lights);

            //Creating a dir vector for the casted ray 
            //The casted ray direction is the same as the pixel position, but just to create a depth effect, we have changed the Z coordinate
            //Vec3f dir = Vec3f(x, y, -1).normalize();
            //column_number + row_number*total_columns
            //framebuffer[i+j*width] = cast_ray(Vec3f(0,0,0), dir, spheres, lights);
        }
    }

    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (Vec3f &c : framebuffer) {
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max>1) c = c*(1./max);
        ofs << (char)(255 * c[0]) << (char)(255 * c[1]) << (char)(255 * c[2]);
    }
    ofs.close();
}

int main() {
    
    //Vec3f take the RGB values of the colour
    Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.8, 0.5),   50.);
    Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  1255.);
    Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);

    std::vector<Sphere> spheres;
    //x: It takes width into account. Dimensions are self understandable
    //y: It takes height into account. Dimensions are self understandable ( going up decreases the y and vice versa)
    //z: It takes depth into account. The more the negative value, the more sphere will go far and become small.
    spheres.push_back(Sphere(Vec3f(-3,    0,   -16), 2,      ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
    spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f( 2.8,    7,   -28), 7,     mirror));

    std::vector<Light>  lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.4));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));


    render(spheres, lights);

    return 0;

}
# RaytracingFinal

## Example images

 

![teapot](./teapot.png)

![teapot](./balls.png)

![teapot](./rings.png)



## A description of what I implemented

### The basic functions of raytracing were implemented.

#### Step 1 Ray-triangle & Ray-sphere intersection test

The intersection is used to calculated the intersection of a ray with a triangle or sphere, which could be applied to find where the ray hits. The closest point in the intersecting pixel is found after the traverse and the data saved in HitRecord hr is updated for the next step.

![](./teapot black.png)

```c++
bool Sphere::intersect(const Ray &r, double t0, double t1, HitRecord &hr) const {
    double delta,A,B,C,root,root1,root2,t;
    A = dot(r.d, r.d);
    B = 2 * dot((r.e - c), r.d);
    C = dot((r.e - c), (r.e - c)) - rad * rad;
    delta = B * B - 4 * A * C;
    if (delta < 0) {
        return false;
    }
    if (delta == 0) {
        root = (-B) / (2 * A);
        t = root;
    }
    if (delta > 0) {
        root1 = (-B + sqrt(delta)) / (2 * A);
        root2 = (-B - sqrt(delta)) / (2 * A);
        if (root1 < root2 && root1 >= 0)
            t = root1;
        if (root2 < root1 && root2 >= 0)
            t = root2;
        if (root1 < 0 && root2 >= 0)
            t = root2;
        if (root2 < 0 && root1 >= 0)
            t = root1;
        if (root1 < 0 && root2 < 0)
            return false;
    }
    if (t < t0 || t > t1) 
        return false;
    hr.t = t;
    hr.p = r.e + t * r.d;
    hr.n = (hr.p - c) / rad;
    normalize(hr.n);
    return true;
    // Step 1 Sphere-triangle test
}
```



```c++
bool Triangle::intersect(const Ray &r, double t0, double t1, HitRecord &hr) const {
    double beta, gamma,t,A,B,C,D;
    // Step 1 Ray-triangle test
    A = (r.d[0] * (c[1] - a[1]) - r.d[1] * (c[0] - a[0])) / (r.d[0] * (b[1] - a[1]) - r.d[1] * (b[0] - a[0]));
    B = (r.e[1] * r.d[0] - r.e[0] * r.d[1] + a[0] * r.d[1] - a[1] * r.d[0]) / (r.d[0] * (b[1] - a[1]) - r.d[1] * (b[0] - a[0]));
    C = ((c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0])) / (r.d[0] * (b[1] - a[1]) - r.d[1] * (b[0] - a[0]));
    D = (r.e[1] * (b[0] - a[0]) - r.e[0] * (b[1] - a[1]) + a[0] * b[1] - a[1] * b[0]) / (r.d[0] * (b[1] - a[1]) - r.d[1] * (b[0] - a[0]));
    gamma = (r.e[2] + D * r.d[2] - a[2] - B * (b[2] - a[2])) / (c[2] - a[2] - A * (b[2] - a[2]) - C * r.d[2]);
    beta = ( - gamma * A) + B;
    t = gamma * C + D;
    if (t < t0 || t > t1) 
        return false;
    if (beta < 0 || beta > 1) 
        return false;
    if (gamma < 0.0 || gamma > 1.0 - beta) 
        return false;
    hr.t = t;
    hr.p = r.e + t * r.d;
    hr.n = cross((b-a),(c-a));
    normalize(hr.n);
    hr.alpha = 1.0 - beta - gamma;
    hr.beta = beta;
    hr.gamma = gamma;
    return true;
}
```

```c++
SlVector3 Tracer::trace(const Ray &r, double t0, double t1) const {
    HitRecord hr;
    SlVector3 color(bcolor);
  
    bool hit = false;
    for (int i = 0; i < surfaces.size(); i++) {
        if (surfaces[i].first->intersect(r, t0, t1, hr)) {
            t1 = hr.t;
            hr.f = surfaces[i].second;
            hr.v = r.e - hr.p;
            hr.raydepth = r.depth;
            normalize(hr.v);
            normalize(hr.n);
            hit = true;
        }
    }
    // Step 1 See what a ray hits  

    if (hit) color = shade(hr);
    return color;
}
```

#### Step 2 Shading

Calculate the diffuse and specular light by the shading skills which used in assignment 1 to get the shading color 

only diffuse:

![](./teapot diffuse.png)

shading=diffuse + specular:

![](./teapot%20shading-1662933575331-1.png)

#### Step 3 Shadows

Use the similar principle when dealing with shadows and reflections----to figure out if the light ray or reflect ray hit any surfaces

In shadow part: if the light ray hit any surfaces return black color. The initial value of t0 is set to be greater than 0 to prevent the intersection of light ray and itself, while the initial value of t1 is set to a sufficiently large number, which is the magnitude of light ray.

![](teapot shadow.png)

#### Step 4 Reflection

In reflection part: if the reflection ray of the eye-ray hit anything, the point where eye-ray hit becomes new "eye", the reflection ray and new eye will be used to do a "trace" again

when doing reflection there was a recursive loop between trace and shade, what I could do is to set a threshold value to stop this recursive loop

![teapot](./teapot.png)

```c++
SlVector3 Tracer::shade(HitRecord &hr) const {
    if (color) return hr.f.color;
          
    SlVector3 color(0.0);
    HitRecord dummy,hrr;
  

    for (unsigned int i = 0; i < lights.size(); i++) {
        const Light& light = lights[i];
        bool shadow = false;
        SlVector3 l = light.p - hr.p;
        normalize(l);
        SlVector3 r = -l + 2 * dot(l, hr.n) * hr.n;
        normalize(r);
        Ray StoL = Ray(hr.p, l, hr.raydepth);
        double x = dot(r,hr.v);
        for (int j = 0; j < surfaces.size(); j++) {
            if (surfaces[j].first->intersect(StoL, 0.01, mag(light.p-hr.p), dummy)) {
                shadow = true;   
            }
        }
        // Step 3 Check for shadows here

        if (!shadow) {
            SlVector3 diffuse = hr.f.color * hr.f.kd * std::max(dot(l, hr.n), 0.0) * light.c;
            SlVector3 specular = hr.f.color * hr.f.ks * light.c * pow(std::max(x, 0.0), hr.f.shine);
            color = color + diffuse+specular;
            // Step 2 do shading here
        }
        
    }
    if (hr.raydepth < 5) {
        SlVector3 reflectlight = -hr.v + 2 * dot(hr.v, hr.n) * hr.n;
        normalize(reflectlight);
        hr.raydepth = hr.raydepth + 1;
        Ray reflect = Ray(hr.p, reflectlight, hr.raydepth);
        SlVector3 reflectcolor = trace(reflect, 0.01, 50);
        color = color + hr.f.ks * reflectcolor;
        
    }
    // Step 4 Add code for computing reflection color here

    // Step 5 Add code for computing refraction color here
    return color;
}
```



## Some problems and how I solved them

#### First

The first problem I met in programming is the shading part. it is easy to miss the color of the object hr.f.color. As the result of this process, the rendering figure was white.

![teapot](./teapot white.jpg)

#### Second

The second problem I met is that I consider the normal vector of the triangle in the opposite direction, which result the black color of the teapot base.

![teapot](./teapot blackbase.png)

To find out the reason, I have removed the section of Specular and examined the Diffuse error.

![teapot](./teapot white2.png)

To solve this problem, the only thing we need to do is to change the order of two line of triangle in cross product.

#### Third

Then when I calculate the specular of the light shading, I've reversed the order of taking non-negative value and power function. The result is a very interesting and strange picture.

![teapot](./teapot spec.png)

#### Fourth

This problem I met is in the shadow part. After programming, I found that there were too many black spots. I suspect this is due to the precision of the numbers. So I changed some of the float data types that I defined to double.

![teapot](./teapot blackspots.png)

#### Fifth

The problem has to do with the extra part. I am currently working on an accelerate module to reduce program runtime. Before this function was implemented, except the balls, the teapot and the rings all others took a lot of time even more than an hour. To achieve the accelerate module, the BVH（Bounding Volume Hierarchy）skills should be implemented. The basic theory of BVH is the bounding box and the "tree" which could extremely save the time.

#### Sixth

The final problem is that the resulting image is blurry and not very sharp like a real object. There are some jagged shapes visible to the naked eye around the object. This may be caused by interference and noise in the program. So far, I'm still confused about how to solve this problem in code. After the course finished, I will continue to search for materials and study to solve this problem.

## What I learned and what could do for extending the code

From this short course, I learned the basic theory of the ray tracing. What is ray tracing? Why we use ray tracing? How to achieve ray tracing? How to optimize ray tracing?

Ray tracing is a real display method of the object, along the ray tracing method to reach the viewpoint in the opposite direction of the light track, after every pixel on the screen, find the intersecting line of sight of the surface of the object points, and continue to track, affecting all the point light source, so as to work out this point eyes can see the exact color in pixels.

To achieve the basic function of ray tracing, we need to figure out the intersection, shading, shadow and reflection part.

In addition, further research is needed to optimize ray tracing. To optimize the result of ray tracing, there are many things we can do. In the shading part, we could not only use the approximate BRDF phong illumination model, but also can use the Ashikhmin-Shirley BRDF function to shade closed to the real world. In the shading part of the code, we could add the refraction function. The next part is one of the most significant part. The BVH（Bounding Volume Hierarchy）skills should be implemented to reduce the programming time. The basic theory of BVH is the bounding box and the "tree" which could extremely save the time. At last, the photon mapping also can be considered in the future optimize code. Last but no least,  the blur image caused by some interferences and noise should also be updated and optimized in the code.

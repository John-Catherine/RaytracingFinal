#include "trace.H"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>
//#include <getopt.h>
#ifdef __APPLE__
#define MAX std::numeric_limits<double>::max()
#else
//#include <values.h>
#define MAX DBL_MAX
#endif

// return the determinant of the matrix with columns a, b, c.
double det(const SlVector3 &a, const SlVector3 &b, const SlVector3 &c) {
    return a[0] * (b[1] * c[2] - c[1] * b[2]) +
        b[0] * (c[1] * a[2] - a[1] * c[2]) +
            c[0] * (a[1] * b[2] - b[1] * a[2]);
}

inline double sqr(double x) {return x*x;} 

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

bool TrianglePatch::intersect(const Ray &r, double t0, double t1, HitRecord &hr) const {
    bool temp = Triangle::intersect(r,t0,t1,hr);
    if (temp) {
        hr.n = hr.alpha * n1 + hr.beta * n2 + hr.gamma * n3;
        normalize(hr.n);
        return true;
    }
    return false;
}


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




Tracer::Tracer(const std::string &fname) {
    std::ifstream in(fname.c_str(), std::ios_base::in);
    std::string line;
    char ch;
    Fill fill;
    bool coloredlights = false;
    while (in) {
        getline(in, line);
        switch (line[0]) {
            case 'b': {
                std::stringstream ss(line);
                ss>>ch>>bcolor[0]>>bcolor[1]>>bcolor[2];
                break;
            }

            case 'v': {
                getline(in, line);
                std::string junk;
                std::stringstream fromss(line);
                fromss>>junk>>eye[0]>>eye[1]>>eye[2];

                getline(in, line);
                std::stringstream atss(line);
                atss>>junk>>at[0]>>at[1]>>at[2];

                getline(in, line);
                std::stringstream upss(line);
                upss>>junk>>up[0]>>up[1]>>up[2];

                getline(in, line);
                std::stringstream angless(line);
                angless>>junk>>angle;

                getline(in, line);
                std::stringstream hitherss(line);
                hitherss>>junk>>hither;

                getline(in, line);
                std::stringstream resolutionss(line);
                resolutionss>>junk>>res[0]>>res[1];
                break;
            }

            case 'p': {
                bool patch = false;
                std::stringstream ssn(line);
                unsigned int nverts;
                if (line[1] == 'p') {
                    patch = true;
                    ssn>>ch;
                }
                ssn>>ch>>nverts;
                std::vector<SlVector3> vertices;
                std::vector<SlVector3> normals;
                for (unsigned int i=0; i<nverts; i++) {
                    getline(in, line);
                    std::stringstream ss(line);
                    SlVector3 v,n;
                    if (patch) ss>>v[0]>>v[1]>>v[2]>>n[0]>>n[1]>>n[2];
                    else ss>>v[0]>>v[1]>>v[2];
                    vertices.push_back(v);
                    normals.push_back(n);
                }
                bool makeTriangles = false;
                if (vertices.size() == 3) {
                    if (patch) {
                        surfaces.push_back(std::pair<Surface *, Fill>(new TrianglePatch(vertices[0], vertices[1], vertices[2], 
                        normals [0], normals [1], normals [2]), fill));
                    } else {
                        surfaces.push_back(std::pair<Surface *, Fill>(new Triangle(vertices[0], vertices[1], vertices[2]), fill));
                    }
                } else if (vertices.size() == 4) {
                    SlVector3 n0 = cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);
                    SlVector3 n1 = cross(vertices[2] - vertices[1], vertices[3] - vertices[1]);
                    SlVector3 n2 = cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
                    SlVector3 n3 = cross(vertices[0] - vertices[3], vertices[1] - vertices[3]);
                    if (dot(n0,n1) > 0 && dot(n0,n2) > 0 && dot(n0,n3) > 0) {
                        makeTriangles = true;
                        if (patch) {
                            surfaces.push_back(std::pair<Surface *, Fill>(new TrianglePatch(vertices[0], vertices[1], vertices[2], 
                            normals[0], normals[1], normals[2]), fill));
                            surfaces.push_back(std::pair<Surface *, Fill>(new TrianglePatch(vertices[0], vertices[2], vertices[3], 
                            normals[0], normals[2], normals[3]), fill));
                        } else {
                            surfaces.push_back(std::pair<Surface *, Fill>(new Triangle(vertices[0], vertices[1], vertices[2]), fill));
                            surfaces.push_back(std::pair<Surface *, Fill>(new Triangle(vertices[0], vertices[2], vertices[3]), fill));
                        }
                    }
                    if (!makeTriangles) {
                        std::cerr << "I didn't make triangles.  Poly not flat or more than quad.\n";
                    }
                }
                break;
            }

            case 's' : {
                std::stringstream ss(line);
                SlVector3 c;
                double r;
                ss>>ch>>c[0]>>c[1]>>c[2]>>r;
                surfaces.push_back(std::pair<Surface *, Fill>(new Sphere(c,r), fill));
                break;
            }
	  
            case 'f' : {
                std::stringstream ss(line);
                ss>>ch>>fill.color[0]>>fill.color[1]>>fill.color[2]>>fill.kd>>fill.ks>>fill.shine>>fill.t>>fill.ior;
                break;
            }

            case 'l' : {
                std::stringstream ss(line);
                Light l;
                ss>>ch>>l.p[0]>>l.p[1]>>l.p[2];
                if (!ss.eof()) {
                    ss>>l.c[0]>>l.c[1]>>l.c[2];
                    coloredlights = true;
                }
                lights.push_back(l);
                break;
            }

            default:
            break;
        }
    }
    if (!coloredlights) for (unsigned int i=0; i<lights.size(); i++) lights[i].c = 1.0/sqrt(lights.size());
    im = new SlVector3[res[0]*res[1]];
    shadowbias = 1e-6;
    samples = 1;
    aperture = 0.0;
}

Tracer::~Tracer() {
    if (im) delete [] im;
    for (unsigned int i=0; i<surfaces.size(); i++) delete surfaces[i].first;
}


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

void Tracer::traceImage() {
    // set up coordinate system
    SlVector3 w = eye - at;
    w /= mag(w);
    SlVector3 u = cross(up,w);
    normalize(u);
    SlVector3 v = cross(w,u);
    normalize(v);

    double d = mag(eye - at);
    double h = tan((3.14/180.0) * (angle/2.0)) * d;
    double l = -h;
    double r = h;
    double b = h;
    double t = -h;

    SlVector3 *pixel = im;

    for (unsigned int j=0; j<res[1]; j++) {
        std::cout << j << std::endl;
        for (unsigned int i=0; i<res[0]; i++, pixel++) {

            SlVector3 result(0.0,0.0,0.0);

            for (int k = 0; k < samples; k++) {

                double rx = 1.1 * rand() / RAND_MAX;
                double ry = 1.1 * rand() / RAND_MAX;

                double x = l + (r-l)*(i+rx)/res[0];
                double y = b + (t-b)*(j+ry)/res[1];
                SlVector3 dir = -d * w + x * u + y * v;
	
                Ray r(eye, dir);
                normalize(r.d);

                result += trace(r, hither, MAX);

            }
            (*pixel) = result / samples;
        }
    }
}

void Tracer::writeImage(const std::string &fname) {
#ifdef __APPLE__
    std::ofstream out(fname, std::ios::out | std::ios::binary);
#else
    std::ofstream out(fname.c_str(), std::ios_base::binary);
#endif
    out<<"P6"<<"\n"<<res[0]<<" "<<res[1]<<"\n"<<255<<"\n";
    SlVector3 *pixel = im;
    char val;
    for (unsigned int i=0; i<res[0]*res[1]; i++, pixel++) {
        val = (unsigned char)(std::min(1.0, std::max(0.0, (*pixel)[0])) * 255.0);
        out.write (&val, sizeof(unsigned char));
        val = (unsigned char)(std::min(1.0, std::max(0.0, (*pixel)[1])) * 255.0);
        out.write (&val, sizeof(unsigned char));
        val = (unsigned char)(std::min(1.0, std::max(0.0, (*pixel)[2])) * 255.0);
        out.write (&val, sizeof(unsigned char));
    }
    out.close();
}


int main(int argc, char *argv[]) {
    int c;
    double aperture = 0.0;
    int samples = 1;
    int maxraydepth = 5;
    bool color = false;
    /*std::cout << argv[0] << " " << argv[1] << std::endl;
    while ((c = getopt(argc, argv, "a:s:d:c")) != -1) {
        switch(c) {
            case 'a':
            aperture = atof(optarg);
            break;
            case 's':
            samples = atoi(optarg);
            break;
            case 'c':
            color = true;
            break;
            case 'd':
            maxraydepth = atoi(optarg);
            break;
            default:
            abort();
        }
    }

    if (argc-optind != 2) {
        std::cout<<"usage: trace [opts] input.nff output.ppm"<<std::endl;
        for (unsigned int i=0; i<argc; i++) std::cout<<argv[i]<<std::endl;
        exit(0);
    }	*/

    Tracer tracer(argv[1]);
    tracer.aperture = aperture;
    tracer.samples = samples;
    tracer.color = color;
    tracer.maxraydepth = maxraydepth;
    tracer.traceImage();
    tracer.writeImage(argv[2]);
};

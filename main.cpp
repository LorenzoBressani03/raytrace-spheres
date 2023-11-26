#ifdef WIN32
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <random>
#include <math.h>
#include <cmath>
//puts pixels on screen
void putPixel(SDL_Renderer *renderer, float x, float y, unsigned char r, unsigned char g, unsigned char b)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}

//point object
struct point_t
{
    float x;
    float y;
    float z;
};

//vector object
struct vec_t
{
    float x;
    float y;
    float z;
};

//color
struct color_t
{
    float r;
    float g;
    float b;
};

//sphere object
struct sphere_t
{
    point_t centre;
    float r;
    struct color_t color;
};

//hit object
struct hit_t
{
    int hit;
    color_t color;
    float dist;
    sphere_t obj;
};

struct light_t
{
    struct point_t pos;
    struct color_t col;
    float brightness;
};

vec_t vecAddition(vec_t a, vec_t b)
{
    vec_t addition;
    addition.x = (a.x+b.x);
    addition.y = (a.y+b.y);
    addition.z = (a.z+b.z);
    return addition;
}

//dot product function
float dotprod(vec_t a, vec_t b)
{
    return(a.x*b.x+a.y*b.y+a.z*b.z);
}

//point subtraction function
vec_t pointSub(point_t a, point_t b)
{
    vec_t result = {a.x-b.x, a.y-b.y, a.z-b.z};
    return result;
}

//normalisation function
vec_t normalise(vec_t v)
{
    float length = sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
    vec_t result = {v.x/length, v.y/length, v.z/length};
    return result;
}

//figures out what and if it hits something
struct hit_t traceObj(point_t src, vec_t dir, sphere_t obj)
{
    hit_t h;
    vec_t L = pointSub(obj.centre, src);
    float tca = dotprod(L,dir);
    float d = sqrt(dotprod(L,L)-tca*tca);
    if(obj.r>d){
        h.hit = 1;
        float thc = sqrt(obj.r*obj.r - d*d);
        h.dist = tca - thc;
    }
    else h.hit = 0;
    h.obj = obj;
    return h;
}

//calculates the normal to the surface
color_t shade(point_t src, vec_t dir, sphere_t obj, float dist)
{
    struct point_t Ph;
    Ph.x=src.x+dist*dir.x;
    Ph.y=src.y+dist*dir.y;
    Ph.z=src.z+dist*dir.z;

    /*hit_t shadowHit = traceObjects(Ph, normalise(pointSub(alight.pos,Ph)), spheres);
    if (shadowHit.hit)
        return {0,0,0};*/

    vec_t V = pointSub(Ph, src);
    light_t alight = {{0,0,8}, {255,255,255}, 1};
    vec_t L = pointSub(Ph, alight.pos);
    vec_t H = vecAddition(V, L);
    vec_t N = normalise(pointSub(obj.centre, Ph));

    float lpointl = dotprod(L, L);
    float brightll = alight.brightness/lpointl;
    color_t incidentLight;
    incidentLight.r = alight.col.r*brightll;
    incidentLight.g = alight.col.g*brightll;
    incidentLight.b = alight.col.b*brightll;

    //V = {V.x*-1, V.y*-1, V.z*-1};
    //printf("%f, %f, %f \n", incidentLight.r,incidentLight.g,incidentLight.b);
    H = normalise(H);

    N.x/=obj.r;
    N.y/=obj.r;
    N.z/=obj.r;
    float ndoth = dotprod(N, H);
    if (ndoth < 0)
        return {0,0,0};
    color_t reflectivity;
    reflectivity.r= ndoth*obj.color.r;
    reflectivity.g = ndoth*obj.color.g;
    reflectivity.b = ndoth*obj.color.b;
    color_t reflectedLight;
    reflectedLight.r = incidentLight.r*reflectivity.r;
    reflectedLight.g = incidentLight.g*reflectivity.g;
    reflectedLight.b = incidentLight.b*reflectivity.b;
    return reflectedLight;
    //float NV=dotprod(N, V);
    //if (NV<0)
        //NV*=-1;
    //return NV;
}

void traceObjects (SDL_Renderer *renderer, sphere_t*spheres, int WindowWidth, int WindowHeight)
{
    point_t origin = {0,0,0};
    float x;
    float y;
    vec_t direction;
    float hDist;
    sphere_t obj;
    //goes through every pixel one at a time
    for(x=0; x<WindowWidth;x++)
    {
        for(y=0; y<WindowHeight;y++)
        {   
            hit_t closestHit;
            closestHit.hit = 0;
            closestHit.dist = 9999;
            for(int i= 0; spheres[i].r; i++)
            {
                direction = {x-WindowWidth/2, y-WindowHeight/2,500}; //direction of ray
                direction = normalise(direction);
                hit_t h = traceObj(origin, direction, spheres[i]); //calls trace obj, finds if the obj has been hit
                if(h.hit)
                {
                    if(h.dist < closestHit.dist) //if the distance is the closest, set it as the closest
                    {
                        obj = h.obj;
                        hDist=h.dist;
                        closestHit.hit = 1;
                        closestHit.dist = h.dist;
                    }
                }
            }
            if(closestHit.hit) //shade the closest hit
            {
                color_t reflectedLight = shade(origin, direction, obj, hDist);
                //printf("%f, %f, %f \n", reflectedLight.r,reflectedLight.g,reflectedLight.b);
                if (reflectedLight.r>255)
                    reflectedLight.r = 255;
                if (reflectedLight.g>255)
                    reflectedLight.g = 255;
                if (reflectedLight.b>255)
                    reflectedLight.b = 255;
                       putPixel(renderer,x,y,reflectedLight.r,reflectedLight.g,reflectedLight.b); //finds shaded colour value
            }
        }
    }
}

auto main() -> int
{
    constexpr int WindowWidth = 700;
    constexpr int WindowHeight = 700;
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;

    std::random_device rd;
    std::mt19937 gen(rd());
    // create a distribution to produce our 2d points
    std::uniform_int_distribution<> width_dist(0, WindowWidth - 1);
    std::uniform_int_distribution<> height_dist(0, WindowHeight - 1);
    // create a distribution to produce our colour range
    // note windows does not allow an unsigned char dist see
    // https://stackoverflow.com/questions/31460733/why-arent-stduniform-int-distributionuint8-t-and-stduniform-int-distri
    std::uniform_int_distribution<unsigned short> colour_dist(0, 255);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WindowWidth, WindowHeight, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Sphere Light");
    // clear to background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    bool quit = false;

    // clear to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    sphere_t spheres[]={{{0,5,20}, 5, {0,255,0}}, {{0,0,20}, 5, {255,255,255}}, {{0,-5,20}, 5, {255,0,0}}, {{0,0,0}, 0, {0,0,0}}};
    traceObjects(renderer, spheres, WindowWidth, WindowHeight);

    while (!quit)
    {

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            // this is the window x being clicked.
            case SDL_QUIT:
                quit = true;
                break;
            // now we look for a keydown event
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
                {
                // if it's the escape key quit
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default:
                    break;
                } // end of key process
            }     // end of keydown
            break;
            default:
                break;
            } // end of event switch
        }     // end of poll events

        // flip buffers
        SDL_RenderPresent(renderer);
        // wait 100 ms
        SDL_Delay(100);
    }
    // clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
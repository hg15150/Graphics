#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <omp.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

SDL_Event event;

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512
// #define SCREEN_WIDTH 1920
// #define SCREEN_HEIGHT 1080

#define FULLSCREEN_MODE false
vec4 cameraPos(0,0,-3,1);
int rotL=0;
int rotU=0;

vec4 lightPos( 0, -0.5, -0.7, 1.0 );
vec3 lightColor = 14.f * vec3( 1, 1, 1 );


struct Intersection
  {
  vec4 position;
  float distance;
  int triangleIndex;
  };

//Intersection* Intersections = NULL;

// -----------------------------------------------------------------------------

bool Update(vector<Triangle>& triangles);
void Draw(screen* screen,vector<Triangle>& triangles);
bool ClosestIntersection(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection );
float calcDistance(vec3 start, vec3 intersection);
vec3 DirectLight( const Intersection& i, vector<Triangle>& triangles);
bool xChecker(vec3 x);
void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation);

// -----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  vector<Triangle> triangles; //Array of all triangles in the image
  LoadTestModel( triangles );
  //Intersections =  (Intersection*)malloc(sizeof(Intersection) * (SCREEN_WIDTH*SCREEN_HEIGHT));
    // Draw(screen);
    // SDL_Renderframe(screen);
    while ( Update(triangles))
    {
      Draw(screen,triangles);
      SDL_Renderframe(screen);
    }


  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<Triangle>& triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  float focalLength = SCREEN_HEIGHT; //I am not perfectly certain why this works better than SCREEN_WIDTH but it does.
            //This is the start for each ray (backwards raytracing).

  //Optimisations to reduce the amount of divisions within loop
  int halfScreenWidth = SCREEN_WIDTH/2;
  int halfScreenHeight = SCREEN_HEIGHT/2;

  #pragma omp parallel for
  for(int i=0.f; i<SCREEN_WIDTH; i++){
    for (float j = 0.f; j < SCREEN_HEIGHT; j++) {

      //Calculate ray direction { d = x - W/2, y - H/2, f, 1}
      vec4 rayDirection((float)i - halfScreenWidth, j - halfScreenHeight, focalLength, 1);

      //Calculate closestIntersection
      Intersection intersection;
      bool isIntersection = ClosestIntersection(cameraPos, rayDirection, triangles, intersection);
      //Intersections[j + i*SCREEN_WIDTH] = intersection;
      vec3 brightness(0.0, 0.0, 0.0);
      vec3 indirectLight = 0.2f*vec3( 1, 1, 1 );

      if(intersection.triangleIndex < (float) triangles.size()){
        brightness = DirectLight(intersection,triangles);
      }

      brightness += indirectLight;

      vec3 colour(0.0, 0.0, 0.0); //Set initial colour of pixel to black

      if(isIntersection){
        //Get intersected triangle
        Triangle closestIntersectedTriangle = triangles[intersection.triangleIndex];

        colour = brightness*closestIntersectedTriangle.color;

        // colour = brightness*closestIntersectedTriangle.color * vec3(abs(tan(i*j)));
      }

      PutPixelSDL(screen, i, j, colour);
    }
  }
}

/*Place updates of parameters here*/
bool Update(vector<Triangle>& triangles)
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;

  std::cout << "Render time: " << dt << " ms." << std::endl;


  SDL_Event e;
  while(SDL_PollEvent(&e))
    {
      if (e.type == SDL_QUIT)
	{
	  return false;
	}
      else
	if (e.type == SDL_KEYDOWN)
	  {
	    int key_code = e.key.keysym.sym;
	    switch(key_code)
	      {
	      case SDLK_w:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,-0.1f,1.f));
		/* Move camera forward */
		break;
	      case SDLK_s:
  rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,0.1f,1.f));		/* Move camera backwards */
		break;
	      case SDLK_a:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (0.1f,0.f,0.f,1.f));		/* Move camera backwards */
		/* Move camera left */
		break;
	      case SDLK_d:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (-0.1f,0.f,0.f,1.f));		/* Move camera backwards */

    break;
        case SDLK_UP:
        //lightPos.z--;
         rotateCamera(vec4 (0.0785398f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));
           /* Move camera forward */
    break;
      case SDLK_DOWN:
      //lightPos.z++;
      rotateCamera(vec4 (-0.0785398f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));

         /* Move camera backwards */
    break;
      case SDLK_LEFT:
      rotateCamera(vec4 (0.f,-0.0785398f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));
      //lightPos.x++;
/* Move camera left */
   break;
      case SDLK_RIGHT:
      //lightPos.x--;
      rotateCamera(vec4 (0.f,0.0785398f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));
    break;
		/* Move camera right */
	      case SDLK_ESCAPE:
		/* Move camera quit */
		return false;
	      }
	  }
    }
  return true;
}

vec3 DirectLight( const Intersection& i, vector<Triangle>& triangles){
   // printf("Intersection value = %d\n",i.triangleIndex );
   vec4 normal = triangles[i.triangleIndex].normal; //Surface normal
   float r = glm::distance(lightPos, i.position);   //Distance from light source to Intersection
   vec4 reflection = (lightPos - i.position) / r;  //Unit vector of reflection

   Intersection newIntersection;
   bool isIntersection = ClosestIntersection(i.position + (reflection/1000.f), reflection, triangles, newIntersection);

   if(r > newIntersection.distance && isIntersection) return vec3(0,0,0);

   else return ((lightColor * max(dot(reflection, normal), 0.f)) / (float)(4*3.1415*pow(r,2)));
}


void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation){
  float x = rotation.x;
  float y = rotation.y;
  float z = rotation.z;
  vec4 col1(cos(y)*cos(z), cos(y)*sin(z),-sin(y),0);
  vec4 col2(-cos(x)*sin(z)+sin(x)*sin(y)*cos(z),cos(x)*cos(z)+sin(x)*sin(y)*sin(z),sin(x)*cos(y),0);
  vec4 col3(sin(x)*sin(z)+cos(x)*sin(y)*cos(z),-sin(x)*cos(z)+cos(x)*sin(y)*sin(z),cos(x)*cos(y),0);


  mat4 rotationMatrix(col1,col2,col3,translation);
  for (size_t i = 0; i < triangles.size(); i++) {

    //Extract triangle vertices




    triangles[i].v0 = rotationMatrix*triangles[i].v0;
    triangles[i].v1 = rotationMatrix*triangles[i].v1;
    triangles[i].v2 = rotationMatrix*triangles[i].v2;
  }
  lightPos = rotationMatrix*lightPos;

}

bool xChecker(vec3 x){
  return ( (x.x >= 0) && (x.y >= 0) && (x.z >= 0) && ( (x.y + x.z) < 1) );
}

bool ClosestIntersection(vec4 s, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection ){

  bool intersectionOccurred = false;
  closestIntersection.distance = std::numeric_limits<float>::max();

  for (size_t i = 0; i < triangles.size(); i++) {
    // closestIntersection.triangleIndex = -1;

    //Extract triangle vertices
    vec4 v0 = triangles[i].v0;
    vec4 v1 = triangles[i].v1;
    vec4 v2 = triangles[i].v2;

    //Determine axis of the plane that the triangle lies within
    vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
    vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);

    //Calculate start of ray - vertex 0
    vec3 b = vec3(s.x-v0.x,s.y-v0.y,s.z-v0.z);

    //Create matrix
    mat3 A( -vec3(dir), e1, e2 );

    //Calculate Intersection
    vec3 x = inverse(A) * b;

    //If Intersection is within triangle, calculate distance
    if(xChecker(x)){
      intersectionOccurred = true; //At least one intersection occurred
      vec3 start(s.x, s.y, s.z);   //Convert start vector to 3D
      // float distance = glm::distance(start, x);
      float distance = x.x;

      //Overwrite closestIntersection
      if(distance < closestIntersection.distance){
        closestIntersection.position =  s + distance*dir;
        closestIntersection.distance = distance;
        closestIntersection.triangleIndex = i;
      }
    }
  }

return intersectionOccurred;
}

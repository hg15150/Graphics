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

// #define SCREEN_WIDTH 32
// #define SCREEN_HEIGHT 25
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512
// #define SCREEN_WIDTH 1280
// #define SCREEN_HEIGHT 1024

#define NUMBEROFLIGHTS 1

#define FULLSCREEN_MODE false
vec4 cameraPos(0,0,-3,1);
int rotL=0;
int rotU=0;

vec4 lightPos( 0, -0.5, -0.7, 1.0 );
vec4 lightPosX;
int sizeOfLight = 2;
float lightHeight = 0.2;
vec3 lightColor = 25.f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.2f*vec3( 1, 1, 1 );

vec4 tmpNormal = vec4(-1);

vector<Item*> triangles;

struct Intersection
  {
     vec4 position;
     float distance;
     uint index;
  };

// -----------------------------------------------------------------------------

bool Update();
void Draw(screen* screen);
bool ClosestIntersection(vec4 start, vec4 dir, Intersection& closestIntersection );
// bool ClosestIntersectionMirror(vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection, uint ignore );
float calcDistance(vec3 start, vec3 intersection);
vec3 DirectLight( Intersection& );
bool xChecker(vec3 x);
void rotateCamera(vec4 rotation, vec4 translation);
void moveLight(vec4 rotation,  vec4 translation);
// float Specular(Intersection& i, Material material);
// void SetReflection(Intersection& i);
// vec3 TheBigDaddy(Intersection& i, Material material, vector<Triangle>& triangles);
// vec3 TheBigReflectionDaddy(Intersection& i, Material material, vector<Triangle>& triangles);
vec3 calculateColour(Intersection& i);

// -----------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );
  LoadTestModel( triangles );

    while ( Update()){
      Draw(screen);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  float focalLength = SCREEN_HEIGHT;
  int halfScreenWidth = SCREEN_WIDTH/2;
  int halfScreenHeight = SCREEN_HEIGHT/2;

  #pragma omp parallel for
  for(int i=0; i<SCREEN_WIDTH; i++){
    for (int j = 0; j < SCREEN_HEIGHT; j++){
      //Calculate ray direction { d = x - W/2, y - H/2, f, 1}
      vec4 rayDirection( (float) i - halfScreenWidth, (float) j - halfScreenHeight, focalLength, 1);
      vec3 colour(0.0, 0.0, 0.0); //Set inititrianglestrianglesal colour of pixel to black

      Intersection intersection;
      bool isIntersection = ClosestIntersection(cameraPos, rayDirection, intersection);

      if(isIntersection){
         colour = calculateColour(intersection);
      }
      // else printf("Out\n");
      PutPixelSDL(screen, i, j, colour);
    }
  }
}

/*Place updates of parameters here*/
bool Update()
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
            case SDLK_s:
           moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,0.f,-0.1f,1.f));
        /* Move camera forward */
        break;
           case SDLK_w:
           moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,0.f,0.1f,1.f));		/* Move camera backwards */
        break;
           case SDLK_d:
           moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (0.1f,0.f,0.f,1.f));		/* Move camera backwards */
        /* Move camera left */
        break;
           case SDLK_a:
           moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (-0.1f,0.f,0.f,1.f));		/* Move camera backwards */

       break;
       case SDLK_q:
            moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,-0.1f,0.f,1.f));		/* Move camera backwards */

       break;
      case SDLK_e:
      moveLight(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,0.1f,0.f,1.f));		/* Move camera backwards */

      break;
	      case SDLK_i:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,0.f,-0.1f,1.f));
		/* Move camera forward */
		break;
	      case SDLK_k:
  rotateCamera(vec4 (0.f,0.f,0.f,1.f), vec4 (0.f,0.f,0.1f,1.f));		/* Move camera backwards */
		break;
	      case SDLK_j:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), vec4 (0.1f,0.f,0.f,1.f));		/* Move camera backwards */
		/* Move camera left */
		break;
	      case SDLK_l:
        rotateCamera(vec4 (0.f,0.f,0.f,1.f), vec4 (-0.1f,0.f,0.f,1.f));		/* Move camera backwards */

    break;
        case SDLK_UP:
        //lightPos.z--;
         rotateCamera(vec4 (0.0785398f,0.f,0.f,1.f), vec4 (0.f,0.f,0.f,1.f));
           /* Move camera forward */
    break;
      case SDLK_DOWN:
      //lightPos.z++;
      rotateCamera(vec4 (-0.0785398f,0.f,0.f,1.f), vec4 (0.f,0.f,0.f,1.f));

         /* Move camera backwards */
    break;
      case SDLK_LEFT:
      rotateCamera(vec4 (0.f,-0.0785398f,0.f,1.f), vec4 (0.f,0.f,0.f,1.f));
      //lightPos.x++;
/* Move camera left */
   break;
      case SDLK_RIGHT:
      //lightPos.x--;
      rotateCamera(vec4 (0.f,0.0785398f,0.f,1.f), vec4 (0.f,0.f,0.f,1.f));
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

vec3 DirectLight( Intersection& i){
   vec4 normal = triangles[i.index]->computeNormal(i.position); //Surface normal

   float r = glm::distance(lightPos, i.position);   //Distance from light source to Intersection
   vec4 reflection = (lightPos - i.position) / r;  //Unit vector of reflection

   Intersection newIntersection;
   bool isIntersection = ClosestIntersection(i.position + reflection*0.00001f, reflection, newIntersection);

   // printf("%.2f %.2f %.2f\n", lightColor.x, lightColor.y, lightColor.z);
   // printf("Ref: %.2f %.2f %.2f\n", reflection.x, reflection.y, reflection.z);
   // printf("Nor: %.2f %.2f %.2f\n", normal.x, normal.y, normal.z);


   if(r > newIntersection.distance && isIntersection) return vec3(0,0,0);
   else return (1.f/(float)(sizeOfLight * sizeOfLight)) * (lightColor * max(dot(reflection, normal), 0.f)) / (float) (4*PI*pow(r,2));
}

void rotateCamera(vec4 rotation, vec4 translation){
  float x = rotation.x;
  float y = rotation.y;
  float z = rotation.z;
  vec4 col1(cos(y)*cos(z), cos(y)*sin(z),-sin(y),0);
  vec4 col2(-cos(x)*sin(z)+sin(x)*sin(y)*cos(z),cos(x)*cos(z)+sin(x)*sin(y)*sin(z),sin(x)*cos(y),0);
  vec4 col3(sin(x)*sin(z)+cos(x)*sin(y)*cos(z),-sin(x)*cos(z)+cos(x)*sin(y)*sin(z),cos(x)*cos(y),0);


  mat4 rotationMatrix(col1,col2,col3,translation);
  mat4 rotationMatrixNoTrans(col1,col2,col3,vec4(0,0,0,1));

  for (uint i = 0; i < triangles.size(); i++) {
     triangles[i]->rotate(rotationMatrix);
  }
  lightPos = rotationMatrix*lightPos;

}

void moveLight(vec4 rotation ,vec4 translation){
  float x = rotation.x;
  float y = rotation.y;
  float z = rotation.z;
  vec4 col1(cos(y)*cos(z), cos(y)*sin(z),-sin(y),0);
  vec4 col2(-cos(x)*sin(z)+sin(x)*sin(y)*cos(z),cos(x)*cos(z)+sin(x)*sin(y)*sin(z),sin(x)*cos(y),0);
  vec4 col3(sin(x)*sin(z)+cos(x)*sin(y)*cos(z),-sin(x)*cos(z)+cos(x)*sin(y)*sin(z),cos(x)*cos(y),0);


  mat4 rotationMatrix(col1,col2,col3,translation);

  lightPos = rotationMatrix*lightPos;

}

bool xChecker(vec3 x){
  return ( (x.x >= 0) && (x.y >= 0) && (x.z >= 0) && ( (x.y + x.z) < 1) );
}

bool ClosestIntersection(vec4 s, vec4 dir, Intersection& closestIntersection ){

   bool intersectionOccurred = false;
   closestIntersection.distance = std::numeric_limits<float>::max();
   float t = closestIntersection.distance;
   vec4 p = vec4(0.f, 0.f, 0.f, 1.f);

   for (uint j = 0; j < triangles.size(); j++) {
      if(triangles[j]->intersection(s, dir, t, p)){
         if(t < closestIntersection.distance){
            intersectionOccurred = true;
            closestIntersection.index = j;
            closestIntersection.distance = t;
            closestIntersection.position = p;
         }
      }
   }

   return intersectionOccurred;
}

// bool ClosestIntersectionMirror(vec4 s, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection, uint ignore ){
//
//   bool intersectionOccurred = false;
//   closestIntersection.distance = std::numeric_limits<float>::max();
//
//    for (size_t i = 0; i < triangles.size(); i++) {
//       if(i == ignore) continue;
//
//       vec4 v0 = triangles[i].v0;
//       vec4 v1 = triangles[i].v1;
//       vec4 v2 = triangles[i].v2;
//
//       vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
//       vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
//       vec3 b = vec3(s.x-v0.x,s.y-v0.y,s.z-v0.z);
//
//       mat3 A( -vec3(dir), e1, e2 );
//       mat3 Ax( b , e1, e2 );
//
//       float determinantA = glm::determinant(A);
//       float determinantAx = glm::determinant(Ax);
//
//       float x = determinantAx/determinantA;
//       if(x < 0) continue;
//
//       // vec3 xy = inverse(A) * b;
//
//       mat3 Ay( -vec3(dir) , b, e2 );
//       float determinantAy = glm::determinant(Ay);
//       float y = determinantAy/determinantA;
//
//       if(y >= 0){
//          mat3 Az( -vec3(dir) , e1, b );
//          float determinantAz = glm::determinant(Az);
//          float z = determinantAz / determinantA;
//          if ((z >= 0) && ( (y + z) < 1)){
//             intersectionOccurred = true; //At least one intersection occurred
//             float distance = x;
//             if(distance < closestIntersection.distance){
//                closestIntersection.position =  s + distance*dir;
//                closestIntersection.distance = distance;
//                closestIntersection.triangleIndex = i;
//             }
//          }
//       }
//    }
//    return intersectionOccurred;
// }

float Specular(Intersection& i, Material material){
   // return (
   //    (glm::pow(glm::max( glm::dot( i.reflectedRay , glm::normalize(i.cameraRay)), 0.f ), material.shi))
   //    /
   //    4 * PI * glm::pow(glm::distance(lightPos, i.position), 2)
   // );
   return -1.f;
}


vec4 reflect(vec4 normal, vec4 dir){
  return dir - 2 * glm::dot(dir, normal) * normal;
}


vec3 calculateColour(Intersection& i){

   vec3 colour = vec3(0);

   switch (triangles[i.index]->material.type) {
      case GlassType:
         // colour = TheBigDaddy(intersection, Glass, triangles);
         break;

      case RoughType:
         // printf("Light: %.2f %.2f %.2f\n", DirectLight(i).x, DirectLight(i).y, DirectLight(i).z);
         colour = triangles[i.index]->colour * (DirectLight(i) + indirectLight);
         break;

      case MirrorType:

         // colour = TheBigReflectionDaddy(intersection, Mirror, triangles);
   break;
   }

   return colour;
}

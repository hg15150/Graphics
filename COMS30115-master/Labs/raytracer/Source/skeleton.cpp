#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <omp.h>
#include <math.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::normalize;

SDL_Event event;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 250
// #define SCREEN_WIDTH 640
// #define SCREEN_HEIGHT 512
// #define SCREEN_WIDTH 1280
// #define SCREEN_HEIGHT 1024

#define NUMBEROFLIGHTS 1

#define FULLSCREEN_MODE true
vec4 cameraPos(0,0,-3,1);
int rotL=0;
int rotU=0;

vec4 lightPos( 0, -0.5, -0.7, 1.0 );
// vec4 lightPosX;
int sizeOfLight = 5;
float lightHeight = 0.1;
vec3 lightColor = 15.f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.8f*vec3( 1, 1, 1 );

int tmp = 0;

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
bool ClosestIntersection(vec4 start, vec4 dir, Intersection& closestIntersection, int index );
float calcDistance(vec3 start, vec3 intersection);
vec3 DirectLight( Intersection& i, vec4 incidentRay);
bool xChecker(vec3 x);
void rotateCamera(vec4 rotation, vec4 translation);
void moveLight(vec4 rotation,  vec4 translation);
vec3 calculateColour(Intersection& i, vec4 incidentRay, int depth);
vec3 mirror(Intersection& i, vec4 dir, int depth);
float Specular(Intersection i, vec4 incidentRay, vec4 reflectedRay, float r);
vec4 refract(vec4 I, vec4 N);
vec3 glass(Intersection i, vec4 dir, int depth);



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

  // #pragma omp parallel for
  for(int i=0; i<SCREEN_WIDTH; i++){
    for (int j = 0; j < SCREEN_HEIGHT; j++){
      //Calculate ray direction { d = x - W/2, y - H/2, f, 1}
      vec4 rayDirection( (float) i - halfScreenWidth, (float) j - halfScreenHeight, focalLength, 1);
      vec3 colour(0.0, 0.0, 0.0); //Set inititrianglestrianglesal colour of pixel to black

      Intersection intersection;
      bool isIntersection = ClosestIntersection(cameraPos, rayDirection, intersection, -1);

      if(isIntersection){
         colour = calculateColour(intersection, rayDirection, 10);
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

vec3 DirectLight( Intersection& i, vec4 incidentRay ){
   vec4 normal = triangles[i.index]->computeNormal(i.position); //Surface normal
   vec3 brightness = vec3(0);
   float sectionSize = lightHeight/sizeOfLight;
   float a = triangles[i.index]->material.spec;
   float b = triangles[i.index]->material.diff;

   for (int x = 0; x < sizeOfLight; x++) {
      for (int y = 0; y < sizeOfLight; y++) {
         vec4 lightPosX = vec4( lightPos.x - (0.5 * lightHeight) + x * sectionSize, lightPos.y - (0.5 * lightHeight) + y * sectionSize, lightPos.z, 1.0 );

         float r = glm::distance(lightPosX, i.position);   //Distance from light source to Intersection
         vec4 reflection = (lightPosX - i.position) / r;  //Unit vector of reflection

         Intersection newIntersection;
         bool isIntersection = ClosestIntersection(i.position, reflection, newIntersection, i.index);

         vec4 optimalReflection = reflect(normal, reflection);

         if(r > newIntersection.distance && isIntersection) brightness += vec3(0,0,0);
         else brightness += a * vec3(Specular(i, incidentRay, optimalReflection, r)) + b * ((1.f/(float)(sizeOfLight * sizeOfLight)) * (lightColor * max(dot(reflection, normal), 0.f)) / (float) (4*PI*pow(r,2)));
      }
   }
   return brightness;
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

bool ClosestIntersection(vec4 s, vec4 dir, Intersection& closestIntersection , int index){

   bool intersectionOccurred = false;
   closestIntersection.distance = std::numeric_limits<float>::max();
   float t = closestIntersection.distance;
   vec4 p = vec4(0.f, 0.f, 0.f, 1.f);

   for (uint j = 0; j < triangles.size(); j++) {
      if(j == index) continue;
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

float Specular(Intersection i, vec4 incidentRay, vec4 reflectedRay, float r){
   if(triangles[i.index]->material.type == GlossType){
      return (
         1000.f * (glm::pow(glm::max( glm::dot( reflectedRay, incidentRay ), 0.f ), 100))
         /
         (4 * PI * glm::pow(r, 2))
      );
   }
   else return 0.f;

}

vec4 reflect(vec4 normal, vec4 dir){
  return dir - 2 * glm::dot(dir, normal) * normal;
}

vec3 mirror(Intersection& i, vec4 dir, int depth){
   depth--;

   vec4 normal = triangles[i.index]->computeNormal(i.position);
   vec4 reflectedRay = normalize(reflect(normal , dir));



   // printf("n: %.2f %.2f %.2f \n", normal.x, normal.y, normal.z );
   // printf("d: %.2f %.2f %.2f \n", dir.x, dir.y, dir.z );
   // printf("r: %.2f %.2f %.2f \n", reflectedRay.x, reflectedRay.y, reflectedRay.z );

   Intersection newIntersection;
   bool isIntersection = ClosestIntersection(i.position, reflectedRay, newIntersection, i.index);
   if(isIntersection){
       reflectedRay = glm::reflect( triangles[newIntersection.index]->computeNormal(newIntersection.position) , reflectedRay);
       // printf("r: %.2f %.2f %.2f \n", reflectedRay.x, reflectedRay.y, reflectedRay.z );

      return (depth > 0) ? calculateColour(newIntersection, reflectedRay, depth) : vec3(0.3);
   }
   else {
      return vec3(0);
   }
}

vec4 refract (vec4 dir, vec4 normal) {
  float cosi = glm::clamp(-1.f,1.f,glm::dot(dir, normal));
  float n1 = 1.5f, n2 = 1.f;
  if (cosi < 0) cosi = -cosi;
  else {
    std::swap(n1, n2);
    normal = -normal;
  }
  float n = n1 / n2;
  float k = 1 - n * n * (1 - cosi * cosi);
  return (k < 0) ? vec4() : n * dir + (n * cosi - sqrtf(k)) * normal;
}

float fresnel(vec4 I, vec4 N) {
   float cosi = glm::clamp(-1.f, 1.f, glm::dot(I, N));
   float n1 = 1.3f;
   float n2 = 1.f;
   float kr;

   if (cosi > 0) std::swap(n1, n2);

   // Compute sini using Snell's law
   float sint = n1 / n2 * sqrtf(std::max(0.f, 1 - cosi * cosi));

   // Total internal reflection
   if (sint >= 1) kr = 1.f;
   else {
      float cost = sqrtf(std::max(0.f, 1 - sint * sint));
      cosi = fabsf(cosi);
      float Rs = ((n2 * cosi) - (n1 * cost)) / ((n2 * cosi) + (n1 * cost));
      float Rp = ((n1 * cosi) - (n2 * cost)) / ((n1 * cosi) + (n2 * cost));
      kr = (Rs * Rs + Rp * Rp) / 2;
   }
   return kr;
}

vec3 glass(Intersection i, vec4 dir, int depth) {

   vec4 normal = triangles[i.index]->computeNormal(i.position);
   vec3 refractedColour = vec3(0);
   vec3 reflectedColour = vec3(0);

   // //Fresnel
   float kr = fresnel(dir, normal);

   //Refraction
   if(kr < 1){
      vec4 refractedRay = normalize(refract(dir, normal));
      Intersection newIntersection;
      bool isIntersection = ClosestIntersection(i.position, refractedRay, newIntersection, i.index);
      refractedColour = isIntersection ? calculateColour(newIntersection, refractedRay, depth) : vec3(0);
   }

   vec4 reflection = normalize(reflect(normal, dir));

   Intersection intersection;
   if(ClosestIntersection(i.position, reflection, intersection, i.index)){
     reflectedColour = calculateColour(intersection, reflection, depth + 1);
   }

   return 0.8f * (reflectedColour * kr  +  refractedColour * (1 - kr));
}

vec3 calculateColour(Intersection& i, vec4 incidentRay, int depth){

   vec3 colour = vec3(0);

   incidentRay = normalize(incidentRay);

   switch (triangles[i.index]->material.type) {
      case GlossType:
         colour = 0.95f * triangles[i.index]->colour * (DirectLight(i,incidentRay) + indirectLight) + 0.03f * mirror(i, incidentRay, depth);
         break;

      case RoughType:
         colour = 0.9f * triangles[i.index]->colour * (DirectLight(i, incidentRay) + indirectLight);
         break;


      case MirrorType:
         colour = mirror(i, incidentRay, depth);
         break;

      case GlassType:
         colour = 0.9f * glass(i, incidentRay, depth);
         break;
   }

   return colour;
}

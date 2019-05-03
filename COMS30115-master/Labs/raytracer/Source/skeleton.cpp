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

#define FULLSCREEN_MODE false
vec4 cameraPos(0,0,-3,1);
int rotL=0;
int rotU=0;

vec4 lightPos( 0, -0.5, -0.7, 1.0 );
// vec4 lightPosX;
int sizeOfLight = 5;
float lightHeight = 0.1;
vec3 lightColor = 15.f * vec3( 1, 1, 1 );
vec3 indirectLight = 0.8f*vec3( 1, 1, 1 );

float tmp0 = 0.f;
float tmp1 = 1.f;
float tmp2 = 1.f;
float tmp3 = 1.f;
float tmpTick = 0.f;
float quartPI = PI / 256; //Use for tmp3
// float quartPI = PI / 64;
// float quartPI = PI * 250;

vector<Item*> triangles;

Sphere *sphere;

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
  vec3 white(  0.75f, 0.75f, 0.75f );
  sphere = new Sphere( white, Fancy, vec4( -0.5, 0.5, -0.5, 1 ), 0.1, 3 );
  triangles.push_back(sphere);

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

  // tmpTick += 1.f;

  printf("%.2f\n", tmpTick);

  tmp0 = abs( sin( tmpTick ) );               //Bottom right
  tmp1 = tan( tmpTick );              //Bottom left
  // tmp2 = abs ( cos( tmpTick ) * sin ( tmpTick ) );  //Top right [stable for filter]
  tmp3 = abs( tan( exp (tmpTick) ) );                 //Top left
  tmp2 += quartPI;




  //Flavour
  tmpTick += quartPI;
  // tmp0 = abs( tan ( tmp0 + 0.01f ) );               //Bottom right
  // tmp1 = abs ( sin( tmp0 + tmpTick ) );              //Bottom left
  // tmp2 = abs ( cos( tmpTick ) * sin ( tmpTick ) );  //Top right [stable for filter]
  // tmp3 = abs( sin( tmp3 + 0.1f ) );                 //Top left


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
         // else {
         //    float spec = Specular(i, incidentRay, optimalReflection, r);
         //    brightness += a * vec3( tan(spec*spec) , tan(tan(spec)), tan(spec))
         //       + b * ((1.f/(float)(sizeOfLight * sizeOfLight)) * (lightColor * max(dot(reflection, normal), 0.f)) / (float) (4*PI*pow(r,2)));
         // }
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
   vec4 reflectedRay = reflect(normal , dir);

   // printf("n: %.2f %.2f %.2f \n", normal.x, normal.y, normal.z );
   // printf("d: %.2f %.2f %.2f \n", dir.x, dir.y, dir.z );
   // printf("r: %.2f %.2f %.2f \n", reflectedRay.x, reflectedRay.y, reflectedRay.z );

   Intersection newIntersection;
   bool isIntersection = ClosestIntersection(i.position, reflectedRay, newIntersection, i.index);
   if(isIntersection){
       vec4 normal = triangles[newIntersection.index]->computeNormal(newIntersection.position);
       reflectedRay = reflect( normal , reflectedRay);
       // printf("r: %.2f %.2f %.2f \n", reflectedRay.x, reflectedRay.y, reflectedRay.z );


      return (depth > 0) ? calculateColour(newIntersection, reflectedRay, depth) : vec3(0.3);
   }
   else {
      return vec3(0);
   }
}

vec4 refract (vec4 dir, vec4 normal) {
  float cosi = glm::clamp(-1.f,1.f,glm::dot(dir, normal));
  float n1 = 1.517f, n2 = 1.0003f;
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
   float n1 = 1.517f;
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

vec3 average(vec3 colour){
   float a = colour.x;
   float b = colour.y;
   float c = colour.z;
   float av = (a+b+c) / 3;
   return vec3(av);
}

vec3 glass(Intersection i, vec4 dir, int depth) {

   vec4 normal = triangles[i.index]->computeNormal(i.position);
   vec3 refractedColour = vec3(0);
   vec3 reflectedColour = vec3(0);

   vec4 dir_norm = normalize(dir);

   // //Fresnel
   float kr = fresnel( normal, dir_norm );

   //Refraction
   if(kr < 1){
      vec4 refractedRay = refract(dir_norm, normal);
      Intersection newIntersection;
      bool isIntersection = ClosestIntersection(i.position, refractedRay, newIntersection, i.index);
      refractedColour = isIntersection ? calculateColour(newIntersection, refractedRay, depth) : vec3(0);
   }

   vec4 reflection = reflect(normal, dir);

   Intersection intersection;
   if(ClosestIntersection(i.position, reflection, intersection, i.index)){
     reflectedColour = calculateColour(intersection, reflection, depth - 1);
   }

   return 0.95f * (reflectedColour * kr  +  refractedColour * (1 - kr));
}

vec3 filterFlavour(Intersection& i, float tmp, vec4 normal){
   normal = glm::abs(normal);
   float x = tan(normal.x);
   float y = tan(normal.y);
   float z = tan(normal.z);
   return vec3(x, y, z);
}

vec3 filterFlavour1(Intersection& i, float tmp){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = tan(normal.x);
   float y = tan(normal.y);
   float z = tan(normal.z);
   return vec3(x, y, z);
}

vec3 filterFlavourMove(Intersection& i, float tmp){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = tan(normal.x + tmp);
   float y = tan(normal.y + tmp);
   float z = tan(normal.z + tmp);
   return vec3(x, y, z);
}

vec3 filterFast(Intersection& i, float tmp){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = tan( exp( normal.x + tmp ) * normal.y );
   float y = tan( exp( normal.y + tmp ) * normal.z );
   float z = tan( exp( normal.z + tmp ) * normal.x );
   return vec3(x, y, z);
}

vec3 filter(Intersection& i, float tmp){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = abs( sin( normal.x + tmp ) );
   float y = abs( cos( normal.y + tmp ) );
   float z = abs( tan( normal.z + tmp ) );
   return vec3(x, y, z);
}

vec3 filter2(Intersection& i, float tmp){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = abs( tan( normal.x * i.position.x + tmp ) );
   float y = abs( tan( normal.y * i.position.y + tmp ) );
   float z = abs( tan( normal.z * i.position.z + tmp ) );
   return vec3(x, y, z);
}

vec3 filterSphere(Intersection& i, float tmp, vec4 normal){
   normal = glm::abs(normal);
   float x = abs( tan( normal.x * i.position.x + tmp ) );
   float y = abs( tan( normal.y * i.position.y + tmp ) );
   float z = abs( tan( normal.z * i.position.z + tmp ) );
   return vec3(x, y, z);
}

vec3 filterTmp0( Intersection& i){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   float x = tan( normal.x + tmp2 );
   float y = tan( normal.y + tmp2 );
   float z = tan( normal.z + tmp2 );
   return vec3(x, y, z);
}

vec3 filterTmp1( Intersection& i){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = abs( tan (normal.x + tmp1 * normal.z) );
   float y = abs( tan (normal.y + tmp1 * normal.z) );
   float z = abs( tan (normal.z + tmp1 * normal.z) );
   return vec3(x, y, z);
}

vec3 filterCanvas1( Intersection& i, vec4 normal){
   normal = glm::abs(normal);
   float x = abs( tan ( normal.x + tmp1 * normal.z) );
   float y = abs( tan ( normal.y + tmp1 * normal.z) );
   float z = abs( tan ( normal.z + tmp1 * normal.z) );
   return vec3(x, y, z);
}

vec3 filterTmp2( Intersection& i){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   float x = tan(exp(normal.x + tmp2) * normal.z);
   float y = tan(exp(normal.y + tmp2) * normal.z);
   float z = tan(exp(normal.z + tmp2) * normal.z);
   return vec3(x, y, z);
}

vec3 filterCanvas2( Intersection& i, vec4 normal){
   normal = glm::abs(normal);
   float x = tan(exp(normal.x + tmp2) * normal.z);
   float y = tan(exp(normal.y + tmp2) * normal.z);
   float z = tan(exp(normal.z + tmp2) * normal.z);
   return vec3(x, y, z);
}

vec3 filterTmp3( Intersection& i){
   vec4 normal = triangles[i.index]->computeNormal(i.position);
   normal = glm::abs(normal);
   printf("%.2f\n", tmp3);
   float x = tan(exp(normal.x + tmp3) * normal.y);
   float y = tan(exp(normal.y + tmp3) * normal.z);
   float z = tan(exp(normal.z + tmp3) * normal.x);
   return vec3(x, y, z);
}

vec3 filterCanvas3( Intersection& i, vec4 normal){
   normal = glm::abs(normal);
   float x = tan(exp(normal.x + tmp3) * normal.y);
   float y = tan(exp(normal.y + tmp3) * normal.z);
   float z = tan(exp(normal.z + tmp3) * normal.x);
   return vec3(x, y, z);
}

bool SphereIntersection(Intersection i, Intersection& closestIntersection, int index, vec4& normal){


   float distanceToSphere = glm::distance(i.position, sphere->center);
   vec4 dir = normalize(i.position - sphere->center);

   // printf("distanceToSphere: %.2f\n", distanceToSphere);
   // printf("dir: %.2f %.2f %.2f\n", dir.x, dir.y, dir.z);


   Intersection newIntersection;
   bool isIntersection = ClosestIntersection(i.position, dir, newIntersection, i.index);

   if(isIntersection && newIntersection.distance < distanceToSphere) {
      return false;
   }
   //Therefore closest intersection is the sphere.
   normal = -dir;
   // printf("norm: %.2f %.2f %.2f\n", normal.x, normal.y, normal.z);

   // printf("true\n");
   return true;
}

vec3 discoBall(Intersection& i, float tmp){

   Intersection closestIntersection;
   vec4 normal;
   bool isIntersectionWithSphere = SphereIntersection(i, closestIntersection, i.index, normal);

   if(isIntersectionWithSphere){
      // printf("norm: %.2f %.2f %.2f\n", normal.x, normal.y, normal.z);
      return filterCanvas2(i, normal);
   }

   return vec3(1);

}

vec3 calculateColour(Intersection& i, vec4 incidentRay, int depth){

   vec3 colour = vec3(0);
   incidentRay = normalize(incidentRay);

   switch (triangles[i.index]->material.type) {

      case CanvasType:
      {
         vec3 disco = discoBall(i, tmp3);
         // printf("%.2f %.2f %.2f\n", disco.x, disco.y, disco.z);
         // colour = 0.9f * triangles[i.index]->colour * (DirectLight(i, incidentRay) + indirectLight);
         colour = disco * 0.9f * triangles[i.index]->colour * (DirectLight(i, incidentRay) + indirectLight);
      }
         break;

      case FancyType:
         float tmp;
         switch(triangles[i.index]->getId()){
            case 0: tmp = tmp0;
            break;

            case 1: tmp = tmp1;
            break;

            case 2: tmp = tmp2;
            break;

            case 3: tmp = tmp3;
            break;

            default: tmp = 0.f;
            break;
         }

         colour = filterTmp2(i) * 0.95f * triangles[i.index]->colour * (DirectLight(i,incidentRay) + indirectLight) ;
         break;

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
         colour = 0.96f * glass(i, incidentRay, depth);
         break;
   }

   return colour;
}

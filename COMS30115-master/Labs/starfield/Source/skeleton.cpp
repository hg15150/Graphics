#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false


/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */
int t;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen);
void Interpolate( float a, float b, vector<float>& result );
void InterpolateVec( vec3 a, vec3 b, vector<vec3>& result );


/* ----------------------------------------------------------------------------*/

int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );


  while( NoQuitMessageSDL() )
    {
      Draw(screen);
      // Update();
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

  vec3 topLeft(1,0,0);     //red
  vec3 topRight(0,0,1);    //blue
  vec3 bottomRight(0,1,0); //green
  vec3 bottomLeft(1,1,0);  //yellow

  vector<vec3> leftSide(SCREEN_HEIGHT);  //Array of colours
  vector<vec3> rightSide(SCREEN_HEIGHT); //Array of colours
  InterpolateVec(topLeft, bottomLeft, leftSide);  //Fill in left side
  InterpolateVec(topRight, bottomRight, rightSide);  //Fill in right side

  vector<vec3> rowToDraw (SCREEN_WIDTH); //Array of colours of row

  for(int i=0; i < SCREEN_HEIGHT; i++)
    {
      InterpolateVec(leftSide[i], rightSide[i], rowToDraw);
      for (int j = 0; j < SCREEN_WIDTH; j++) {
        PutPixelSDL(screen, j, i, rowToDraw[j]);
      }
    }
}

/*Place updates of parameters here*/
void Update()
{
  /*
   Compute frame time
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  Good idea to remove this
  std::cout << "Render time: " << dt << " ms." << std::endl;
  Update variables
  */
}

/*
Populate result with evenly distanced values between a and b
*/
void Interpolate( float a, float b, vector<float>& result ){
  float size = result.size();

  if(size == 1) result[0] = b - (b-a)/2;

  else{
    float value = a;
    float incr  = (b-a) / (size-1);
    for (unsigned int i = 0; i < result.size(); i++) {
      result[i] = value;
      value += incr;
    }
  }

}

/*
Vector version
*/
void InterpolateVec( vec3 a, vec3 b, vector<vec3>& result ){

  float size = result.size();
  vec3 diff = b - a;
  vec3 current = a;

  if(size==1) {
    result[0].x = b.x - ((b.x-a.x)/2);
    result[0].y = b.y - ((b.y-a.y)/2);
    result[0].z = b.z - ((b.z-a.z)/2);
  }
  else{
    vec3 incr(diff.x / (size-1), diff.y / (size-1), diff.z / (size-1));

    for (size_t i = 0; i < size; i++) {
      result[i] = current;
      current += incr;
    }
  }

}

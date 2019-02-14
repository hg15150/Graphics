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
  t = SDL_GetTicks();	/*Set start value for timer.*/

  // vector<float> result( 1 ); // Create a vector width 10 floats
  // Interpolate( 5, 14, result ); // Fill it with interpolated values
  // for(unsigned int i=0; i<result.size(); ++i )
  // cout << result[i] << " "; // Print the result to the terminal

  vector<vec3> result( 4 );
  vec3 a(1,4,9.2);
  vec3 b(4,1,9.8);
  InterpolateVec( a, b, result );
  for( int i=0; i<result.size(); ++i )
  {
    cout << "( "
    << result[i].x << ", "
    << result[i].y << ", "
    << result[i].z << " ) ";
  }

  // while( NoQuitMessageSDL() )
  //   {
  //     Draw(screen);
  //     Update();
  //     SDL_Renderframe(screen);
  //   }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec3 colour(30.5,30.5,30.5);
  for(int i=0; i<1000; i++)
    {
      uint32_t x = rand() % screen->width;
      uint32_t y = rand() % screen->height;
      PutPixelSDL(screen, x, y, colour);
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

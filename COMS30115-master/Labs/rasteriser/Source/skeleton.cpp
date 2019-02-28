#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::ivec2;
using glm::vec2;

SDL_Event event;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE true
#define FOCAL_LENGTH SCREEN_HEIGHT/2
vec4 cameraPos( 0, 0, 0, 1 );
vec4 cameraPosM( 0, 0, 3.001, 0 );

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

bool Update(vector<Triangle>& triangles);
void Draw(screen* screen, vector<Triangle> triangles);
void VertexShader( const vec4& v, ivec2& p );
void Interpolate( ivec2 a, ivec2 b, vector<ivec2>& result );
void DrawLineSDL( SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color );
void DrawPolygonEdges( const vector<vec4>& vertices , screen* screen);
void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels );


int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vector<Triangle> triangles;
  LoadTestModel( triangles );

  for (size_t i = 0; i < triangles.size(); i++) {
    triangles[i].v0 += cameraPosM;
    triangles[i].v1 += cameraPosM;
    triangles[i].v2 += cameraPosM;

  }

  while (Update(triangles))
    {

      Draw(screen, triangles);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<Triangle> triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  for( uint32_t i=0; i<triangles.size(); ++i ){
    vector<vec4> vertices(3);
    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;

    DrawPolygonEdges( vertices, screen );
  }
}

/*Place updates of parameters here*/
bool Update(vector<Triangle>& triangles)
{
  // static int t = SDL_GetTicks();
  // /* Compute frame time */
  // int t2 = SDL_GetTicks();
  // float dt = float(t2-t);
  // t = t2;

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

void VertexShader( const vec4& v, ivec2& p ){
  p.x = FOCAL_LENGTH * (v.x / v.z) + SCREEN_WIDTH/2;
  p.y = FOCAL_LENGTH * (v.y / v.z) + SCREEN_HEIGHT/2;
}

void Interpolate( ivec2 a, ivec2 b, vector<ivec2>& result ){
  int N = result.size();
  vec2 step = vec2(b-a) / float(max(N-1,1));
  vec2 current( a );
  for( int i=0; i<N; ++i ){
    result[i] = current;
    current += step;
  }
}

void DrawLineSDL( screen* screen, ivec2 a, ivec2 b, vec3 color ){
  ivec2 delta = glm::abs( a - b );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<ivec2> line(pixels);
  Interpolate(a, b, line);

  for (int j = 0; j < pixels; j++) {
    vec3 color(1,1,1);
    PutPixelSDL( screen, line[j].x, line[j].y, color );
  }
}

void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation){
  float x = rotation.x;
  float y = rotation.y;
  float z = rotation.z;
  vec4 col1(cos(y)*cos(z), cos(y)*sin(z),-sin(y),0);
  vec4 col2(-cos(x)*sin(z)+sin(x)*sin(y)*cos(z), cos(x)*cos(z)+sin(x)*sin(y)*sin(z),sin(x)*cos(y),0);
  vec4 col3(sin(x)*sin(z)+cos(x)*sin(y)*cos(z),-sin(x)*cos(z)+cos(x)*sin(y)*sin(z),cos(x)*cos(y),0);

  mat4 rotationMatrix(col1,col2,col3,translation);
  for (size_t i = 0; i < triangles.size(); i++) {


    triangles[i].v0 = rotationMatrix*triangles[i].v0;
    triangles[i].v1 = rotationMatrix*triangles[i].v1;
    triangles[i].v2 = rotationMatrix*triangles[i].v2;
  }
  // lightPos = rotationMatrix*lightPos;

}

void DrawPolygonEdges( const vector<vec4>& vertices, screen* screen){

  for (unsigned int v = 0; v < vertices.size(); v++) {
    ivec2 projPos;
    ivec2 a;
    ivec2 b;

    VertexShader( vertices[v], a);
    VertexShader( vertices[(v+1)%3], b);
    vec3 colour(1,1,1);

    DrawLineSDL(screen, a, b, colour);
  }
}

void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels ){

  int height = 0;
  int lowestY = +numeric_limits<int>::max();;

  for (size_t j = 0; j < vertexPixels.size(); j++) {
    int temp = glm::abs(vertexPixels[j].y - vertexPixels[(j+1) % 3].y);
    if(height < temp) height = temp;
    if(lowestY < vertexPixels[j].y) lowestY = vertexPixels[j].y;

  }

  for (int i = 0; i < height; i++) {
    leftPixels[i].x = +numeric_limits<int>::max();
    leftPixels[i].y = lowestY + i;
    rightPixels[i].x = -numeric_limits<int>::max();
    rightPixels[i].y = lowestY + i;
  }


  // for (unsigned int i = 0; i < polygonEdges.size(); i++) {
  //   for (unsigned j = 0; j < polygonEdges[i].size(); j++) {
  //     for(unsigned k = 0; k < leftPixels.size(); k++){
  //       int tmpX = polygonEdges[i][j].x;
  //       int tmpY = polygonEdges[i][j].y;
  //       int tmpIndex = tmpY-height;
  //       if(tmpX > leftPixels[tmpIndex].x) leftPixels[tmpIndex].x   = tmpX;
  //       if(tmpX < rightPixels[tmpIndex].x) rightPixels[tmpIndex].x = tmpX;
  //     }
  //   }
  // }
}

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

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define FULLSCREEN_MODE true
#define FOCAL_LENGTH SCREEN_HEIGHT/2
vec4 cameraPos( 0, 0, 3.001, 0 );
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

struct Pixel
  {
  ivec2 position;
  float depth;
  };

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

bool Update(vector<Triangle>& triangles);
void Draw(screen* screen, vector<Triangle> triangles);
void VertexShader( const vec4& v, Pixel& p );
void Interpolate( Pixel a, Pixel b, vector<Pixel>& result );
void DrawLineSDL( screen* surface, Pixel a, Pixel b, vec3 color );
void DrawPolygon( const vector<vec4>& vertices , screen* screen, vec3 color);
void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels );
void DrawRows( const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels, screen* screen , vec3 color);

int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vector<Triangle> triangles;
  LoadTestModel( triangles );

  for (size_t i = 0; i < triangles.size(); i++) {
    triangles[i].v0 += cameraPos;
    triangles[i].v1 += cameraPos;
    triangles[i].v2 += cameraPos;

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
void Draw(screen* screen, vector<Triangle> triangles){
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  for (int i = 0; i < SCREEN_WIDTH; i++) {
    for (int j = 0; j < SCREEN_HEIGHT; j++) {
      depthBuffer[j][i] = 0;
    }
  }


  for( uint32_t i=0; i<triangles.size(); ++i ){
    vector<vec4> vertices(3);
    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;
    vec3 color = triangles[i].color;

    DrawPolygon( vertices, screen, color );

  }

}

void DrawPolygon( const vector<vec4>& vertices, screen* screen, vec3 color){
  vector<Pixel> vertexPixels( vertices.size() );

  for (unsigned int v = 0; v < vertices.size(); v++) {
    VertexShader( vertices[v], vertexPixels[v]);
  }

  vector<Pixel> leftPixels;
  vector<Pixel> rightPixels;
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );

  DrawRows(leftPixels,rightPixels,screen,color);
}

void VertexShader( const vec4& v, Pixel& p ){
  vec4 copyV = v+cameraPos;
  p.position.x = FOCAL_LENGTH * (copyV.x / copyV.z) + SCREEN_WIDTH/2;
  p.position.y = FOCAL_LENGTH * (copyV.y / copyV.z) + SCREEN_HEIGHT/2;
  p.depth = 1/copyV.z;
}

void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels ){

  int highestY = 0;
  int lowestY = +numeric_limits<int>::max();

  for (size_t j = 0; j < vertexPixels.size(); j++) {
    if(lowestY > vertexPixels[j].position.y) lowestY = vertexPixels[j].position.y;
    if(highestY < vertexPixels[j].position.y) highestY = vertexPixels[j].position.y;
  }

  int numOfRows = highestY - lowestY + 1;

  leftPixels = vector<Pixel>(numOfRows);
  rightPixels = vector<Pixel>(numOfRows);


  for (int i = 0; i < numOfRows; i++) {
    leftPixels[i].position.x = +numeric_limits<int>::max();
    rightPixels[i].position.x = -numeric_limits<int>::max();
  }

  vector<Pixel> result(numOfRows);

  for (unsigned int i=0; i<vertexPixels.size(); i++){
    Interpolate( vertexPixels[i], vertexPixels[(i+1)%vertexPixels.size()], result);

    for(unsigned int j=0; j < result.size(); j++){
      int y = (result[j].position.y)%numOfRows;
      if(leftPixels[y].position.x > result[j].position.x) leftPixels[y] = result[j];
      if(rightPixels[y].position.x < result[j].position.x) rightPixels[y] = result[j];
    }
  }
}

void Interpolate( Pixel a, Pixel b, vector<Pixel>& result ){
  int N = result.size();

  vec2 step = vec2(b.position-a.position) / float(max(N-1,1));

  float depthStep = (b.depth-a.depth) / float(max(N-1,1));

  vec2 current( a.position );
  for( int i=0; i<N; ++i ){
    result[i].position = round(current);
    current += step;
    result[i].depth = a.depth+i*depthStep;

  }
}

void DrawRows( const vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels,screen* screen, vec3 color ){
  for(unsigned int i = 0; i < leftPixels.size(); i++){
    DrawLineSDL(screen, leftPixels[i], rightPixels[i], color);
  }
}

void DrawLineSDL( screen* screen, Pixel a, Pixel b, vec3 color ){
  ivec2 delta = glm::abs( a.position - b.position );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<Pixel> line(pixels);
  Interpolate(a, b, line);

  for (int i = 0; i < pixels; i++) {
    if(depthBuffer[line[i].position.y][line[i].position.x] < line[i].depth){

      depthBuffer[line[i].position.y][line[i].position.x] = line[i].depth;
      PutPixelSDL( screen, line[i].position.x, line[i].position.y, color );
    }
  }
}

/*Place updates of parameters here*/
bool Update(vector<Triangle>& triangles){
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
}

#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <complex>

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
vec4 cameraPos( 0, 0, 3.001, 1 );
vec4 cameraPosAdd( 0, 0, 3.0001, 0 );
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
vec4 lightPos(0,-0.5,-0.7, 1);
vec3 lightPower = 14.f*vec3( 1, 1, 1 );
vec3 indirectLightPowerPerArea = 0.5f*vec3( 1, 1, 1 );

mat4 R(
   vec4(1,0,0,0) ,
   vec4(0,1,0,0) ,
   vec4(0,0,1,0) ,
   cameraPos
);

struct Pixel
{
  ivec2 position;
  float depth;
  vec3 illumination;
  vec4 pos3d;
};

struct Vertex
{
  vec4 position;
  vec4 normal;
  vec3 colour;
};

struct Fract
{
   complex<float> z;
   complex<float> c;
};

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

bool Update(vector<Triangle>& triangles);
void Draw(screen* screen, vector<Triangle> triangles);
void VertexShader( Vertex& v, Pixel& p );
void Interpolate( Pixel a, Pixel b, vector<Pixel>& result );
void DrawLineSDL( screen* surface, Pixel a, Pixel b, vec3 color, vec4 normal );
void DrawPolygon(  vector<Vertex>& vertices , screen* screen, vec3 color);
void rotateCamera(vec4 rotation, vector<Triangle>& triangles, vec4 translation);
void ComputePolygonRows( vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels );
void DrawRows(  vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels, screen* screen , vec3 color, vec4 normal);
Fract Fractal(Pixel& p);

int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vector<Triangle> triangles;
  LoadTestModel( triangles );

  // for (size_t i = 0; i < triangles.size(); i++) {
  //   triangles[i].v0 += cameraPos;
  //   triangles[i].v1 += cameraPos;
  //   triangles[i].v2 += cameraPos;
  //
  // }

  while (Update(triangles))
    {
      Draw(screen, triangles);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

Fract Fractal(Pixel& p){
   std::complex<float> z = (float) p.position.x / SCREEN_WIDTH;
   std::complex<float> c = (float) p.position.y / 2 * SCREEN_HEIGHT;
   // std::complex<float> c = (float) 0.5f;

   int iteration = 0;
   while(abs(z) < 200 && ++iteration < SCREEN_HEIGHT) {
      z = pow(z, 2) + c;
   }
   Fract f = { z = z, c = c };

   return f;
}

/*Place your drawing here*/
void Draw(screen* screen, vector<Triangle> triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  // vec3 out = vec3(0.f);
  // float mani = 0.f;
  // unsigned long max_iteration = SCREEN_HEIGHT;
  // std::complex<float> prevZ = 0.f;
  // for (unsigned int i = 0; i < SCREEN_WIDTH; i++) {
  //    for (unsigned int j = 0; j < SCREEN_HEIGHT; j++) {
  //       std::complex<float> z = (float)i * 2 / SCREEN_WIDTH;
  //       std::complex<float> c = (float)j * 2 / SCREEN_HEIGHT;
  //
  //       // printf("%.10f %.10f\n", abs(imag(z)), abs(c));
  //       // printf("%.10f %.10f\n", imag(z), imag(c));
  //
  //       int iteration = 0;
  //       while(abs(z) < 150 && ++iteration < max_iteration) {
  //          z = pow(z, 2) + c;
  //       }
  //
  //       // printf("%.10f %.10f %d\n", abs(z), abs(c), i*j);
  //
  //       // if(iteration == max_iteration) PutPixelSDL(screen, i, j, vec3(0,0,0));
  //       PutPixelSDL(screen, i, j, vec3( tan(abs(z)), tan(abs(c)), tan(i*j)));
  //
  //       // PutPixelSDL(screen, i, j, out);
  //       // mani = ((sin(mani) + exp(mani))) / pow(2, 500);
  //       // out = vec3(mani);
  //    }
  // }

  for (int i = 0; i < SCREEN_WIDTH; i++) {
    for (int j = 0; j < SCREEN_HEIGHT; j++) {
      depthBuffer[j][i] = 0;
    }
  }

  for( uint32_t i=0; i<triangles.size(); ++i ){
    vector<Vertex> vertices(3);
    vertices[0].position = triangles[i].v0;
    vertices[1].position = triangles[i].v1;
    vertices[2].position = triangles[i].v2;

    vertices[0].normal = triangles[i].normal;
    vertices[1].normal = triangles[i].normal;
    vertices[2].normal = triangles[i].normal;

    vec3 color = triangles[i].color;

    DrawPolygon( vertices, screen, color );
  }
}

void DrawPolygon( vector<Vertex>& vertices, screen* screen, vec3 color){
  vector<Pixel> vertexPixels( vertices.size() );

  for (unsigned int v = 0; v < vertices.size(); v++) {
    VertexShader( vertices[v], vertexPixels[v]);
  }

  vector<Pixel> leftPixels;
  vector<Pixel> rightPixels;
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
  DrawRows(leftPixels,rightPixels,screen,color, vertices[0].normal);
}

void VertexShader( Vertex& v, Pixel& p ){
   Vertex copyV { R * v.position };
   cameraPos = R * cameraPos;
   p.position.x = FOCAL_LENGTH * (copyV.position.x / copyV.position.z) + SCREEN_WIDTH/2;
   p.position.y = FOCAL_LENGTH * (copyV.position.y / copyV.position.z) + SCREEN_HEIGHT/2;
   p.depth = 1/copyV.position.z;
   p.pos3d = vec4(copyV.position.x*p.depth,copyV.position.y*p.depth,copyV.position.z*p.depth,1);
   // float r = glm::distance(lightPos, v.position);   //Distance from light source to vertex
   // vec4 reflection = (lightPos - v.position) / r;  //Unit vector of reflection
   // p.illumination = ( ((lightPower) * max(dot(reflection, v.normal), 0.f)) / (float)(4*3.1415*pow(r,2)));
}

void ComputePolygonRows( vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels ){

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

  vec4 pos3dStep ((b.pos3d - a.pos3d)/ float(max(N-1,1)));

  vec2 current( a.position );
  vec4 currentPos3d( a.pos3d );


  for( int i=0; i<N; ++i ){
    result[i].position = round(current);
    current += step;
    result[i].depth = a.depth+i*depthStep;
    result[i].pos3d = currentPos3d;
    currentPos3d += pos3dStep;

  }
}

void DrawRows(  vector<Pixel>& leftPixels,const vector<Pixel>& rightPixels,screen* screen, vec3 color, vec4 normal ){
  for(unsigned int i = 0; i < leftPixels.size(); i++){
    DrawLineSDL(screen, leftPixels[i], rightPixels[i], color,normal);
  }
}

void DrawLineSDL( screen* screen, Pixel a, Pixel b, vec3 color , vec4 normal){
  ivec2 delta = glm::abs( a.position - b.position );
  int pixels = glm::max( delta.x, delta.y ) + 1;
  vector<Pixel> line(pixels);
  Interpolate(a, b, line);

  for (int i = 0; i < pixels; i++) {

    if(depthBuffer[line[i].position.y][line[i].position.x] < line[i].depth){


      depthBuffer[line[i].position.y][line[i].position.x] = line[i].depth;
      vec4 position(line[i].pos3d.x,line[i].pos3d.y,line[i].pos3d.z,1);
      position = position / line[i].depth;
      position.w = 1.f;
      float r = glm::distance(lightPos+cameraPosAdd, position);   //Distance from light source to vertex
      vec4 reflection = (lightPos+cameraPosAdd - position) / r;  //Unit vector of reflection
      vec3 illumination( ((lightPower) * max(dot(reflection, normal), 0.f)) / (float)(4*3.1415*pow(r,2)));

      Fract filter = Fractal(line[i]);
      // printf("%.2f %.2f %.2f\n", sin(filter), tan(filter), cos(exp(filter)));

      PutPixelSDL( screen, line[i].position.x, line[i].position.y, vec3(abs(tan((filter.z)))) * color *(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, vec3(abs(tan((filter.z))), abs((tan(filter.c))), abs((tan(i*i)))) * color *(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, vec3(abs(tan(exp(filter))), abs(sin(exp(filter))), abs(cos(exp(filter)))) * color *(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, vec3(sin(1.f/i) + exp(1.f/i) + 0) * color *(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, (vec3(tan(1000.f/i), tan(1000.f/i), tan(1000.f/i)) * color )*(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, vec3(tan(exp(i)), cos(exp(i)), sin(exp(i))) * color*(indirectLightPowerPerArea +illumination) );
      // PutPixelSDL( screen, line[i].position.x, line[i].position.y, color*(indirectLightPowerPerArea +illumination) );
    }
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
         // rotateCamera(vec4 (0.0785398f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));
         rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (0.f,-0.1f,0.f,1.f));		/* Move camera backwards */

           /* Move camera forward */
    break;
      case SDLK_DOWN:
      //lightPos.z++;
      // rotateCamera(vec4 (-0.0785398f,0.f,0.f,1.f), triangles, vec4 (0.f,0.f,0.f,1.f));
      rotateCamera(vec4 (0.f,0.f,0.f,1.f), triangles, vec4 (0.f,0.1f,0.f,1.f));		/* Move camera backwards */


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
  lightPos = rotationMatrix*lightPos;
}

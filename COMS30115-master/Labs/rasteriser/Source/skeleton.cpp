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
#define FULLSCREEN_MODE false
#define FOCAL_LENGTH SCREEN_HEIGHT/2
#define Y_MAX SCREEN_HEIGHT
#define Y_MIN 0
#define X_MAX SCREEN_WIDTH
#define X_MIN 0

vec4 cameraPos( 0, 0, 3.001, 1 );
vec4 cameraPosAdd( 0, 0, 3.0001, 0 );
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
vec4 lightPos(0,-0.5,-0.7, 1);
vec3 lightPower = 14.f*vec3( 1, 1, 1 );
vec3 indirectLightPowerPerArea = 0.5f*vec3( 1, 1, 1 );
bool isClipping = true;

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
  unsigned char outcode;
};

struct Vertex
{
  vec4 position;
  vec4 normal;
  vec3 colour;
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
void Outcode( Pixel& a );
int clipTOP( Pixel& a, Pixel& b);
int clipBOTTOM( Pixel& a, Pixel& b);
int clipLEFT( Pixel& a, Pixel& b);
int clipRIGHT( Pixel& a, Pixel& b);
void clipZ(vector<Pixel>& trianglePixels);
void clipPolygon(vector<Pixel>& trianglePixels);
void clipEdge(vector<Pixel>& trianglePixels, vec2 edgeVertices[]);
bool isOnScreen(Pixel& pixel, vec2 * edgeVertices);
void Intermediate(Pixel& p1, Pixel& p2, Pixel& intermediate, vec2 edgeVertices);


int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vector<Triangle> triangles;
  LoadTestModel( triangles );

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

  if(isClipping) clipZ(vertexPixels);

 //  if(vertexPixels.size() == 4){
 //     printf("%.2f %.2f %.2f %.2f\n", vertexPixels[0].depth, vertexPixels[1].depth, vertexPixels[2].depth, vertexPixels[3].depth);
 // }

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
   p.pos3d = vec4(copyV.position.x*p.depth, copyV.position.y*p.depth, copyV.position.z*p.depth,1);
   // p.pos3d = v.position;
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
      vec4 position(line[i].pos3d.x, line[i].pos3d.y, line[i].pos3d.z,1);
      position = position / line[i].depth;
      position.w = 1.f;
      float r = glm::distance(lightPos+cameraPosAdd, position);   //Distance from light source to vertex
      // printf("%.2f//%.2f//%.2f//%.2f\n", position.x,position.y,position.z,position.w);

      vec4 reflection = (lightPos+cameraPosAdd - position) / r;  //Unit vector of reflection

      vec3 illumination( ((lightPower) * max(dot(reflection, normal), 0.f)) / (float)(4*3.1415*pow(r,2)));
      //if(illumination.x<0.00)printf("%.2f//%.2f//%.2f\n\n", illumination.x,illumination.y,illumination.z);
      //if(((lightPower * dot(reflection, normal))/(float)(4*3.1415*pow(r,2))).x>0.f)  printf("%.2f\n",((lightPower * dot(reflection, normal))/(float)(4*3.1415*pow(r,2))).x);
      PutPixelSDL( screen, line[i].position.x, line[i].position.y, color*(indirectLightPowerPerArea +illumination) );

   }
  }
}

int clipLine( Pixel& a, Pixel& b){
   Outcode(a);
   Outcode(b);

   int outA = a.outcode;
   int outB = b.outcode;

   if( outA | outB ) return 0;
   if( (outA & outB) != 0 ) return -1;

   return 1;

}

void Outcode( Pixel& a ){
   int outcode = 0;
   int x = a.position.x;
   int y = a.position.y;

   if(x > X_MAX)      outcode += 2;
   else if(x < X_MIN) outcode += 1;

   if(y > Y_MAX)      outcode += 8;
   else if(y < Y_MIN) outcode += 4;

   a.outcode = outcode;
}

int clipTOP( Pixel& a, Pixel& b){
   int x0 = a.position.x;
   int y0 = a.position.y;
   int x1 = b.position.x;
   int y1 = b.position.y;

   return x1 + (x0 - x1) * (Y_MAX - y1) / (y0 - y1);
}
int clipBOTTOM( Pixel& a, Pixel& b){
   int x0 = a.position.x;
   int y0 = a.position.y;
   int x1 = b.position.x;
   int y1 = b.position.y;

   return x1 + (x0 - x1) * (Y_MIN - y1) / (y0 - y1);
}
int clipLEFT( Pixel& a, Pixel& b){
   int x0 = a.position.x;
   int y0 = a.position.y;
   int x1 = b.position.x;
   int y1 = b.position.y;

   return y1 + (X_MIN - x1) * (y0 - y1) / (x0 - x1);
}
int clipRIGHT( Pixel& a, Pixel& b){
   int x0 = a.position.x;
   int y0 = a.position.y;
   int x1 = b.position.x;
   int y1 = b.position.y;

   return y1 + (X_MAX - x1) * (y0 - y1) / (x0 - x1);
}

void clipPolygon(vector<Pixel>& trianglePixels){
   vec2 edgeVertices[2];

    clipZ(trianglePixels);

    /* Top edge */  // x, y
    edgeVertices[0] = vec2(SCREEN_WIDTH, 0);
    edgeVertices[1] = vec2(0, 0);
    clipEdge(trianglePixels, edgeVertices);

    /* Left edge */
    edgeVertices[0] = vec2(0, 0);
    edgeVertices[1] = vec2(0, SCREEN_HEIGHT);
    clipEdge(trianglePixels, edgeVertices);

    /* Right edge */
    edgeVertices[0] = vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
    edgeVertices[1] = vec2(SCREEN_WIDTH, 0);
    clipEdge(trianglePixels, edgeVertices);

    /* Bottom edge */
    edgeVertices[0] = vec2(0, SCREEN_HEIGHT);
    edgeVertices[1] = vec2(SCREEN_WIDTH, SCREEN_HEIGHT);
    clipEdge(trianglePixels, edgeVertices);
}

void clipZ(vector<Pixel>& trianglePixels){

   int size = trianglePixels.size();

   vector<Pixel> clippedPixels;
   Pixel p1;
   Pixel p2 = trianglePixels[size - 1];
   float threshold = 1.00000f;

   for (int i = 0; i < size; i++) {
      p1 = trianglePixels[i];

      if( (p1.depth <= threshold) && (p2.depth <= threshold) ){
         clippedPixels.push_back(p2);
      }
      else if( (p1.depth <= threshold) && !(p2.depth <= threshold)){
         Pixel intermediate;
         intermediate.position.x = FOCAL_LENGTH * (p1.pos3d.x / p1.depth) + SCREEN_WIDTH/2;
         intermediate.position.y = FOCAL_LENGTH * (p1.pos3d.y / p1.depth) + SCREEN_HEIGHT/2;
         intermediate.depth = 1;
         intermediate.pos3d = vec4((p1.pos3d.x / p1.depth), (p1.pos3d.y / p1.depth)*1, 1,1);

         clippedPixels.push_back(intermediate);
      }
      else if( !(p1.depth <= threshold) && (p2.depth <= threshold)){
         Pixel intermediate;
         intermediate.position.x = FOCAL_LENGTH * (p1.pos3d.x / p1.depth) + SCREEN_WIDTH/2;
         intermediate.position.y = FOCAL_LENGTH * (p1.pos3d.y / p1.depth) + SCREEN_HEIGHT/2;
         intermediate.depth = 1;
         intermediate.pos3d = vec4((p1.pos3d.x / p1.depth), (p1.pos3d.y / p1.depth)*1, 1,1);

         clippedPixels.push_back(intermediate);
         clippedPixels.push_back(p2);
      }
      p2 = p1;
   }
   trianglePixels = clippedPixels;
}

void clipEdge(vector<Pixel>& trianglePixels, vec2 edgeVertices[]){
   int size = trianglePixels.size();

   vector<Pixel> clippedPixels;
   Pixel p1;
   Pixel p2 = trianglePixels[size - 1];

   for (int i = 0; i < size; i++) {
      p1 = trianglePixels[i];

      if( (isOnScreen(p1, edgeVertices)) && (isOnScreen(p2, edgeVertices))) {
         clippedPixels.push_back(p1);
      }
      else if( (isOnScreen(p1, edgeVertices)) && !(isOnScreen(p2, edgeVertices))) {
         Pixel intermediate;
         //intermediate function

         clippedPixels.push_back(intermediate);
      }
      else if( !(isOnScreen(p1, edgeVertices)) && (isOnScreen(p2, edgeVertices))) {
         Pixel intermediate;

         //intermediate function
         intermediate.position.x = FOCAL_LENGTH * (p1.pos3d.x / p1.depth) + SCREEN_WIDTH/2;
         intermediate.position.y = FOCAL_LENGTH * (p1.pos3d.y / p1.depth) + SCREEN_HEIGHT/2;
         intermediate.depth = 1;
         intermediate.pos3d = vec4((p1.pos3d.x / p1.depth), (p1.pos3d.y / p1.depth)*1, 1,1);



         clippedPixels.push_back(intermediate);
         clippedPixels.push_back(p1);
      }
   }
}

bool isOnScreen(Pixel& pixel, vec2 * edgeBounds){
   /* Top edge */
    if ((edgeBounds[0].x > edgeBounds[1].x) && (pixel.position.y >= edgeBounds[0].y)) {
        return true;
    }
        /* Left edge */
    else if ((edgeBounds[1].y > edgeBounds[0].y) && (pixel.position.x >= edgeBounds[0].x)) {
        return true;
    }
        /* Right edge */
    else if ((edgeBounds[0].y > edgeBounds[1].y) && (edgeBounds[0].x >= pixel.position.x)) {
        return true;
    }
        /* Bottom edge */
    else if ((edgeBounds[1].x > edgeBounds[0].x) && (edgeBounds[0].y >= pixel.position.y)) {
        return true;
    }

    return false;
}

void Intermediate(Pixel& p1, Pixel& p2, Pixel& Intermediate, vec2 edgeVertices){

   //Top
   if(edgeVertices[0].y > edgeVertices[1].y){
      float TotalDist = glm::abs(p1.position.y - p2.position.y);
      float FractDist = p1.position.y;
      float scale = FractDist / TotalDist;

      vec3

   }

   //Left

   //Bottom

   //Right
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

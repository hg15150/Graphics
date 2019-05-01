#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>

#define PI 3.14159265359

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;


enum MaterialType { GlassType , MirrorType, RoughType };

struct Material
   {
		MaterialType type;
		float amb;
      float diff;
      float spec;
      float shi;
      float emit;
   };

Material Glass = { .type = GlassType,.amb = 0.5, .diff = 0.5, .spec = 50 , .shi = 15 };
Material Rough = {  .type = RoughType,.amb = 1, .diff = 0.8, .spec = 0.0, .shi = 2};
Material Mirror = { .type = MirrorType,.amb = 0.9, .diff = 0.9, .spec = 10, .shi = 10};

struct Light
   {
		glm::vec3 brightness;
      glm::vec3 amb;
      glm::vec3 diff;
      glm::vec3 spec;
      glm::vec4 pos;
   };

class Item {
public:
  glm::vec3 colour;
  Material material;

  Item(glm::vec3 colour, Material material): colour(colour), material(material){} // constructor
  // virtual ~Item() {} // virtual destructor
  virtual bool intersection(vec4 start, vec4 dir, float &t, vec4& position){ return false; }
  virtual vec4 computeNormal(vec4 position) { return vec4(-1); }
  virtual void scale(float L){}
  virtual void rotate(mat4 rotationMatrix){}
};



// class Sphere {
// public:
// 	glm::vec4 c;
// 	float r;
// 	std::vector<glm::vec4> points;
// 	std::vector<glm::vec3> indices;
//
// 	uint sectorCount = 12;
// 	uint stackCount = 12;
//
// 	float sectorStep = 2*PI / sectorCount;
// 	float stackStep = PI / stackCount;
// 	float stackAngle, sectorAngle;
// 	float x, y, z, xz;
//
// 	Sphere( glm::vec4 c, float r ) : c(c), r(r)
// 	{
// 		ComputePoints();
// 		ComputeIndices();
// 	}
//
// 	void ComputePoints()
// 	{
// 		points.push_back(c + glm::vec4(0, -r, 0, 0));
// 		for (uint i = 1; i < stackCount; i++) {
// 			stackAngle = PI - i*stackStep;
// 			xz = r * sinf(stackAngle);
// 			y = r * cosf(stackAngle);
// 			// printf("%.2f %.2f\n",xz, y );
//
//
// 			for (uint j = 0; j < sectorCount; j++) {
// 				sectorAngle = 2*PI - j * sectorStep;
//
// 				//vertex
// 				x = xz * cosf(sectorAngle);
// 				z = xz * sinf(sectorAngle);
//
// 				points.push_back(c + glm::vec4(x, y, z, 0));
// 			}
// 		}
// 		points.push_back(c + glm::vec4(0, r, 0, 0));
// 	}
//
// 	void ComputeIndices(){
// 		for (uint j = 0; j < sectorCount-1; j++) {
// 			indices.push_back(glm::vec3(0,j+1,j+2));
// 		}
// 		indices.push_back(glm::vec3(0,sectorCount,1));
//
// 		for (uint i = 0; i < stackCount-2; i++) {
// 			int sectorFirst = i * sectorCount + 1;
// 			int sectorSecond = (i+1) * sectorCount + 1;
//
// 			for (uint j = 0; j < sectorCount; j++) {
// 				indices.push_back(glm::vec3(sectorFirst+j,sectorSecond+j,sectorFirst+((j+1)%sectorCount)));
// 				indices.push_back(glm::vec3(sectorFirst+((j+1)%sectorCount),sectorSecond+j,sectorSecond+((j+1)%sectorCount)));
// 			}
// 		}
//
// 		int finalValue = (stackCount-1)*sectorCount+1;
// 		for (uint j = 1; j < sectorCount; j++) {
// 			indices.push_back(glm::vec3( finalValue , finalValue - (j), finalValue - (j+1) ));
// 		}
// 		indices.push_back(glm::vec3( finalValue, finalValue - (sectorCount), finalValue - (1) ));
// }
//
// };


class Sphere2 : public Item{
public:
   float radius;
   glm::vec4 center;

   Sphere2( glm::vec4 c, float r, vec3 colours, Material material ) : Item(colour, material), center(c), radius(r) {}

   bool intersection(glm::vec4 start, glm::vec4 dir, float& t, glm::vec4& position) override {
      return(start.x > dir.x);
   };

   glm::vec4 computeNormal( glm::vec4 position) override{
      glm::vec4 normal = glm::normalize(position - center);
      normal.w = 1.f;
      return normal;
   }

   void scale(float L) override {

   }

   void rotate(mat4 rotationMatrix) override {
      center = rotationMatrix * center;
   }
};


// Used to describe a triangular surface:
class Triangle : public Item {
public:
	vec4 v0;
	vec4 v1;
	vec4 v2;
	vec4 normal;
   // vec3 colour;
   // Material material;

	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 colour, Material material )
		: Item(colour, material), v0(v0), v1(v1), v2(v2)
      {

	   }

	vec4 computeNormal(vec4 position) override
	{
	  vec3 e1 = glm::vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
	  vec3 e2 = glm::vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
	  vec3 normal3 = glm::normalize( glm::cross( e2, e1 ) );
	  normal.x = normal3.x;
	  normal.y = normal3.y;
	  normal.z = normal3.z;
	  normal.w = 1.0;
     return normal;
	}

   bool intersection( vec4 s, vec4 dir, float &x, vec4& position) override {

       bool intersectionOccurred = false;

       //Determine axis of the plane that the triangle lies within
       vec3 e1 = vec3(v1.x-v0.x, v1.y-v0.y, v1.z-v0.z);
       vec3 e2 = vec3(v2.x-v0.x, v2.y-v0.y, v2.z-v0.z);
       //Calculate start of ray - vertex 0
       vec3 b = vec3(s.x-v0.x, s.y-v0.y, s.z-v0.z);

       //Create matrix
       mat3 A( -vec3(dir), e1, e2 );
       mat3 Ax( b , e1, e2 );

       float determinantA = glm::determinant(A);
       float determinantAx = glm::determinant(Ax);

       x = determinantAx/determinantA;

       if(x >= 0){
         mat3 Ay( -vec3(dir) , b, e2 );
         float determinantAy = glm::determinant(Ay);
         float y = determinantAy/determinantA;

            if(y >= 0){
               mat3 Az( -vec3(dir) , e1, b );
               float determinantAz = glm::determinant(Az);
               float z = determinantAz/determinantA;

               if ((z >= 0) && ( (y + z) < 1)){
                  intersectionOccurred = true; //At least one intersection occurred
                  position = v0 + (y * vec4(e1.x, e1.y, e1.z, 0) + (z * vec4(e2.x, e2.y, e2.z, 0)));
               }
            }
         }
        return intersectionOccurred;
   }

   void scale(float L) override {
     v0 *= 2/L;
     v1 *= 2/L;
     v2 *= 2/L;

     v0 -= glm::vec4(1,1,1,1);
     v1 -= glm::vec4(1,1,1,1);
     v2 -= glm::vec4(1,1,1,1);

     v0.x *= -1;
     v1.x *= -1;
     v2.x *= -1;

     v0.y *= -1;
     v1.y *= -1;
     v2.y *= -1;

     v0.w = 1.0;
     v1.w = 1.0;
     v2.w = 1.0;
   }

   void rotate(mat4 rotationMatrix) override {
      v0 = rotationMatrix*v0;
      v1 = rotationMatrix*v1;
      v2 = rotationMatrix*v2;
      computeNormal(vec4(0));
   }
};

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Item*>& triangles )
{
	using glm::vec3;
	using glm::vec4;

	// Defines colours:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );

	triangles.clear();
	triangles.reserve( 5*2*3 );

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec4 A(L,0,0,1);
	vec4 B(0,0,0,1);
	vec4 C(L,0,L,1);
	vec4 D(0,0,L,1);

	vec4 E(L,L,0,1);
	vec4 F(0,L,0,1);
	vec4 G(L,L,L,1);
	vec4 H(0,L,L,1);

	// Floor:
	triangles.push_back( new Triangle( C, B, A, green, Rough ) );
	triangles.push_back( new Triangle( C, D, B, green, Rough ) );

	// Left wall
	// triangles.push_back( new Triangle( A, E, C, purple, Mirror ) );
	// triangles.push_back( new Triangle( C, E, G, purple, Mirror ) );
	triangles.push_back( new Triangle( A, E, C, purple, Rough ) );
	triangles.push_back( new Triangle( C, E, G, purple, Rough ) );

	// Right wall
	triangles.push_back( new Triangle( F, B, D, yellow, Rough ) );
	triangles.push_back( new Triangle( H, F, D, yellow, Rough ) );

	// Ceiling
	triangles.push_back( new Triangle( E, F, G, cyan, Rough ) );
	triangles.push_back( new Triangle( F, H, G, cyan, Rough ) );

	// Back wall
	triangles.push_back( new Triangle( G, D, C, white, Rough ) );
	triangles.push_back( new Triangle( G, H, D, white, Rough ) );

	// ---------------------------------------------------------------------------
	// Short block

	A = vec4(290,0,114,1);
	B = vec4(130,0, 65,1);
	C = vec4(240,0,272,1);
	D = vec4( 82,0,225,1);

	E = vec4(290,165,114,1);
	F = vec4(130,165, 65,1);
	G = vec4(240,165,272,1);
	H = vec4( 82,165,225,1);

	// Front
	triangles.push_back( new Triangle(E,B,A,red, Rough) );
	triangles.push_back( new Triangle(E,F,B,red, Rough) );

	// Front
	triangles.push_back( new Triangle(F,D,B,red, Rough) );
	triangles.push_back( new Triangle(F,H,D,red, Rough) );

	// BACK
	triangles.push_back( new Triangle(H,C,D,red, Rough) );
	triangles.push_back( new Triangle(H,G,C,red, Rough) );

	// LEFT
	triangles.push_back( new Triangle(G,E,C,red, Rough) );
	triangles.push_back( new Triangle(E,A,C,red, Rough) );

	// TOP
	triangles.push_back( new Triangle(G,F,E,red, Rough) );
	triangles.push_back( new Triangle(G,H,F,red, Rough) );

   // // Front
	// triangles.push_back( new Triangle(E,B,A,red, Glass) );
	// triangles.push_back( new Triangle(E,F,B,red, Glass) );
   //
	// // Front
	// triangles.push_back( new Triangle(F,D,B,red, Glass) );
	// triangles.push_back( new Triangle(F,H,D,red, Glass) );
   //
	// // BACK
	// triangles.push_back( new Triangle(H,C,D,red, Glass) );
	// triangles.push_back( new Triangle(H,G,C,red, Glass) );
   //
	// // LEFT
	// triangles.push_back( new Triangle(G,E,C,red, Glass) );
	// triangles.push_back( new Triangle(E,A,C,red, Glass) );
   //
	// // TOP
	// triangles.push_back( new Triangle(G,F,E,red, Glass) );
	// triangles.push_back( new Triangle(G,H,F,red, Glass) );

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec4(423,0,247,1);
	B = vec4(265,0,296,1);
	C = vec4(472,0,406,1);
   F = vec4(265,330,296,1);
	D = vec4(314,0,456,1);

	E = vec4(423,330,247,1);
	G = vec4(472,330,406,1);
	H = vec4(314,330,456,1);

	// Front
	triangles.push_back( new Triangle(E,B,A,blue, Rough) );
	triangles.push_back( new Triangle(E,F,B,blue, Rough) );

	// Front
	triangles.push_back( new Triangle(F,D,B,blue, Rough) );
	triangles.push_back( new Triangle(F,H,D,blue, Rough) );

	// BACK
	triangles.push_back( new Triangle(H,C,D,blue, Rough) );
	triangles.push_back( new Triangle(H,G,C,blue, Rough) );

	// LEFT
	triangles.push_back( new Triangle(G,E,C,blue, Rough) );
	triangles.push_back( new Triangle(E,A,C,blue, Rough) );

	// TOP
	triangles.push_back( new Triangle(G,F,E,blue, Rough) );
	triangles.push_back( new Triangle(G,H,F,blue, Rough) );



	// ----------------------------------------------
	// Scale to the volume [-1,1]^3
	for( size_t i=0; i<triangles.size(); ++i )
	{
      triangles[i]->scale(L);
	}
}

#endif

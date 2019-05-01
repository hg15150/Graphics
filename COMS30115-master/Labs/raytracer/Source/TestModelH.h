#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>

#define PI 3.14159265359


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
     glm::vec3 color;
     Material material;

     Item(glm::vec3 color, Material material): color(color), material(material){} // constructor
     // virtual ~Object() {} // virtual destructor
     virtual bool isIntersection(glm::vec4 orig, glm::vec4 dir, float &t, glm::vec4 &position) const = 0;
     virtual glm::vec4 computeNormal(glm::vec4 position) const = 0;
};

class Sphere {
public:
	glm::vec4 c;
	float r;
	std::vector<glm::vec4> points;
	std::vector<glm::vec3> indices;

	uint sectorCount = 12;
	uint stackCount = 12;

	float sectorStep = 2*PI / sectorCount;
	float stackStep = PI / stackCount;
	float stackAngle, sectorAngle;
	float x, y, z, xz;

	Sphere( glm::vec4 c, float r ) : c(c), r(r)
	{
		ComputePoints();
		ComputeIndices();
	}

	void ComputePoints()
	{
		points.push_back(c + glm::vec4(0, -r, 0, 0));
		for (uint i = 1; i < stackCount; i++) {
			stackAngle = PI - i*stackStep;
			xz = r * sinf(stackAngle);
			y = r * cosf(stackAngle);
			// printf("%.2f %.2f\n",xz, y );


			for (uint j = 0; j < sectorCount; j++) {
				sectorAngle = 2*PI - j * sectorStep;

				//vertex
				x = xz * cosf(sectorAngle);
				z = xz * sinf(sectorAngle);

				points.push_back(c + glm::vec4(x, y, z, 0));
			}
		}
		points.push_back(c + glm::vec4(0, r, 0, 0));
	}

	void ComputeIndices(){
		for (uint j = 0; j < sectorCount-1; j++) {
			indices.push_back(glm::vec3(0,j+1,j+2));
		}
		indices.push_back(glm::vec3(0,sectorCount,1));

		for (uint i = 0; i < stackCount-2; i++) {
			int sectorFirst = i * sectorCount + 1;
			int sectorSecond = (i+1) * sectorCount + 1;

			for (uint j = 0; j < sectorCount; j++) {
				indices.push_back(glm::vec3(sectorFirst+j,sectorSecond+j,sectorFirst+((j+1)%sectorCount)));
				indices.push_back(glm::vec3(sectorFirst+((j+1)%sectorCount),sectorSecond+j,sectorSecond+((j+1)%sectorCount)));
			}
		}

		int finalValue = (stackCount-1)*sectorCount+1;
		for (uint j = 1; j < sectorCount; j++) {
			indices.push_back(glm::vec3( finalValue , finalValue - (j), finalValue - (j+1) ));
		}
		indices.push_back(glm::vec3( finalValue, finalValue - (sectorCount), finalValue - (1) ));
}

};


// class Sphere2
// {
//    float radius;
//    glm::vec4 center;
//
//    Sphere( glm::vec4 c, float r ) : center(c), radius(r) {}
//
//    bool intersection(glm::vec4 origin, glm::vec4 dir, float& t, glm::vec4& posi)
//
//    glm::vec4 computeNormal( glm::vec4 position) {
//       glm::vec4 normal = glm::normalize(position - center);
//       normal.w = 1.f;
//       return normal;
//    }
//
//
// }


// Used to describe a triangular surface:
class Triangle
{
public:
	glm::vec4 v0;
	glm::vec4 v1;
	glm::vec4 v2;
	glm::vec4 normal;
	glm::vec3 color;
	Material material;

	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 color, Material material )
		: v0(v0), v1(v1), v2(v2), color(color), material(material)
      {
		   ComputeNormal();
	   }

	void ComputeNormal()
	{
	  glm::vec3 e1 = glm::vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
	  glm::vec3 e2 = glm::vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
	  glm::vec3 normal3 = glm::normalize( glm::cross( e2, e1 ) );
	  normal.x = normal3.x;
	  normal.y = normal3.y;
	  normal.z = normal3.z;
	  normal.w = 1.0;
	}
};

// // Used to describe a triangular surface:
// class Triangle2 : public Item {
// public:
// 	glm::vec4 v0;
// 	glm::vec4 v1;
// 	glm::vec4 v2;
// 	glm::vec4 normal;
// 	glm::vec3 color;
// 	Material material;
//
// 	Triangle( glm::vec4 v0, glm::vec4 v1, glm::vec4 v2, glm::vec3 color, Material material )
// 		: v0(v0), v1(v1), v2(v2), color(color), material(material)
//       {
// 		   ComputeNormal();
// 	   }
//
// 	void ComputeNormal()
// 	{
// 	  glm::vec3 e1 = glm::vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
// 	  glm::vec3 e2 = glm::vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
// 	  glm::vec3 normal3 = glm::normalize( glm::cross( e2, e1 ) );
// 	  normal.x = normal3.x;
// 	  normal.y = normal3.y;
// 	  normal.z = normal3.z;
// 	  normal.w = 1.0;
// 	}
//
//    bool isIntersection(glm::vec4 s, glm::vec4 dir, float& x, glm::vec4& position){
//       bool intersectionOccurred = false;
//       vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);
//       vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);
//       vec3 b = vec3(s.x-v0.x,s.y-v0.y,s.z-v0.z);
//       mat3 A( -vec3(dir), e1, e2 );
//
//       mat3 Ax( b , e1, e2 );
//       float determinantA = glm::determinant(A);
//       float determinantAx = glm::determinant(Ax);
//       x = determinantAx / determinantA;
//
//       if(x < 0) continue;
//
//       mat3 Ay( -vec3(dir) , b, e2 );
//       float determinantAy = glm::determinant(Ay);
//       float y = determinantAy / determinantA;
//
//       if(y >= 0){
//           mat3 Az( -vec3(dir) , e1, b );
//           float determinantAz = glm::determinant(Az);
//           float z = determinantAz / determinantA;
//           if ((z >= 0) && ( (y + z) < 1)){
//             intersectionOccurred = true; //At least one intersection occurred
//             float distance = x;
//
//         //Overwrite closestIntersection
//           if(distance < closestIntersection.distance){
//             position =  s + distance*dir;
//             closestIntersection.triangleIndex = i;
//           }
//         }
//       }
//     }
//    }
// };

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Triangle>& triangles )
{
	using glm::vec3;
	using glm::vec4;

	// Defines colors:
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
	triangles.push_back( Triangle( C, B, A, green, Rough ) );
	triangles.push_back( Triangle( C, D, B, green, Rough ) );

	// Left wall
	triangles.push_back( Triangle( A, E, C, purple, Mirror ) );
	triangles.push_back( Triangle( C, E, G, purple, Mirror ) );

	// Right wall
	triangles.push_back( Triangle( F, B, D, yellow, Rough ) );
	triangles.push_back( Triangle( H, F, D, yellow, Rough ) );

	// Ceiling
	triangles.push_back( Triangle( E, F, G, cyan, Rough ) );
	triangles.push_back( Triangle( F, H, G, cyan, Rough ) );

	// Back wall
	triangles.push_back( Triangle( G, D, C, white, Rough ) );
	triangles.push_back( Triangle( G, H, D, white, Rough ) );

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
	triangles.push_back( Triangle(E,B,A,red, Glass) );
	triangles.push_back( Triangle(E,F,B,red, Glass) );

	// Front
	triangles.push_back( Triangle(F,D,B,red, Glass) );
	triangles.push_back( Triangle(F,H,D,red, Glass) );

	// BACK
	triangles.push_back( Triangle(H,C,D,red, Glass) );
	triangles.push_back( Triangle(H,G,C,red, Glass) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,red, Glass) );
	triangles.push_back( Triangle(E,A,C,red, Glass) );

	// TOP
	triangles.push_back( Triangle(G,F,E,red, Glass) );
	triangles.push_back( Triangle(G,H,F,red, Glass) );

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
	triangles.push_back( Triangle(E,B,A,blue, Rough) );
	triangles.push_back( Triangle(E,F,B,blue, Rough) );

	// Front
	triangles.push_back( Triangle(F,D,B,blue, Rough) );
	triangles.push_back( Triangle(F,H,D,blue, Rough) );

	// BACK
	triangles.push_back( Triangle(H,C,D,blue, Rough) );
	triangles.push_back( Triangle(H,G,C,blue, Rough) );

	// LEFT
	triangles.push_back( Triangle(G,E,C,blue, Rough) );
	triangles.push_back( Triangle(E,A,C,blue, Rough) );

	// TOP
	triangles.push_back( Triangle(G,F,E,blue, Rough) );
	triangles.push_back( Triangle(G,H,F,blue, Rough) );



	// ----------------------------------------------
	// Scale to the volume [-1,1]^3
	for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= vec4(1,1,1,1);
		triangles[i].v1 -= vec4(1,1,1,1);
		triangles[i].v2 -= vec4(1,1,1,1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].v0.w = 1.0;
		triangles[i].v1.w = 1.0;
		triangles[i].v2.w = 1.0;

		triangles[i].ComputeNormal();
	}
}

#endif

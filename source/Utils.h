#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// 3-4 fps geometric formula
			/*Vector3 line{sphere.origin - ray.origin };
			float dp{ Vector3::Dot(line,ray.direction) };
			float lineLenght{ line.Normalize() };
			float od{ sqrtf(lineLenght * lineLenght - dp * dp) };
			float tca{ sqrtf(sphere.radius * sphere.radius - od * od) };
			float t{ dp - tca };
			Vector3 intersectPoint{ ray.origin + t * ray.direction };*/

			// 17-18 fps discriminant formula
			float A{ Vector3::Dot(ray.direction,ray.direction)};
			float B{ Vector3::Dot(2 * ray.direction,(ray.origin - sphere.origin)) };
			float C{ Vector3::Dot((ray.origin - sphere.origin),(ray.origin - sphere.origin))  - (sphere.radius * sphere.radius) };
			float discriminant{ B * B - 4 * A * C };

			if (discriminant > 0)
			{
				float t{ (- B - sqrtf(B * B - 4 * A * C))/(2*A)};
				Vector3 intersectPoint{ ray.origin + t * ray.direction };

				if (t > 0 && t > ray.min && t < ray.max)
				{
					intersectPoint = ray.origin + t * ray.direction;
					hitRecord.didHit = true;
					hitRecord.origin = intersectPoint;
					hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
					hitRecord.t = t;
					return true;
				}
			}
			hitRecord.didHit = false;
			return false;

			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			////todo W1
			float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal) / Vector3::Dot(ray.direction,plane.normal) };
			if (t > ray.min && t < ray.max)
			{
				hitRecord.didHit = true;
				Vector3 intersectPoint{ ray.origin + t * ray.direction };
				hitRecord.origin = intersectPoint;
				hitRecord.normal = plane.normal;
				hitRecord.t = t;
				return true;
			}
			hitRecord.didHit = false;
			return false;

			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Vector3 a{ triangle.v1 - triangle.v0 };
			Vector3 b{ triangle.v2 - triangle.v0 };
			Vector3 normal{ triangle.normal };

			TriangleCullMode triangleCull = triangle.cullMode;
			if (ignoreHitRecord)
			{
				switch (triangle.cullMode)
				{
				case dae::TriangleCullMode::FrontFaceCulling:
					triangleCull = TriangleCullMode::BackFaceCulling;
					break;
				case dae::TriangleCullMode::BackFaceCulling:
					triangleCull = TriangleCullMode::FrontFaceCulling;
					break;
				case dae::TriangleCullMode::NoCulling:
					triangleCull = TriangleCullMode::NoCulling;
					break;
				default:
					break;
				}
			}

			if ((Vector3::Dot(normal,ray.direction) != 0 && triangleCull == TriangleCullMode::NoCulling) ||
				(Vector3::Dot(normal, ray.direction) < 0 && triangleCull == TriangleCullMode::BackFaceCulling) ||
				(Vector3::Dot(normal, ray.direction) > 0 && triangleCull == TriangleCullMode::FrontFaceCulling))
			{
				Vector3 l{ ((triangle.v0+triangle.v1+triangle.v2)/3.f)-ray.origin};
				float t{ Vector3::Dot(l, normal) / Vector3::Dot(ray.direction, normal) };
				if (t > ray.min && t < ray.max)
				{
					Vector3 intersectPoint{ ray.origin + t * ray.direction };
					Vector3 pointToSideA{ intersectPoint - triangle.v0 };
					Vector3 edgeA{ triangle.v1 - triangle.v0 };
					Vector3 pointToSideB{ intersectPoint - triangle.v1 };
					Vector3 edgeB{ triangle.v2 - triangle.v1 };
					Vector3 pointToSideC{ intersectPoint - triangle.v2 };
					Vector3 edgeC{ triangle.v0 - triangle.v2 };

					if (Vector3::Dot(normal, Vector3::Cross(edgeA,pointToSideA)) > 0 
						&& Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSideB)) > 0
						&& Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSideC)) > 0)
					{
						hitRecord.didHit = true;
						hitRecord.origin = intersectPoint;
						hitRecord.normal = normal;
						hitRecord.t = t;
						return true;
					}
				}
			}

			hitRecord.didHit = false;
			return false;
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float currentClosedHit{0};
			bool firstHit{ false };
			Vector3 closeHitOrigin;
			Vector3 closeHitNormal;

			for (size_t i = 0; i < mesh.indices.size(); i += 3)
			{
				Vector3 v0 = mesh.transformedPositions[mesh.indices[i]];
				Vector3 v1 = mesh.transformedPositions[mesh.indices[i + 1]];
				Vector3 v2 = mesh.transformedPositions[mesh.indices[i + 2]];
				Triangle triangle{ v0,v1,v2 };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;
				triangle.normal = mesh.transformedNormals[i/3];
				GeometryUtils::HitTest_Triangle(triangle, ray, hitRecord,ignoreHitRecord);
				if (hitRecord.didHit)
				{
					if (!firstHit)
					{
						currentClosedHit = hitRecord.t;
						closeHitOrigin = hitRecord.origin;
						closeHitNormal = hitRecord.normal;
						firstHit = true;
						hitRecord.materialIndex = triangle.materialIndex;
					}
					if (currentClosedHit > hitRecord.t)
					{
						currentClosedHit = hitRecord.t;
						closeHitOrigin = hitRecord.origin;
						closeHitNormal = hitRecord.normal;
						hitRecord.materialIndex = triangle.materialIndex;
					}
					else
					{
						hitRecord.origin = closeHitOrigin;
						hitRecord.normal = closeHitNormal;
						hitRecord.t = currentClosedHit;
					}
				}
			}
			hitRecord.didHit = firstHit;
			return hitRecord.didHit;
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			switch (light.type)
			{
			case LightType::Point:
				return light.origin - origin;
				break;
			case LightType::Directional:
				break;
			default:
				break;
			}
			assert(false && "No Implemented Yet!");
			return {};
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			switch (light.type)
			{
			case LightType::Point:
				return light.color * (light.intensity/powf((light.origin-target).Normalize(),2));
				break;
			case LightType::Directional:
				break;
			default:
				break;
			}
			assert(false && "No Implemented Yet!");
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}
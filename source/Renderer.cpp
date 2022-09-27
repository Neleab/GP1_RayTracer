//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Vector3 cameraOrigen{ 0,0,0 };
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float widthScreen{ static_cast<float>(m_Width) };
			float heightScreen{ static_cast<float>(m_Height) };

			float aspectRatio{ widthScreen/ heightScreen};
			float pixelX = px + 0.5f;
			float pixelY = py + 0.5f;

			float cameraX{ ((2.f * (pixelX + 0.5f) / widthScreen) - 1.f) * aspectRatio };
			float cameraY{ 1.f - (2.f * pixelY)/ heightScreen};

			Vector3 rayDirection{ cameraX ,cameraY ,1 };
			Vector3 rayDirectionNormelized{ rayDirection.Normalized() };

			Ray hitRay{ cameraOrigen,rayDirectionNormelized};

			//ColorRGB finalColor{ rayDirectionNormelized.x, rayDirectionNormelized.y, rayDirectionNormelized.z };

			//TODO#4
			ColorRGB finalColor{};
			HitRecord closeHit{};

			//Sphere testSphere{ Vector3{0.f,0.f,100.f},50.f,0 };
			//GeometryUtils::HitTest_Sphere(testSphere, hitRay, closeHit);
			//if (closeHit.didHit)
			//{
			//	const float scaled_t{ (closeHit.t - 50.f) / 40.f };
			//	//finalColor = materials[closeHit.materialIndex]->Shade();
			//	//TODO 5
			//	finalColor = { scaled_t, scaled_t ,scaled_t };
			//}

			//TODO6
			pScene->GetClosestHit(hitRay,closeHit);
			if (closeHit.didHit)
			{
				finalColor = materials[closeHit.materialIndex]->Shade();
			}

			//TODO 9/10
			//Plane testPlane{ {0.f,-50.f,0.f},{0.f,1.f,0.f},0 };
			//GeometryUtils::HitTest_Plane(testPlane, hitRay, closeHit);
			//if (closeHit.didHit)
			//{
			//	const float scaled_t{ closeHit.t / 500.f };
			//	//finalColor = materials[closeHit.materialIndex]->Shade();
			//	finalColor = { scaled_t, scaled_t ,scaled_t };
			//}


			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

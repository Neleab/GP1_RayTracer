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
	float fov = tanf(camera.fovAngle * TO_RADIANS / 2);
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

			float cameraX{ ((2.f * (pixelX + 0.5f) / widthScreen) - 1.f) * aspectRatio * fov };
			float cameraY{ (1.f - (2.f * pixelY)/ heightScreen) * fov };

			Vector3 rayDirection{ cameraX ,cameraY ,1 };
			rayDirection = camera.CalculateCameraToWorld().TransformVector(rayDirection);
			Vector3 rayDirectionNormelized{ rayDirection.Normalized() };

			Ray hitRay{ camera.origin,rayDirectionNormelized};

			ColorRGB finalColor{};
			HitRecord closeHit{};

			pScene->GetClosestHit(hitRay,closeHit);
			if (closeHit.didHit)
			{
				//Lights

				for (size_t i = 0; i < pScene->GetLights().size(); i++)
				{
					Vector3 lightDirection{ LightUtils::GetDirectionToLight(pScene->GetLights().at(i), closeHit.origin) };
					Vector3 lightDirectionNormalized = lightDirection.Normalized();
					Vector3 offsetOrigin = closeHit.origin + closeHit.normal * 0.0001f;
					Ray lightRay{ offsetOrigin, lightDirectionNormalized};

					lightRay.max = lightDirection.Normalize();
					lightRay.min = 0.0001f;

					if (!pScene->DoesHit(lightRay) || !m_ShadowsEnabled)
					{
						float diffusedLight = Vector3::Dot(closeHit.normal, lightDirection);
						Vector3 fromHitToCamera = camera.origin - closeHit.origin;

						if (diffusedLight >= 0)
						{
							ColorRGB Radiance = LightUtils::GetRadiance(pScene->GetLights().at(i), closeHit.origin);
							switch (m_CurrentLightingMode)
							{
							case dae::Renderer::LightingMode::ObservedArea:
								finalColor += ColorRGB{1,1,1} * diffusedLight;
								break;
							case dae::Renderer::LightingMode::Radiance:
								finalColor += Radiance;
								break;
							case dae::Renderer::LightingMode::BRDF:
								finalColor += materials[closeHit.materialIndex]->Shade(closeHit, lightDirection.Normalized(), -rayDirection.Normalized());
								break;
							case dae::Renderer::LightingMode::Combined:
								finalColor += Radiance
									* materials[closeHit.materialIndex]->Shade(closeHit, lightDirection.Normalized(), -rayDirection.Normalized())
									* diffusedLight;
								break;
							default:
								break;
							}
						}
					}
				}
			}

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

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case dae::Renderer::LightingMode::ObservedArea:
		m_CurrentLightingMode = dae::Renderer::LightingMode::Radiance;
		break;
	case dae::Renderer::LightingMode::Radiance:
		m_CurrentLightingMode = dae::Renderer::LightingMode::BRDF;
		break;
	case dae::Renderer::LightingMode::BRDF:
		m_CurrentLightingMode = dae::Renderer::LightingMode::Combined;
		break;
	case dae::Renderer::LightingMode::Combined:
		m_CurrentLightingMode = dae::Renderer::LightingMode::ObservedArea;
		break;
	default:
		break;
	}
}

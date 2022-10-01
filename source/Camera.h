#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{ Vector3::UnitZ };//0.266f,-0.453f,0.860f
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			Vector3 rightVector = Vector3::Cross(Vector3::UnitY, forward);
			Vector3 rightVectorNormalized = rightVector.Normalized();
			Vector3 upVector = Vector3::Cross(forward, rightVectorNormalized);
			Vector3 upVectorNormalized = upVector.Normalized();
			cameraToWorld = Matrix(rightVectorNormalized, upVectorNormalized, forward, origin);
			return cameraToWorld;
			assert(false && "Not Implemented Yet");
			return {};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float speedMovement = 100;
			const float speedRotation = 10;
			Matrix totalPitch;
			Matrix totalYaw;
			 
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin.z += speedMovement * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin.z -= speedMovement * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin.x -= speedMovement * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin.x += speedMovement * deltaTime;
			}


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState || (SDL_BUTTON(SDL_BUTTON_LEFT) && SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState))
			{
				origin.z -= speedMovement * mouseY * deltaTime;
			}
			if (SDL_BUTTON(SDL_BUTTON_LEFT) & mouseState)
			{
				totalPitch = Matrix::CreateRotationX((speedRotation * (mouseX*-1) * deltaTime) * TO_RADIANS);
				Matrix totalRotation = totalPitch * totalYaw;

				forward = totalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalized();
			}
			if (SDL_BUTTON(SDL_BUTTON_RIGHT) & mouseState)
			{
				totalPitch = Matrix::CreateRotationX((speedRotation * (mouseX * -1) * deltaTime) * TO_RADIANS);
				totalYaw = Matrix::CreateRotationY((speedRotation * (mouseY * -1) * deltaTime) * TO_RADIANS);
				Matrix totalRotation = totalPitch * totalYaw;

				forward = totalRotation.TransformVector(Vector3::UnitZ);
				forward.Normalized();
			}

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}

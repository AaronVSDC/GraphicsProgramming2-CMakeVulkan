#pragma once
#include "Model.h"

//libs
#include <glm\gtc\matrix_transform.hpp>

//std 
#include <memory>

namespace cve
{

	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f,1.f, 1.f}; 
		glm::vec3 rotation; 


		//rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)

		//if wanting to inerpret as INTRINSIC rotations: read translations LEFT TO RIGHT (PITCH, YAW, ROLL) 
		//if wanting to interpret as EXTRINSIC rotations: read translations RIGHT TO LEFT
		//toalso optimize this by programming this in the shaders
		//TODO: also optimize this by programming this in the shaders
		glm::mat4 mat4() 
		{
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f} };
		}
	};


	class GameObject
	{
	public: 
		using id_t = unsigned int; 

		static GameObject CreateGameObject()
		{
			static id_t currentID = 0; 
			return GameObject{ currentID++ }; 
		}
		GameObject(const GameObject& other) = delete; 
		GameObject(GameObject&& ohter) = default; 
		GameObject& operator=(const GameObject& rhs) = delete; 
		GameObject& operator=(GameObject&& rhs) = default;





		id_t GetID() const { return m_ID; }

		std::shared_ptr<Model> m_Model{}; 
		glm::vec3 m_Color{}; 
		TransformComponent m_Transform{}; 

	private: 

		GameObject(id_t gameObjID) : m_ID{ gameObjID } {}
		
		//keep unique id for every game object
		id_t m_ID; 






	};



}
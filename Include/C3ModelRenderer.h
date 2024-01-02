// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <C3.h>
#include <C3Component.h>


namespace c3
{

	// ModelRenderer is a Component that you can attach to an Object to make it render a Model when it's Draw function is called

	class C3_API ModelRenderer : public Component
	{

	public:

		static const ComponentType *Type();

		// Sets the origin position of the model (offsets any effects of Positionable); linked to 'MPOS' property
		virtual void SetPos(float x, float y, float z) = NULL;

		// Gets the origina position of the model
		virtual const glm::fvec3 *GetPosVec(glm::fvec3 *pos = nullptr) = NULL;

		// Sets the origin orientation of the model (offsets any effects of Positionable); linked to 'MORI' property
		virtual void SetOriQuat(const glm::fquat *ori) = NULL;

		// Gets the origin orientation of the model
		virtual const glm::fquat *GetOriQuat(glm::fquat *ori = nullptr) = NULL;

		// Sets the origin scvale of the model (offsets any effects of Positionable); linked to 'MSCL' property
		virtual void SetScl(float x, float y, float z) = NULL;

		// Gets the origin scale of the model
		virtual const glm::fvec3 *GetScl(glm::fvec3 *scl = nullptr) = NULL;

		// Gets the transform composed of the given pos, ori, scl values
		virtual const glm::fmat4x4 *GetMatrix(glm::fmat4x4 *mat = nullptr) const = NULL;

		// Returns the model that is being rendered
		virtual const Model *GetModel() const = NULL;

		// Returns the InstanceData for the rendered Model
		virtual Model::InstanceData *GetModelInstanceData() = NULL;

	};

};

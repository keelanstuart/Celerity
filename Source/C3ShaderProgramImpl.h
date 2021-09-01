// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#pragma once

#include <C3ShaderProgram.h>
#include <C3RendererImpl.h>

namespace c3
{

	class ShaderComponentImpl;

	class ShaderProgramImpl : public ShaderProgram
	{

	protected:
		RendererImpl *m_Rend;
		GLuint m_glID;

		bool m_Linked;
		ShaderComponentImpl *m_Comp[Renderer::ShaderComponentType::ST_NUMTYPES];

		typedef uint32_t UniformNameCRC;
		typedef std::map<UniformNameCRC, int64_t> TNameCRCToIdMap;
		TNameCRCToIdMap m_NameCrcToId;

		typedef enum EShaderUniformUse
		{
			SUU_AMBIENT = 0,
#if 0
			SUU_BOXX,
			SUU_BOXY,
			SUU_BOUNDINGBOXMAX,
			SUU_BOUNDINGBOXMIN,
			SUU_BOUNDINGBOXSIZE,
			SUU_BOUNDINGCENTER,
			SUU_BOUNDINGSPHERESIZE,
			SUU_BOUNDINGSPHEREMIN,
			SUU_BOUNDINGSPHEREMAX,
#endif
			SUU_ELAPSEDTIME,
			SUU_LASTTIME,
			SUU_PROJECTION,
			SUU_PROJECTIONINVERSE,
			SUU_PROJECTIONINVERSETRANSPOSE,
			SUU_RANDOM,
			SUU_REFRACTION,
			SUU_RENDERCOLORTARGET,
			SUU_RENDERDEPTHSTENCILTARGET,
			SUU_RENDERTARGETCLIPPING,
			SUU_RENDERTARGETDIMENSIONS,
			SUU_TIME,
			SUU_FRAMENUMBER,
			SUU_VIEW,
			SUU_VIEWINVERSE,
			SUU_VIEWINVERSETRANSPOSE,
			SUU_VIEWPROJECTION,
			SUU_VIEWPROJECTIONINVERSE,
			SUU_VIEWPROJECTIONINVERSETRANSPOSE,
			SUU_WORLD,
			SUU_WORLDINVERSE,
			SUU_WORLDINVERSETRANSPOSE,
			SUU_WORLDVIEW,
			SUU_WORLDVIEWINVERSE,
			SUU_WORLDVIEWINVERSETRANSPOSE,
			SUU_WORLDVIEWPROJECTION,
			SUU_WORLDVIEWPROJECTIONINVERSE,
			SUU_WORLDVIEWPROJECTIONINVERSETRANSPOSE,

			SUU_NUMUSES

		} ShaderUniformUse;

		props::IPropertySet *m_Uniforms;

	public:

		ShaderProgramImpl(RendererImpl *prend);
		virtual ~ShaderProgramImpl();

		virtual void Release();

		virtual ShaderProgram::RETURNCODE AttachShader(ShaderComponent *pshader);
		virtual ShaderProgram::RETURNCODE Link();
		virtual bool IsLinked();

		enum { INVALID_UNIFORM = -1 };

		virtual int64_t GetUniformLocation(const TCHAR *name);
		virtual bool SetUniformMatrix(int64_t location, const glm::fmat4x4 *mat);
		virtual bool SetUniform1(int64_t location, float f);
		virtual bool SetUniform2(int64_t location, const glm::fvec2 *v2);
		virtual bool SetUniform3(int64_t location, const glm::fvec3 *v3);
		virtual bool SetUniform4(int64_t location, const glm::fvec4 *v4);
		virtual bool SetUniformTexture(int64_t location, uint64_t sampler, Texture *tex);

		/// collects all the global (world, view, projection xforms, etc)
		void CaptureGlobalUniforms();

		virtual void UpdateGlobalUniforms();

		operator GLuint() const { return m_glID; }

	};

	DEFINE_RESOURCETYPE(ShaderProgram, RTFLAG_RUNBYRENDERER, GUID({0x9c62e3e0, 0x95d0, 0x4023, { 0xad, 0xb3, 0x6a, 0xb2, 0xd5, 0xcf, 0x4e, 0x9a }}), "ShaderProgram", "Shader Programs (GLSL)", "c3shader", "c3shader");

};

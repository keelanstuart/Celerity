// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2021, Keelan Stuart


#include "pch.h"

#include <C3ShaderProgramImpl.h>
#include <C3ShaderComponentImpl.h>
#include <C3TextureImpl.h>


using namespace c3;


//#define IMMEDIATE_UNIFORMS

ShaderProgramImpl::ShaderProgramImpl(RendererImpl *prend)
{
	m_Rend = prend;
	m_glID = GL_INVALID_VALUE;
	m_Linked = false;
	memset(m_Comp, 0, sizeof(ShaderComponentImpl *) * Renderer::ShaderComponentType::ST_NUMTYPES);
	m_Uniforms = props::IPropertySet::CreatePropertySet();
}


ShaderProgramImpl::~ShaderProgramImpl()
{
	m_Uniforms->Release();

	if (m_Rend && (m_glID != GL_INVALID_VALUE))
	{
		for (UINT i = (Renderer::ShaderComponentType::ST_NONE + 1), maxi = Renderer::ShaderComponentType::ST_NUMTYPES; i < maxi; i++)
		{
			if (!m_Comp[i])
				continue;

			m_Rend->gl.DetachShader(m_glID, (ShaderComponentImpl &)*m_Comp[i]);
			m_Comp[i] = nullptr;
		}

		m_Rend->gl.DeleteProgram(m_glID);
		m_glID = GL_INVALID_VALUE;
	}
}


void ShaderProgramImpl::Release()
{
	delete this;
}


ShaderProgram::RETURNCODE ShaderProgramImpl::AttachShader(ShaderComponent *pshader)
{
	if (!pshader)
		return ShaderProgram::RETURNCODE::RET_NULL_SHADER;

	int shtype = pshader->Type();
	if ((shtype < 0) || (shtype >= c3::Renderer::ShaderComponentType::ST_NUMTYPES))
		return ShaderProgram::RETURNCODE::RET_BAD_TYPE;

	if (m_glID == GL_INVALID_VALUE)
		m_glID = m_Rend->gl.CreateProgram();

	if (m_glID == GL_INVALID_VALUE)
		return ShaderProgram::RETURNCODE::RET_CREATE_FAILED;

	if (m_Comp[shtype])
		m_Rend->gl.DetachShader(m_glID, (ShaderComponentImpl &)*m_Comp[shtype]);
	m_Comp[shtype] = (ShaderComponentImpl *)pshader;

	m_Linked = false;
	m_Uniforms->DeleteAll();

	m_Rend->gl.AttachShader(m_glID, (ShaderComponentImpl &)*pshader);

	return ShaderProgram::RETURNCODE::RET_OK;
}


ShaderProgram::RETURNCODE ShaderProgramImpl::Link()
{
	if (m_glID == GL_INVALID_VALUE)
		return ShaderProgram::RETURNCODE::RET_CREATE_FAILED;

	m_Rend->gl.LinkProgram(m_glID);

	GLint link_ok = 0;
	m_Rend->gl.GetProgramiv(m_glID, GL_LINK_STATUS, &link_ok);
	if (link_ok == GL_FALSE)
	{
		m_Rend->GetSystem()->GetLog()->Print(_T("* shader link error:"));

		GLint maxlen = 0;
		m_Rend->gl.GetProgramiv(m_glID, GL_INFO_LOG_LENGTH, &maxlen);

		if (maxlen)
		{
			// The maxLength includes the NULL character
			char *pserr = (char *)_alloca(maxlen);
			m_Rend->gl.GetProgramInfoLog(m_glID, maxlen, &maxlen, pserr);
			TCHAR *tpserr;
			CONVERT_MBCS2TCS(pserr, tpserr);

			m_Rend->GetSystem()->GetLog()->Print(_T("\n\t%s\n\n"), tpserr);
		}
		else
		{
			m_Rend->GetSystem()->GetLog()->Print(_T(" unspecified\n\n"));
		}

		m_Linked = false;

		return ShaderProgram::RETURNCODE::RET_LINK_FAILED;
	}

	m_Linked = true;
	CaptureUniforms();

	return ShaderProgram::RETURNCODE::RET_OK;
}


bool ShaderProgramImpl::IsLinked()
{
	return m_Linked;
}


int32_t ShaderProgramImpl::GetUniformLocation(const TCHAR *name)
{
	if (!m_Linked || !name)
		return INVALID_UNIFORM;

	props::IProperty *p = m_Uniforms->GetPropertyByName(name);
	if (p)
		return p->GetID();

	return INVALID_UNIFORM;
}


bool ShaderProgramImpl::SetUniformMatrix(int32_t location, const glm::fmat4x4 *mat)
{
	if ((location < 0) || !mat)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetMat4x4F((const props::TMat4x4F *)mat);

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->gl.ProgramUniformMatrix4fv(m_glID, (GLint)location, 1, GL_FALSE, (const GLfloat *)mat);
#endif

	return true;
}


bool ShaderProgramImpl::SetUniform1(int32_t location, float f)
{
	if (location < 0)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetFloat(f);

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->gl.ProgramUniform1f(m_glID, (GLint)location, f);
#endif

	return true;
}


bool ShaderProgramImpl::SetUniform2(int32_t location, const glm::fvec2 *v2)
{
	if ((location < 0) || !v2)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetVec2F(*(const props::TVec2F *)v2);

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->gl.ProgramUniform2fv(m_glID, (GLint)location, 1, (const GLfloat *)v2);
#endif

	return true;
}


bool ShaderProgramImpl::SetUniform3(int32_t location, const glm::fvec3 *v3)
{
	if ((location < 0) || !v3)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetVec3F(*(const props::TVec3F *)v3);

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->gl.ProgramUniform3fv(m_glID, (GLint)location, 1, (const GLfloat *)v3);
#endif

	return true;
}


bool ShaderProgramImpl::SetUniform4(int32_t location, const glm::fvec4 *v4)
{
	if ((location < 0) || !v4)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetVec4F(*(const props::TVec4F *)v4);

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->gl.ProgramUniform4fv(m_glID, (GLint)location, 1, (const GLfloat *)v4);
#endif

	return true;
}


bool ShaderProgramImpl::SetUniformTexture(int32_t location, uint32_t sampler, Texture *tex)
{
	if (location < 0)
		return false;

	props::IProperty *p = m_Uniforms->GetPropertyById(location);
	if (!p)
		return false;

	p->SetInt((int64_t)tex);
	p->SetAspect(props::IProperty::PROPERTY_ASPECT((size_t)props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_DIFFUSE + sampler));

#ifdef IMMEDIATE_UNIFORMS
	m_Rend->UseTexture(sampler, tex);
	m_Rend->gl.ProgramUniform1i(m_glID, (GLint)location, (GLint)sampler);
#endif

	return true;
}


void ShaderProgramImpl::CaptureUniforms()
{
	if (!m_Linked)
		return;

	GLint total = 0;
	m_Rend->gl.GetProgramiv(m_glID, GL_ACTIVE_UNIFORMS, &total); 
	for (GLint i = 0; i < total; i++)
	{
		GLint name_len = 0;
		GLint num = -1;
		GLenum type = GL_ZERO;

		char name[100];

		m_Rend->gl.GetActiveUniform(m_glID, GLuint(i), sizeof(name) - 1, &name_len, &num, &type, name);
		name[name_len] = 0;

		GLuint location = m_Rend->gl.GetUniformLocation(m_glID, name);

		TCHAR *n;
		CONVERT_MBCS2TCS(name, n);
		props::IProperty *p = m_Uniforms->CreateProperty(n, location);

		switch (type)
		{
			case GL_FLOAT_MAT3:
			{
				props::TMat3x3F tmp;
				p->SetMat3x3F(&tmp);
				break;
			}

			case GL_FLOAT_MAT4:
			{
				props::TMat4x4F tmp;
				p->SetMat4x4F(&tmp);

				if (!_tcscmp(n, _T("uMatrixMVP")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_WORLDVIEWPROJECTION);
				else if (!_tcscmp(n, _T("uMatrixN")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_WORLDVIEWINVTRANS);
				else if (!_tcscmp(n, _T("uMatrixP")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_PROJECTION);

				break;
			}

			case GL_FLOAT_VEC4:
				p->SetVec4F(props::TVec4F(1, 1, 1, 1));

				if (!_tcscmp(n, _T("uColorDiffuse")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE);
				else if (!_tcscmp(n, _T("uColorEmissive")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE);
				else if (!_tcscmp(n, _T("uColorSpecular")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR);

				break;

			case GL_FLOAT_VEC3:
				p->SetVec3F(props::TVec3F(0, 0, 0));

				if (!_tcscmp(n, _T("uColorDiffuse")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_DIFFUSE);
				else if (!_tcscmp(n, _T("uColorEmissive")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_EMISSIVE);
				else if (!_tcscmp(n, _T("uColorSpecular")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_COLOR_SPECULAR);

				break;

			case GL_FLOAT_VEC2:
				p->SetVec2F(props::TVec2F(0, 0));
				break;

			case GL_FLOAT:
				p->SetFloat(0);
				break;

			case GL_INT:
				p->SetInt(0);
				break;

			case GL_BOOL:
				p->SetBool(false);
				break;

			case GL_SAMPLER_2D:
				p->SetInt(0);

				if (!_tcscmp(n, _T("uSamplerDiffuse")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_DIFFUSE);
				else if (!_tcscmp(n, _T("uSamplerNormal")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_NORMAL);
				else if (!_tcscmp(n, _T("uSamplerSurfDesc")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_SURFDESC);
				else if (!_tcscmp(n, _T("uSamplerEmissive")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_EMISSIVE);
				else if (!_tcscmp(n, _T("uSamplerPosDepth")))
					p->SetAspect(props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_POSITIONDEPTH);

				break;
		}
	}
}

void ShaderProgramImpl::UpdateGlobalUniforms()
{
	for (size_t i = 0, maxi = m_Uniforms->GetPropertyCount(); i < maxi; i++)
	{
		props::IProperty *p = m_Uniforms->GetProperty(i);
		switch (p->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_MAT4X4:
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_WORLDVIEWPROJECTION:
						SetUniformMatrix(p->GetID(), m_Rend->GetWorldViewProjectionMatrix());
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_WORLDVIEWINVTRANS:
						SetUniformMatrix(p->GetID(), m_Rend->GetNormalMatrix());
						break;

					case props::IProperty::PROPERTY_ASPECT::PA_PROJECTION:
						SetUniformMatrix(p->GetID(), m_Rend->GetProjectionMatrix());
						break;
				}
				break;
		}
	}
}

void ShaderProgramImpl::ApplyUniforms(bool update_globals)
{
	if (update_globals)
		UpdateGlobalUniforms();

#ifdef IMMEDIATE_UNIFORMS
	return;
#endif

	for (size_t i = 0, maxi = m_Uniforms->GetPropertyCount(); i < maxi; i++)
	{
		props::IProperty *p = m_Uniforms->GetProperty(i);
		switch (p->GetType())
		{
			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_MAT3X3:
				m_Rend->gl.ProgramUniformMatrix3fv(m_glID, (GLint)p->GetID(), 1, GL_FALSE, (const GLfloat *)(p->AsMat3x3F()));
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_MAT4X4:
				m_Rend->gl.ProgramUniformMatrix4fv(m_glID, (GLint)p->GetID(), 1, GL_FALSE, (const GLfloat *)(p->AsMat4x4F()));
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V4:
				m_Rend->gl.ProgramUniform4fv(m_glID, (GLint)(GLint)p->GetID(), 1, (const GLfloat *)p->AsVec4F());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V3:
				m_Rend->gl.ProgramUniform3fv(m_glID, (GLint)(GLint)p->GetID(), 1, (const GLfloat *)p->AsVec3F());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT_V2:
				m_Rend->gl.ProgramUniform3fv(m_glID, (GLint)(GLint)p->GetID(), 1, (const GLfloat *)p->AsVec2F());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_FLOAT:
				m_Rend->gl.ProgramUniform1f(m_glID, (GLint)p->GetID(), p->AsFloat());
				break;

			case props::IProperty::PROPERTY_TYPE::PT_INT:
				switch (p->GetAspect())
				{
					case props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_DIFFUSE:
					case props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_NORMAL:
					case props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_SURFDESC:
					case props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_EMISSIVE:
					case props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_POSITIONDEPTH:
					{
						GLint sampler = p->GetAspect() - props::IProperty::PROPERTY_ASPECT::PA_SAMPLER2D_DIFFUSE;

						m_Rend->UseTexture(sampler, (Texture *)p->AsInt());
						m_Rend->gl.ProgramUniform1i(m_glID, (GLint)p->GetID(), (GLint)sampler);
						break;
					}

					default:
						m_Rend->gl.ProgramUniform1i(m_glID, (GLint)p->GetID(), (GLint)p->AsInt());
						break;
				}
				break;

			case props::IProperty::PROPERTY_TYPE::PT_BOOLEAN:
				break;
		}
	}
}


DECLARE_RESOURCETYPE(ShaderProgram);

c3::ResourceType::LoadResult RESOURCETYPENAME(ShaderProgram)::ReadFromFile(c3::System *psys, const TCHAR *filename, void **returned_data) const
{
	if (returned_data)
	{
		*returned_data = psys->GetRenderer()->CreateShaderProgram();
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;

		char *fn;
		CONVERT_TCS2MBCS(filename, fn);
		tinyxml2::XMLDocument doc;
		if (!doc.LoadFile(fn) != tinyxml2::XMLError::XML_SUCCESS)
			return ResourceType::LoadResult::LR_ERROR;

		static const char *shader_tagname[Renderer::ShaderComponentType::ST_NUMTYPES] =
		{
			"vertex_shader",
			"fragment_shader",
			"geometry_shader",
			"tesseval_shader",
			"tesscontrol_shader"
		};

		for (size_t sti = 0; sti < Renderer::ShaderComponentType::ST_NUMTYPES; sti++)
		{
			tinyxml2::XMLElement *pel = doc.FirstChildElement(shader_tagname[sti]);
			if (pel)
			{
				const tinyxml2::XMLAttribute *pa = pel->FindAttribute("filename");
				if (pa)
				{
					TCHAR *sfn, ffn[MAX_PATH];
					CONVERT_MBCS2TCS(pa->Value(), sfn);
					if (!psys->GetFileMapper()->FindFile(sfn, ffn, MAX_PATH))
					{
						psys->GetLog()->Print(_T("Unable to locate \"%S\" referenced by %s tag by \"%S\"\n"), ffn, shader_tagname[sti], filename);
						continue;
					}

					HANDLE hf = CreateFile(ffn, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL);
					if (hf == INVALID_HANDLE_VALUE)
					{
						psys->GetLog()->Print(_T("Error opening \"%S\"\n"), ffn);
						continue;
					}

					DWORD rb = 0, fsz = GetFileSize(hf, nullptr);
					if (!fsz)
						continue;

					char *sh = (char *)malloc(fsz);
					if (sh)
					{
						if (ReadFile(hf, sh, fsz, &rb, nullptr))
						{
							ShaderComponent *psc = psys->GetRenderer()->CreateShaderComponent((Renderer::ShaderComponentType)sti);

							TCHAR *_sh;
							CONVERT_MBCS2TCS(sh, _sh);
							psc->CompileProgram(_sh);
						}

						free(sh);
					}

					CloseHandle(hf);
				}
			}
		}

		((ShaderProgram *)*returned_data)->Link();
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(ShaderProgram)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, void **returned_data) const
{
	return ResourceType::LoadResult::LR_ERROR;
}


bool RESOURCETYPENAME(ShaderProgram)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(ShaderProgram)::Unload(void *data) const
{
	((ShaderProgram *)data)->Release();
}

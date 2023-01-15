// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2022, Keelan Stuart


#include "pch.h"

#include <C3Resource.h>
#include <C3ShaderComponentImpl.h>
#include <shlwapi.h>
#include <C3CRC.h>


using namespace c3;


ShaderComponentImpl::ShaderComponentImpl(RendererImpl *prend, Renderer::ShaderComponentType type)
{
	m_Rend = prend;

	m_Type = type;
	switch (m_Type)
	{
		case Renderer::ShaderComponentType::ST_VERTEX:
			m_glType = GL_VERTEX_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_FRAGMENT:
			m_glType = GL_FRAGMENT_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_GEOMETRY:
			m_glType = GL_GEOMETRY_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_TESSEVAL:
			m_glType = GL_TESS_EVALUATION_SHADER;
			break;

		case Renderer::ShaderComponentType::ST_TESSCONTROL:
			m_glType = GL_TESS_CONTROL_SHADER;
			break;
	}

	m_glID = NULL;
	m_Compiled = false;
}


ShaderComponentImpl::~ShaderComponentImpl()
{
	if (m_Rend && (m_glID != NULL))
	{
		m_Rend->gl.DeleteShader(m_glID);
		m_glID = NULL;
	}
}


void ShaderComponentImpl::Release()
{
	delete this;
}


Renderer::ShaderComponentType ShaderComponentImpl::Type()
{
	return m_Type;
}


ShaderComponent::RETURNCODE ShaderComponentImpl::CompileProgram(const TCHAR *program, const TCHAR *preamble)
{
	if (!program)
		return ShaderComponent::RETURNCODE::RET_NULL_PROGRAM;

	if (m_glID == NULL)
		m_glID = m_Rend->gl.CreateShader(m_glType);

	if (m_glID == NULL)
		return ShaderComponent::RETURNCODE::RET_CREATE_FAILED;

	m_ProgramText = program;

	m_CRC = Crc32::CalculateString(program);
	if (preamble)
		m_CRC = Crc32::CalculateString(preamble, m_CRC);

	size_t progidx = preamble ? 2 : 1;
	char *ps[3] = {"#version 410\n", nullptr, nullptr};
	CONVERT_TCS2MBCS(program, ps[progidx]);
	if (preamble)
	{
		CONVERT_TCS2MBCS(preamble, ps[1]);
	}
	m_Rend->gl.ShaderSource(m_glID, preamble ? 3 : 2, ps, NULL);

	m_Rend->gl.CompileShader(m_glID);

	m_Compiled = true;

	GLint compiled = 0;
	m_Rend->gl.GetShaderiv(m_glID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		m_Rend->GetSystem()->GetLog()->Print(_T("* shader compile error:"));

		GLint maxlen = 0;
		m_Rend->gl.GetShaderiv(m_glID, GL_INFO_LOG_LENGTH, &maxlen);

		if (maxlen)
		{
			// The maxLength includes the NULL character
			char *pserr = (char *)_alloca(maxlen);
			m_Rend->gl.GetShaderInfoLog(m_glID, maxlen, &maxlen, pserr);
			TCHAR *tpserr;
			CONVERT_MBCS2TCS(pserr, tpserr);

			m_Rend->GetSystem()->GetLog()->Print(_T("\n\t%s\n\n"), tpserr);
		}
		else
		{
			m_Rend->GetSystem()->GetLog()->Print(_T(" unspecified\n\n"));
		}

		m_Compiled = false;
	}

	if (!m_Compiled)
		return ShaderComponent::RETURNCODE::RET_COMPILE_FAILED;

	return ShaderComponent::RETURNCODE::RET_OK;
}


const TCHAR *ShaderComponentImpl::GetProgramText() const
{
	return m_ProgramText.c_str();
}


uint32_t ShaderComponentImpl::GetProgramCRC() const
{
	return m_CRC;
}


bool ShaderComponentImpl::IsCompiled() const
{
	return m_Compiled;
}


DECLARE_RESOURCETYPE(ShaderComponent);

c3::ResourceType::LoadResult RESOURCETYPENAME(ShaderComponent)::ReadFromFile(c3::System *psys, const TCHAR *filename, void **returned_data) const
{
	if (!filename || !*filename)
		return ResourceType::LoadResult::LR_ERROR;

	if (returned_data)
	{
		Renderer::ShaderComponentType t;
		TCHAR *e = PathFindExtension(filename);
		if (e && (*e == '.'))
		{
			e++;
			switch (*e)
			{
				case _T('e'):
				case _T('E'):
					t = Renderer::ShaderComponentType::ST_TESSEVAL;
					break;

				case _T('f'):
				case _T('F'):
					t = Renderer::ShaderComponentType::ST_FRAGMENT;
					break;

				case _T('g'):
				case _T('G'):
					t = Renderer::ShaderComponentType::ST_GEOMETRY;
					break;

				case _T('t'):
				case _T('T'):
					t = Renderer::ShaderComponentType::ST_TESSCONTROL;
					break;

				case _T('v'):
				case _T('V'):
					t = Renderer::ShaderComponentType::ST_VERTEX;
					break;

				default:
					return ResourceType::LoadResult::LR_ERROR;
			}
		}

		ShaderComponent *psh = psys->GetRenderer()->CreateShaderComponent(t);
		if (psh)
		{
			HANDLE h = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h != INVALID_HANDLE_VALUE)
			{
				DWORD sz = GetFileSize(h, nullptr);
				char *sc = (char *)malloc(sizeof(char) * (sz + 1));
				if (sc)
				{
					if (sz > 0)
					{
						DWORD br;
						ReadFile(h, sc, sz, &br, nullptr);
					}
					CloseHandle(h);

					sc[sz] = '\0';
					for (DWORD i = 0; i < sz; i++)
						if (sc[i] == '\r')
							sc[i] = '\n';

					TCHAR *tsc;
					CONVERT_MBCS2TCS(sc, tsc);
					free(sc);

					if (psh->CompileProgram(tsc) != ShaderComponent::RET_OK)
					{
						psh->Release();
						psh = nullptr;
					}
				}
			}
		}

		*returned_data = psh;
		if (!*returned_data)
			return ResourceType::LoadResult::LR_ERROR;
	}

	return ResourceType::LoadResult::LR_SUCCESS;
}


c3::ResourceType::LoadResult RESOURCETYPENAME(ShaderComponent)::ReadFromMemory(c3::System *psys, const BYTE *buffer, size_t buffer_length, void **returned_data) const
{
	return ResourceType::LoadResult::LR_ERROR;
}


bool RESOURCETYPENAME(ShaderComponent)::WriteToFile(c3::System *psys, const TCHAR *filename, const void *data) const
{
	return false;
}


void RESOURCETYPENAME(ShaderComponent)::Unload(void *data) const
{
	((ShaderComponent *)data)->Release();
}



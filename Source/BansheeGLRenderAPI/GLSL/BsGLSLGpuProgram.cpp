//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Glsl/BsGLSLGpuProgram.h"
#include "Renderapi/BsRenderAPI.h"
#include "Error/BsException.h"
#include "Glsl/BsGLSLParamParser.h"
#include "Managers/BsHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"
#include "Renderapi/BsGpuParams.h"

namespace bs { namespace ct
{
	UINT32 GLSLGpuProgram::mVertexShaderCount = 0;
	UINT32 GLSLGpuProgram::mFragmentShaderCount = 0;
	UINT32 GLSLGpuProgram::mGeometryShaderCount = 0;
	UINT32 GLSLGpuProgram::mDomainShaderCount = 0;
	UINT32 GLSLGpuProgram::mHullShaderCount = 0;
	UINT32 GLSLGpuProgram::mComputeShaderCount = 0;

	bool checkForGLSLError(const GLuint programObj, String& outErrorMsg)
	{
		StringStream stream;

		GLint linkCompileSuccess = 0;
		glGetProgramiv(programObj, GL_LINK_STATUS, &linkCompileSuccess);

		GLenum glErr;
		bool errorsFound = false;

		glErr = glGetError();
		while (glErr != GL_NO_ERROR)
		{
			const char* glerrStr = (const char*)gluErrorString(glErr);

			if (glerrStr)
			{
				if (errorsFound)
					stream << "\nPrevious errors: \n";

				stream << String(glerrStr) << std::endl;;
			}

			glErr = glGetError();
			errorsFound = true;
		}

		if ((errorsFound || !linkCompileSuccess) && programObj > 0)
		{
			GLint infologLength = 0;
			glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infologLength);

			if (infologLength > 0)
			{
				GLint charsWritten = 0;

				GLchar* infoLog = (GLchar*)bs_alloc(sizeof(GLchar)* infologLength);

				glGetProgramInfoLog(programObj, infologLength, &charsWritten, infoLog);

				stream << "Compile and linker info log: \n";
				stream << String(infoLog);

				bs_free(infoLog);
			}
		}

		outErrorMsg = stream.str();

		return errorsFound || !linkCompileSuccess;
	}
	
	GLSLGpuProgram::GLSLGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuProgram(desc, deviceMask), mProgramID(0), mGLHandle(0)
    { }

	GLSLGpuProgram::~GLSLGpuProgram()
    { 
		if (mIsCompiled && mGLHandle != 0)
		{
			glDeleteProgram(mGLHandle);
			mGLHandle = 0;
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuProgram);
	}

	void GLSLGpuProgram::initialize()
	{
		static const char* VERSION_LINE = "#version 450\n";
		
		if (!isSupported())
		{
			mIsCompiled = false;
			mCompileError = "Specified program is not supported by the current render system.";

			GpuProgram::initialize();
			return;
		}

		GLenum shaderType = 0x0000;
		switch (mProperties.getType())
		{
		case GPT_VERTEX_PROGRAM:
			shaderType = GL_VERTEX_SHADER;
			mProgramID = ++mVertexShaderCount;
			break;
		case GPT_FRAGMENT_PROGRAM:
			shaderType = GL_FRAGMENT_SHADER;
			mProgramID = ++mFragmentShaderCount;
			break;
		case GPT_GEOMETRY_PROGRAM:
			shaderType = GL_GEOMETRY_SHADER;
			mProgramID = ++mGeometryShaderCount;
			break;
		case GPT_HULL_PROGRAM:
			shaderType = GL_TESS_CONTROL_SHADER;
			mProgramID = ++mDomainShaderCount;
			break;
		case GPT_DOMAIN_PROGRAM:
			shaderType = GL_TESS_EVALUATION_SHADER;
			mProgramID = ++mHullShaderCount;
			break;
		case GPT_COMPUTE_PROGRAM:
			shaderType = GL_COMPUTE_SHADER;
			mProgramID = ++mComputeShaderCount;
			break;
		}

		// Add preprocessor extras and main source
		const String& source = mProperties.getSource();
		if (!source.empty())
		{
			Vector<GLchar*> lines;

			const char* versionStr = "#version ";
			UINT32 versionStrLen = (UINT32)strlen(versionStr);

			UINT32 lineLength = 0;
			INT32 versionLine = -1;
			for (UINT32 i = 0; i < source.size(); i++)
			{
				if (source[i] == '\n' || source[i] == '\r')
				{
					assert(sizeof(source[i]) == sizeof(GLchar));

					GLchar* lineData = (GLchar*)bs_stack_alloc(sizeof(GLchar) * (lineLength + 2));
					memcpy(lineData, &source[i - lineLength], sizeof(GLchar) * lineLength);

					lineData[lineLength] = '\n';
					lineData[lineLength + 1] = '\0';

					if(versionLine == -1 && lineLength >= versionStrLen)
					{
						bool isEqual = true;
						for (UINT32 j = 0; j < versionStrLen; ++j)
						{
							if(lineData[j] != versionStr[j])
							{
								isEqual = false;
								break;
							}
						}

						if (isEqual)
							versionLine = (INT32)lines.size();
					}

					lines.push_back(lineData);
					lineLength = 0;
				}
				else
				{
					lineLength++;
				}
			}

			if (lineLength > 0)
			{
				UINT32 end = (UINT32)source.size() - 1;
				assert(sizeof(source[end]) == sizeof(GLchar));

				GLchar* lineData = (GLchar*)bs_stack_alloc(sizeof(GLchar) * (lineLength + 1));
				memcpy(lineData, &source[source.size() - lineLength], sizeof(GLchar) * lineLength);
				lineData[lineLength] = '\0';

				lines.push_back(lineData);
				lineLength = 0;
			}

			int numInsertedLines = 0;
			if(versionLine == -1)
			{
				UINT32 length = (UINT32)strlen(VERSION_LINE) + 1;

				GLchar* extraLineData = (GLchar*)bs_stack_alloc(length);
				memcpy(extraLineData, VERSION_LINE, length);

				lines.insert(lines.begin(), extraLineData);
				numInsertedLines++;
			}

			static const char* EXTRA_LINES[] = { "#define OPENGL\n" };
			UINT32 numExtraLines = sizeof(EXTRA_LINES) / sizeof(EXTRA_LINES[0]);
			UINT32 extraLineOffset = versionLine != -1 ? versionLine + 1 : 0;
			for (UINT32 i = 0; i < numExtraLines; i++)
			{
				UINT32 length = (UINT32)strlen(EXTRA_LINES[i]) + 1;

				GLchar* extraLineData = (GLchar*)bs_stack_alloc(length);
				memcpy(extraLineData, EXTRA_LINES[i], length);

				lines.insert(lines.begin() + extraLineOffset + numInsertedLines, extraLineData);
				numInsertedLines++;
			}

			StringStream codeStream;
			for(auto& entry : lines)
				codeStream << entry;

			for (INT32 i = numInsertedLines - 1; i >= 0; i--)
				bs_stack_free(lines[extraLineOffset + i]);

			if (numInsertedLines > 0)
				lines.erase(lines.begin() + extraLineOffset, lines.begin() + extraLineOffset + numInsertedLines);

			for (auto iter = lines.rbegin(); iter != lines.rend(); ++iter)
				bs_stack_free(*iter);

			String code = codeStream.str();
			const char* codeRaw = code.c_str();
			mGLHandle = glCreateShaderProgramv(shaderType, 1, (const GLchar**)&codeRaw);

			mCompileError = "";
			mIsCompiled = !checkForGLSLError(mGLHandle, mCompileError);
		}

		if (mIsCompiled)
		{
			GLSLParamParser paramParser;
			paramParser.buildUniformDescriptions(mGLHandle, mProperties.getType(), *mParametersDesc);

			if (mProperties.getType() == GPT_VERTEX_PROGRAM)
			{
				List<VertexElement> elementList = paramParser.buildVertexDeclaration(mGLHandle);
				mInputDeclaration = HardwareBufferManager::instance().createVertexDeclaration(elementList);
			}
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuProgram);
		GpuProgram::initialize();
	}

	bool GLSLGpuProgram::isSupported() const
	{
		if (!isRequiredCapabilitiesSupported())
			return false;

		RenderAPI* rapi = RenderAPI::instancePtr();
		return rapi->getCapabilities(0).isShaderProfileSupported("glsl");
	}
}}
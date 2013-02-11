/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "CmD3D9HLSLProgramFactory.h"
#include "CmString.h"
#include "CmD3D9HLSLProgram.h"

namespace CamelotEngine {
    //-----------------------------------------------------------------------
    String D3D9HLSLProgramFactory::sLanguageName = "hlsl";
    //-----------------------------------------------------------------------
    D3D9HLSLProgramFactory::D3D9HLSLProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    D3D9HLSLProgramFactory::~D3D9HLSLProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    const String& D3D9HLSLProgramFactory::getLanguage(void) const
    {
        return sLanguageName;
    }
    //-----------------------------------------------------------------------
	HighLevelGpuProgram* D3D9HLSLProgramFactory::create(const String& source, const String& entryPoint, 
		GpuProgramType gptype, GpuProgramProfile profile, const vector<GpuProgIncludePtr>::type* includes)
    {
		D3D9HLSLProgram* prog = new D3D9HLSLProgram(source, entryPoint, sLanguageName, gptype, profile, includes);

        return prog;
    }
	//-----------------------------------------------------------------------
	HighLevelGpuProgram* D3D9HLSLProgramFactory::create()
	{
		D3D9HLSLProgram* prog = new D3D9HLSLProgram("", "", sLanguageName, GPT_VERTEX_PROGRAM, GPP_NONE, nullptr);

		return prog;
	}
}

﻿#pragma once

#include "internal_compiler.h"

namespace ad_astris::rcore::impl
{
	class GLSLInternalCompiler : public IInternalCompiler
	{
		public:
			virtual void init(ShaderCompilerInitContext& initContext) override;
			virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) override;
	};
}
﻿#pragma once

#include "rendering_base.h"

namespace ad_astris::renderer::impl
{
	class SwapChainPass : public rcore::IRenderPassExecutor, public RenderingBase
	{
		public:
			SwapChainPass(RenderingInitContext& initContext);
			virtual void prepare_render_pass() override;
			virtual void execute(rhi::CommandBuffer* cmd) override;
	};
}
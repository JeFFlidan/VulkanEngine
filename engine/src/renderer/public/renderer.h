﻿#pragma once

#include "core/global_objects.h"
#include "engine_core/project_settings/project_settings.h"
#include "application_core/window.h"

namespace ad_astris::renderer
{
	struct RendererInitializationContext
	{
		ecore::ProjectSettings* projectSettings{ nullptr };
		acore::IWindow* mainWindow{ nullptr };
	};

	struct DrawContext
	{
		ecs::Entity activeCamera;
		float deltaTime;
	};
	
	class IRenderer
	{
		public:
			virtual ~IRenderer() { }
			
			virtual void init(RendererInitializationContext& initializationContext) = 0;
			virtual void cleanup() = 0;
			
			virtual rhi::RHI* get_rhi() = 0;

			virtual void bake() = 0;
			virtual void draw(DrawContext& drawContext) = 0;
	};
}
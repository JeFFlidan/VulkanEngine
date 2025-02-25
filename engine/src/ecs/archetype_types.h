#pragma once

#include "entity_types.h"
#include <cstdint>
#include <algorithm>
#include <functional>

namespace ad_astris::ecs
{
	class EntityManager;
	class Archetype;
	
	class ArchetypeHandle
	{
		friend class EntityManager;
		
		public:
			ArchetypeHandle() = default;
			uint32_t get_id()
			{
				return _id;
			}

		private:
			ArchetypeHandle(uint32_t id) : _id(id) {}
			uint32_t _id;
	};

	// Can be passed into EntityManager only once because all data will be moved 
	class ArchetypeCreationContext
	{
		friend Archetype;
		friend EntityManager;
		
		public:
			template<typename ...TYPES>
			void add_components()
			{
				((set_up_components<TYPES>()), ...);
				std::sort(_componentIDs.begin(), _componentIDs.end());
			}

			template<typename ...TYPES>
			void add_tags()
			{
				((set_up_tag<TYPES>(), ...));
				std::sort(_tagIDs.begin(), _tagIDs.end());
			}

			void set_entity_count(uint32_t entityCount)
			{
				_entityCount = entityCount;
			}
			
		protected:
			std::unordered_map<uint64_t, uint32_t> _sizeByComponentID;
			uint32_t _allComponentsSize{ 0 };
			uint32_t _entityCount{ 1024 };
			std::vector<uint64_t> _componentIDs;
			std::vector<uint64_t> _tagIDs;

			template<typename T>
			void set_up_components()
			{
				uint64_t id = TypeInfoTable::get_component_id<T>();
				uint32_t size = sizeof(T);
				_allComponentsSize += size;
				_sizeByComponentID[id] = size;
				_componentIDs.push_back(id);
			}

			template<typename T>
			void set_up_tag()
			{
				uint64_t id = TypeInfoTable::get_tag_id<T>();
				_tagIDs.push_back(id);
			}
	};

	class ArchetypeExtensionContext : public ArchetypeCreationContext
	{
		friend EntityManager;
		
		public:
			ArchetypeExtensionContext(ArchetypeHandle srcArchetype) : _srcArchetype(srcArchetype)
			{
				
			}

		protected:
			ArchetypeHandle _srcArchetype;
	};

	class ArchetypeReductionContext : public ArchetypeExtensionContext
	{
		public:
			ArchetypeReductionContext(ArchetypeHandle srcArchetype) : ArchetypeExtensionContext(srcArchetype)
			{
				
			}
	};
}

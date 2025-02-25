#pragma once

#include "attributes.h"
#include "engine_core/uuid.h"
#include "profiler/logger.h"
#include "type_info_table.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <mutex>

namespace ad_astris::ecs
{
	class Archetype;
	class EntityManager;
	class ArchetypeCreationContext;
	class ArchetypeExtensionContext;
	class EntityCreationContext;
	
	namespace constants
	{
		constexpr uint32_t MAX_COMPONENT_SIZE = 128;
		constexpr uint32_t MAX_COMPONENT_COUNT = 15;
		constexpr uint32_t MAX_CHUNK_SIZE = MAX_COMPONENT_SIZE * MAX_COMPONENT_COUNT;
	}
	
	class Entity
	{
		public:
			Entity();
			Entity(UUID uuid);
			UUID get_uuid();

			bool operator==(const Entity& other);
			bool operator!=(const Entity& other);
			operator uint64_t() const;

			template<typename Component>
			FORCE_INLINE bool has_component() const
			{
				return has_component_internal(TypeInfoTable::get_component_id<Component>());
			}

			template<typename Tag>
			FORCE_INLINE bool has_tag() const
			{
				return has_tag_internal(TypeInfoTable::get_tag_id<Tag>());
			}

			template<typename Property>
			FORCE_INLINE bool has_property() const
			{
				if constexpr (Reflector::has_attribute<Property, EcsComponent>())
				{
					return has_component<Property>();
				}
				if constexpr (Reflector::has_attribute<Property, EcsTag>())
				{
					return has_tag<Property>();
				}
				LOG_ERROR("Entity::has_property(): Entity properties are tags and components. However, {} is neither a tag nor a component.", get_type_name<Property>())
				return false;
			}

			template<typename Component>
			FORCE_INLINE Component* get_component()
			{
				return static_cast<Component*>(get_component_by_id(TypeInfoTable::get_component_id<Component>()));
			}

			template<typename Component>
			FORCE_INLINE const Component* get_component() const
			{
				return static_cast<const Component*>(get_component_by_id(TypeInfoTable::get_component_id<Component>()));
			}

			bool is_valid() const;
		
		private:
			UUID _uuid;

			bool has_component_internal(uint64_t componentID) const;
			bool has_tag_internal(uint64_t tagID) const;
			void* get_component_by_id(uint64_t componentID) const;
	};

	class IComponent
	{
		public:
			virtual ~IComponent() { }
		
			virtual uint64_t get_type_id() const = 0;
			virtual const void* get_raw_memory() const = 0;
			virtual uint32_t get_structure_size() const = 0;
	};

	class UntypedComponent : public IComponent
	{
		friend Archetype;
		friend EntityManager;
		friend ArchetypeCreationContext;
		friend ArchetypeExtensionContext;
		friend EntityCreationContext;
		
		public:
			UntypedComponent() = default;
			
			UntypedComponent(const void* memory, uint32_t size, uint64_t id) : _memory(memory), _size(size), _id(id)
			{
				
			}

			virtual ~UntypedComponent() override { }

			// ====== Begin IComponent interface ======
			
			virtual uint64_t get_type_id() const override
			{
				return _id;
			}
			
			virtual const void* get_raw_memory() const override
			{
				return _memory;
			}
			
			virtual uint32_t get_structure_size() const override
			{
				return _size;
			}

			// ====== End IComponent interface ======

		protected:
			const void* _memory{ nullptr };
			uint32_t _size{ 0 };
			uint64_t _id{ 0 };

			// Need this for serialization. I can clear data using destructor but I want to do it explicitly
			virtual void destroy_component_value() { } 
	};

	template<typename T>
	class Component : public IComponent
	{
		public:
			Component() = default;
			Component(T& component) : _component(component) { }
		
			virtual uint64_t get_type_id() const override { return TypeInfoTable::get_component_id<T>(); }
			virtual const void* get_raw_memory() const override{ return &_component; }
			virtual uint32_t get_structure_size() const override { return sizeof(T); }

		private:
			T _component;
	};
	
	class EntityCreationContext
	{
		friend Archetype;
		friend EntityManager;
		
		public:
			EntityCreationContext()
			{
				_componentIDs.reserve(constants::MAX_COMPONENT_COUNT);
			}
		
			~EntityCreationContext()
			{
				
			}

			template<typename T>
			void add_tag()
			{
				uint64_t id = TypeInfoTable::get_tag_id<T>();
				if (!check_tag(id))
					return;

				_tagIDs.push_back(id);
			}

			void add_tag(uint64_t tagID)
			{
				if (!check_tag(tagID))
					return;
				_tagIDs.push_back(tagID);
			}
		
			template<typename T>
			void add_component(T& value)
			{
				uint64_t typeId = TypeInfoTable::get_component_id<T>();
				
				if (!check_component(typeId, sizeof(T)))
					return;
				
				_allComponentsSize += sizeof(T);
				std::unique_ptr<IComponent> component(new Component<T>(value));
				_sizeByTypeID[typeId] = component->get_structure_size();
				_componentsMap[typeId] = std::move(component);
				_componentIDs.push_back(typeId);
			}

			template<typename T, typename ...ARGS>
			void add_component(ARGS&&... args)
			{
				uint64_t typeId = TypeInfoTable::get_component_id<T>();
				
				if (!check_component(typeId, sizeof(T)))
					return;
				
				_allComponentsSize += sizeof(T);
				
				std::unique_ptr<IComponent> component(new Component<T>( T{std::forward<ARGS>(args)...} ));
				_sizeByTypeID[typeId] = component->get_structure_size();
				_componentsMap[typeId] = std::move(component);
				_componentIDs.push_back(typeId);
			}

			template<typename ...ARGS>
			void add_components(ARGS&&... args)
			{
				((add_component(args)), ...);
			}

			void add_component(uint64_t componentID, uint32_t componentSize, const void* componentData)
			{
				if (!check_component(componentID, componentSize))
					return;

				_allComponentsSize += componentSize;

				_sizeByTypeID[componentID] = componentSize;
				_componentsMap[componentID] = std::make_unique<UntypedComponent>(componentData, componentSize, componentID);
				_componentIDs.push_back(componentID);
			}

			template<typename T>
			T get_component()
			{
				auto it = _componentsMap.find(TypeInfoTable::get_component_id<T>());
				if (it == _componentsMap.end())
					LOG_ERROR("EntityCreationContext::get_component(): Creation context doesn't contain component {}", get_type_name<T>())
				return *static_cast<const T*>(it->second->get_raw_memory());
			}

			template<typename T>
			void set_component(T& value)
			{
				uint64_t id = TypeInfoTable::get_component_id<T>();
				
				if (!check_component(id, sizeof(T)))
					return;

				IComponent* component = _componentsMap.find(id)->second.get();
				*component = Component<T>(value);
			}

			template<typename T, typename ...ARGS>
			void set_component(ARGS&&... args)
			{
				uint64_t id = TypeInfoTable::get_component_id<T>();
				
				if (!check_component(id, sizeof(T)))
					return;

				//Component<T>* component = dynamic_cast<Component<T*>>(_componentsMap.find(id))->second;
				IComponent* component = _componentsMap.find(id)->second.get();
				*component = Component<T>(T{std::forward<ARGS>(args)...});
			}

			template<typename ...ARGS>
			void set_components(ARGS&&... args)
			{
				((set_component(args)), ...);
			}

			template<typename T>
			void remove_component()
			{
				uint64_t typeID = TypeInfoTable::get_component_id<T>();
				
				if (!is_component_added(typeID))
				{
					return;
				}
			
				auto it = _componentsMap.find(typeID);
				_componentIDs.erase(std::find(_componentIDs.begin(), _componentIDs.end(), typeID));
				_componentsMap.erase(it);
			}

			template<typename ...ARGS>
			void remove_components()
			{
				((remove_component<ARGS>()), ...);
			}

			bool is_component_added(uint64_t componentTypeId)
			{
				if (_componentsMap.find(componentTypeId) != _componentsMap.end())
					return true;
				LOG_ERROR("EntityCreationContext::remove_component(): No component with this type")
				return false;
			}

		private:
			uint32_t _allComponentsSize{ 0 };
			std::vector<uint64_t> _componentIDs;
			std::unordered_map<uint64_t, std::unique_ptr<IComponent>> _componentsMap;
			std::unordered_map<uint64_t, uint32_t> _sizeByTypeID;
			std::vector<uint64_t> _tagIDs;

			bool check_component(uint64_t id, uint32_t size)
			{
				if (_componentsMap.find(id) != _componentsMap.end())
				{
					LOG_ERROR("EntityCreationContext::add_component(): Component of this type was added")
					return false;
				}

				return true;
			}

			bool check_tag(uint64_t id)
			{
				if (std::find(_tagIDs.begin(), _tagIDs.end(), id) != _tagIDs.end())
				{
					LOG_ERROR("EntityCreationContext::add_tag(): Tag of this type was added")
					return false;
				}

				return true;
			}
	};
}

namespace std
{
	template<>
	struct hash<ad_astris::ecs::Entity>
	{
		std::size_t operator()(const ad_astris::ecs::Entity& entity) const
		{
			return hash<uint64_t>()((uint64_t)entity);
		}
	};
}


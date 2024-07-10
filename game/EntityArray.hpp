#pragma once

#include <RefOptional.hpp>
#include <Types.hpp>
#include <Assertions.hpp>
#include <vector>

// New entity version can only be taken once per frame, because the entites are removed and added at the end of the frame. So to overflow a 32 bit version number running at 60 fps it would take 828 days. So this is how long an entity would need to be kept for between frames for the entity array to think that an old entity is a new entity.

// I took these types out so I don't have to write all the template parameters of entity array.

template<typename T>
struct EntityArrayId {
	bool operator==(const EntityArrayId&) const = default;
	i32 index() const;
	i32 version() const;

	static EntityArrayId invalid();

	u32 index_;
	u32 version_;

	EntityArrayId(u32 index, u32 version) : index_{ index }, version_{ version } {}
};

// TODO: Maybe make iterator and pair the same thing. They essentially do the same thing. One issues is that you wouldn't be able to do structured bindings on the object, because it contains array (which you wouldn't want). It uses the index so you would need to call a function to get the id. It also can't store a reference to the entity only a pointer because it needs to modify it. Technically you could reinterpret the reference as a pointer.
template<typename Entity>
struct EntityArrayPair {
	EntityArrayId<Entity> id;
	Entity& entity;
	auto operator->() -> Entity*;
	auto operator->() const -> const Entity*;
};

template<typename Entity, typename DefaultInitialize>
struct EntityArray {
	static EntityArray make();

	void update();
	std::optional<Entity&> get(const EntityArrayId<Entity>& id);
	std::optional<Entity&> getEvenIfInactive(const EntityArrayId<Entity>& id);
	bool isAlive(const EntityArrayId<Entity>& id);
	//bool isAlive(const Entity& entity);
	EntityArrayPair<Entity> create();
	void destroy(const EntityArrayId<Entity>& id);
	void deactivate(const EntityArrayId<Entity>& id);
	void activate(const EntityArrayId<Entity>& id);
	//void destroy(const Entity& entity);

	//std::optional<Id> validate(u32 index);
	void reset();
	i64 aliveCount() { return aliveCount_; };

	struct Iterator {
		auto operator++() -> Iterator&;
		auto operator!=(const Iterator& other) const -> bool;
		auto operator->() -> Entity*;
		auto operator->() const -> const Entity*;
		auto operator*() -> EntityArrayPair<Entity>;

		u32 index;
		EntityArray& array;
	};

	auto begin() -> Iterator;
	auto end() -> Iterator;

	std::vector<Entity> entities;
	std::vector<u32> entityVersions;
	// Could use a set instead of entityIsFree and freeEntities, but it would probably be slower. Don't want to do a red-black tree search for each iterator increment.
	std::vector<bool> entityIsFree;
	std::vector<bool> entityIsActive;

	EntityArray() = default;

	i64 aliveCount_ = 0;

	std::vector<u32> freeEntities;
	// To make pooling more efficient could make a templated function that wouold reset the state of an object. So for example if the type stored a vector it would just clear it instead of calling the constructor, which would cause a reallocation.

	// Delaying entity removal till the end of the frame so systems like for example the collision system can assume that the entites it received as added last frame are still alive at the end of the frame.
	// First add entites the remove so if the entites that was created in one frame and deleted in the same frame gets deleted. This shouldn't even be able to happen, because the entity adding is also buffered till the end of the frame.

	std::vector<EntityArrayId<Entity>> entitiesToRemove;
	std::vector<EntityArrayId<Entity>> entitiesAddedLastFrame_;
	std::vector<EntityArrayId<Entity>> entitiesAddedThisFrame;
	// Could delay the creating of entites until the end of frame. One advantage of doing this is that you can loop over entities and add new ones. The entites would still be created inside the entites list, but the versions would be updated (this also requires the version zero to always be an invalid version, could also make sure it wraps around to 1 on overflow). The created entity ids would be added to at toAdd list, which would be iterated in the update function and the versions would be updated there. Would need to make sure that there aren't any issues if an entity was destroyed on the same frame it was created. Could either first add entites the destroy them or remove all the entites, which are both inside the add and remove list. !!! This wouldn't actually work, when adding an entity pointers could get invalidated so you would need to only allow iterating over indices and only allow access using indices, could make a class that just stores the index and on operator -> gives access to the entity or could just save them into a separate vector, but if I wanted to implmenet the pooling of more complex types so types that store for exapmle vector don't need to get reallocated then this pooling would also need to work for this list.
public:
	auto entitiesAddedLastFrame() const -> const std::vector<EntityArrayId<Entity>>& { return entitiesAddedLastFrame_; }
};

template<typename T>
i32 EntityArrayId<T>::index() const {
	return index_;
}

template<typename T>
i32 EntityArrayId<T>::version() const {
	return version_;
}

template<typename T>
EntityArrayId<T> EntityArrayId<T>::invalid() {
	return EntityArrayId(0xCDCDCDCD, 0xCDCDCDCD);
}

template<typename Entity, typename DefaultInitialize>
EntityArray<Entity, DefaultInitialize> EntityArray<Entity, DefaultInitialize>::make() {
	return EntityArray();
}

template<typename Entity, typename DefaultInitialize>
void EntityArray<Entity, DefaultInitialize>::update() {
	ASSERT(entities.size() == entityVersions.size());
	ASSERT(entityVersions.size() == entityIsFree.size());
	
	std::swap(entitiesAddedThisFrame, entitiesAddedLastFrame_);
	entitiesAddedThisFrame.clear();
	
	for (const auto id : entitiesToRemove) {
		if (id.index_ >= entities.size()) {
			CHECK_NOT_REACHED();
			return;
		}
	
		if (id.version_ != entityVersions[id.index_]) {
			// Should double free be an error?
			CHECK_NOT_REACHED();
			return;
		}
	
		freeEntities.push_back(id.index_);
		entityVersions[id.index_]++;
		entityIsFree[id.index_] = true;
		aliveCount_--;
	}
	entitiesToRemove.clear();
}

template<typename Entity, typename DefaultInitialize>
std::optional<Entity&> EntityArray<Entity, DefaultInitialize>::get(const EntityArrayId<Entity>& id) {
	const auto result = getEvenIfInactive(id);

	if (!result.has_value()) {
		return std::nullopt;
	}

	if (!entityIsActive[id.index_]) {
		return std::nullopt;
	}

	return result;
}

template<typename Entity, typename DefaultInitialize>
std::optional<Entity&> EntityArray<Entity, DefaultInitialize>::getEvenIfInactive(const EntityArrayId<Entity>& id) {
	if (id.index_ >= entities.size()) {
		ASSERT_NOT_REACHED();
		return std::nullopt;
	}

	if (id.version_ != entityVersions[id.index_]) {
		return std::nullopt;
	}
	return entities[id.index_];
}

template<typename Entity, typename DefaultInitialize>
bool EntityArray<Entity, DefaultInitialize>::isAlive(const EntityArrayId<Entity>& id) {
	return get(id).has_value();
}

template<typename Entity, typename DefaultInitialize>
EntityArrayPair<Entity> EntityArray<Entity, DefaultInitialize>::create() {
	EntityArrayId<Entity> id(-1, -1);
	if (freeEntities.size() == 0) {
		id = EntityArrayId<Entity>(static_cast<u32>(entities.size()), 0);
		entities.emplace_back(DefaultInitialize()());
		entityVersions.push_back(0);
		entityIsFree.push_back(false);
		entityIsActive.push_back(true);
	} else {
		const auto index = freeEntities.back();
		freeEntities.pop_back();
		entityIsFree[index] = false;
		entityIsActive[index] = true;
		id = EntityArrayId<Entity>(index, entityVersions[index]);
	}

	aliveCount_++;
	entitiesAddedThisFrame.push_back(id);
	return { id, entities[id.index_] };
}

template<typename Entity, typename DefaultInitialize>
void EntityArray<Entity, DefaultInitialize>::destroy(const EntityArrayId<Entity>& id) {
	entitiesToRemove.push_back(id);
}

template<typename Entity, typename DefaultInitialize>
void EntityArray<Entity, DefaultInitialize>::deactivate(const EntityArrayId<Entity>& id) {
	if (!get(id).has_value()) {
		return;
	}
	entityIsActive[id.index_] = false;
}

template<typename Entity, typename DefaultInitialize>
void EntityArray<Entity, DefaultInitialize>::activate(const EntityArrayId<Entity>& id) {
	if (!getEvenIfInactive(id).has_value()) {
		return;
	}
	entityIsActive[id.index_] = true;
}

template<typename Entity, typename DefaultInitialize>
void EntityArray<Entity, DefaultInitialize>::reset() {
	freeEntities.clear();
	// Iterating backwards so the ids are given out in order, which is required for level loading.
	for (i32 i = static_cast<i32>(entities.size()) - 1; i >= 0; i--) {
		freeEntities.push_back(i);
		entityIsFree[i] = true;
	}
	entitiesToRemove.clear();
	entitiesAddedLastFrame_.clear();
	entitiesAddedThisFrame.clear();
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::begin() -> Iterator {
	u32 index = 0;
	while (index < entityIsFree.size() && (entityIsFree[index] || (!entityIsFree[index] && !entityIsActive[index]))) {
		index++;
	}
	return Iterator{ index, *this };
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::end() -> Iterator {
	return Iterator{ static_cast<u32>(entities.size()), *this };
}


template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::Iterator::operator++() -> Iterator& {
	if (*this != array.end()) {
		index++;
	}
	while (*this != array.end() && (array.entityIsFree[index] || (!array.entityIsFree[index] && !array.entityIsActive[index]))) {
		index++;
	}
	
	return *this;
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::Iterator::operator!=(const Iterator& other) const -> bool {
	ASSERT(&array == &other.array);
	return index != other.index;
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::Iterator::operator->() -> Entity* {
	return &array.entities[index];	
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::Iterator::operator->() const -> const Entity* {
	return &array.entities[index];
}

template<typename Entity, typename DefaultInitialize>
auto EntityArray<Entity, DefaultInitialize>::Iterator::operator*() -> EntityArrayPair<Entity> {
	return EntityArrayPair<Entity>(EntityArrayId<Entity>{ index, array.entityVersions[index] }, array.entities[index]);
}

template<typename Entity>
auto EntityArrayPair<Entity>::operator->() -> Entity* {
	return &entity;
}

template<typename Entity>
auto EntityArrayPair<Entity>::operator->() const -> const Entity* {
	return &entity;
}

#pragma once

#include <game/EditorEntities.hpp>
#include <game/StackAllocator.hpp>
#include <unordered_set>

struct EditorActionCreateEntity {
	EditorActionCreateEntity(EditorEntityId id);

	EditorEntityId id;
};

struct EditorActionDestroyEntity {
	EditorActionDestroyEntity(EditorEntityId id);

	EditorEntityId id;
};

struct EditorActionSelectionChange {
	EditorActionSelectionChange(View<EditorEntityId> oldSelection, View<EditorEntityId> newSelection);

	View<EditorEntityId> oldSelection;
	View<EditorEntityId> newSelection;
};

// Should this store a copy of the reflecting body or should there be a new entity create and this would just store the ids.
struct EditorActionModifyReflectingBody {
	EditorActionModifyReflectingBody(EditorRigidBodyId id, const EditorRigidBody& oldEntity, const EditorRigidBody& newEntity);

	// Storing the memory inside instead of allocating it somewhere else (for example on the stack), because it adds an pointless indirection. Using a specific type instead of a generic thing, because I don't thinkg it would simplify anything. The entity cases would just need to be handled somewhere else.
	EditorRigidBodyId id;
	EditorRigidBody oldEntity; 
	EditorRigidBody newEntity;
};

enum class EditorActionType {
	CREATE_ENTITY,
	DESTROY_ENTITY,
	SELECTION_CHANGE,
	MODIFY_REFLECTING_BODY,
};

// TODO: Because this is all on a stack I could just allocate this direclty on the stack without using a union which takes up more space. Using this I could also use a single allocation on the stack for the whole object. For example selection change would allocate it's data in a single allocation.
// The pointers would be stored in Editor::Action
struct EditorAction {
	union {
		EditorActionCreateEntity createEntity;
		EditorActionDestroyEntity destoryEntity;
		EditorActionSelectionChange selectionChange;
		EditorActionModifyReflectingBody modifyReflectingBody;
	};
	explicit EditorAction(const EditorActionCreateEntity& action);
	explicit EditorAction(const EditorActionDestroyEntity& action);
	explicit EditorAction(const EditorActionSelectionChange& action);
	explicit EditorAction(const EditorActionModifyReflectingBody& action);

	EditorActionType type;
};

struct Editor;

struct EditorActions {
	static EditorActions make();

	List<EditorAction> actionStack;
	// Multiactions can contain multiple actions.
	struct Action {
		Action(i64 subActionCount);
		i64 subActionCount;
	};
	List<Action> actions;
	// When every action is undone lastDoneAction = -1.
	i64 lastDoneAction = -1;

	i64 actionIndexToStackStartOffset(i64 actionIndex);

	// returns if a multicommand actually started.
	bool beginMulticommand();
	void endMulticommand(bool started);

	void add(Editor& editor, EditorAction&& action) noexcept;
	void addSelectionChange(Editor& editor, const std::unordered_set<EditorEntityId>& oldSelection, const std::unordered_set<EditorEntityId>& newSelection);
	void freeSelectionChange(EditorActionSelectionChange& action);

	bool recordingMultiAction = false;
	i64 currentMultiActionSize = 0;

	StackAllocator stackAllocator;
};
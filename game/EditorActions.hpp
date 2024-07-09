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

enum class EditorActionType {
	CREATE_ENTITY,
	DESTROY_ENTITY,
	SELECTION_CHANGE,
};

struct EditorAction {
	union {
		EditorActionCreateEntity createEntity;
		EditorActionDestroyEntity destoryEntity;
		EditorActionSelectionChange selectionChange;
	};
	explicit EditorAction(const EditorActionCreateEntity& action);
	explicit EditorAction(const EditorActionDestroyEntity& action);
	explicit EditorAction(const EditorActionSelectionChange& action);

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

	void beginMulticommand();
	void endMulticommand();

	void add(Editor& editor, EditorAction&& action) noexcept;
	void addSelectionChange(Editor& editor, const std::unordered_set<EditorEntityId>& oldSelection, const std::unordered_set<EditorEntityId>& newSelection);
	void freeSelectionChange(EditorActionSelectionChange& action);

	bool recordingMultiAction = false;
	i64 currentMultiActionSize = 0;

	StackAllocator stackAllocator;
};
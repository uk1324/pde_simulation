#pragma once

#include <game/EditorEntities.hpp>

struct EditorActionCreateEntity {
	EditorActionCreateEntity(EditorEntityId id);

	EditorEntityId id;
};

enum class EditorActionType {
	CREATE_ENTITY
};

struct EditorAction {
	union {
		EditorActionCreateEntity createEntity;
	};
	EditorAction(const EditorActionCreateEntity& action);

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

	bool recordingMultiAction = false;
	i64 currentMultiActionSize = 0;
};
#include <game/EditorActions.hpp>
#include <game/Editor.hpp>

EditorActions EditorActions::make() {
    return EditorActions{
        .actionStack = List<EditorAction>::empty(),
        .actions = List<Action>::empty()
    };
}

i64 EditorActions::actionIndexToStackStartOffset(i64 actionIndex) {
    // TODO: This would be already be computed if the offests into the stack were stored instead of sizes. The sizes can be compuated by taking the difference between to positions.
    i64 offset = 0;
    for (usize i = 0; i < actionIndex; i++) {
        offset += actions[i].subActionCount;
    }
    return offset;
}

void EditorActions::beginMulticommand() {
    ASSERT(!recordingMultiAction);
    recordingMultiAction = true;
    currentMultiActionSize = 0;
}

void EditorActions::endMulticommand() {
    ASSERT(recordingMultiAction);
    // Not asserting that the multicommand isn't empty because it allows writing easier code when adding commands in a loop over a range which can be empty.
    recordingMultiAction = false;
    if (currentMultiActionSize != 0) {
        actions.add(Action{ currentMultiActionSize });
        lastDoneAction++;
    }
}

void EditorActions::add(Editor& editor, EditorAction&& action) noexcept {
    if (lastDoneAction != actions.size() - 1) {
        const auto toFree = actions.size() - 1 - lastDoneAction;
        for (i64 i = 0; i < toFree; i++) {
            const auto& action = actions.back();

            for (i64 j = 0; j < action.subActionCount; j++) {
                auto& actionToDelete = actionStack.back();
                editor.destoryAction(actionToDelete);
                actionStack.pop();
            }

            actions.pop();
        }
    }
    if (recordingMultiAction) {
        currentMultiActionSize++;
    } else {
        lastDoneAction++;
        actions.add(Action(1));
    }
    actionStack.add(std::move(action));
}

EditorActions::Action::Action(i64 subActionCount)
    : subActionCount(subActionCount) {}

EditorAction::EditorAction(const EditorActionCreateEntity& action)
    : createEntity(action)
    , type(EditorActionType::CREATE_ENTITY) {}

EditorActionCreateEntity::EditorActionCreateEntity(EditorEntityId id)
    : id(id) {}
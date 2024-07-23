#pragma once

#include <optional>
#include <string_view>

namespace Gui {
	static constexpr auto MIN_FILE_SELECT_STRING_LENGTH = 256;
	auto openFileSelect() -> std::optional<std::string_view>;
	auto openFileSave() -> std::optional<std::string_view>;
}

#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <PreprocessIncludes.hpp>

using namespace std;
using namespace filesystem;

void outputCamelCaseToUpperSnakeCase(std::ostream& os, std::string_view v) {
	auto previousCharIsDigit = false;
	for (int i = 0; i < v.length(); i++) {
		const auto c = v[i];

		auto isAfterSeparator = false;

		if ((isupper(c) || (isdigit(c) && !previousCharIsDigit)) && i != 0) {
			os << '_';
			isAfterSeparator = true;
		}

		os << static_cast<char>(toupper(c));

		previousCharIsDigit = isdigit(c);
	}
}

bool isWhitespace(char c) {
	switch (c)
	{
	case '\t':
	case '\r':
	case '\f':
	case '\n':
		return true;
	}

	return false;
}

void minifyShader(std::ostream& os, std::string_view source) {
	int i = 0;
	auto isAtEnd = [&]() {
		return i >= source.size();
	};

	auto peek = [&]() {
		if (isAtEnd()) {
			return '\0';
		}
		return source[i];
	};

	auto peekNext = [&]() {
		if (i + 1 >= source.size()) {
			return '\0';
		}
		return source[i + 1];
	};

	std::optional<char> lastOutputChar;
	auto output = [&](char c) {
		if (c == '\r') {
			return;
		}

		lastOutputChar = c;
		// Not using raw string literals, because they might not work with different line endings ('\n' vs '\r\n')

		if (c == '\n') {
			os << "\\n";
		} else {
			os << c;
		}
	};

	while (!isAtEnd()) {
		if (peek() == ' ' && !((lastOutputChar.has_value() && isalnum(*lastOutputChar)) && isalnum(peekNext()))) {
			i++;
			continue;
		}
		if (isWhitespace(peek())) {
			i++;
			continue;
		}

		if (peek() == '#') {
			if (lastOutputChar.has_value() && lastOutputChar != '\n') {
				output('\n');
			}
			output(peek());
			i++;
			while (!isAtEnd()) {
				char c = peek();
				i++;
				if (c == '\n') {
					output(c);
					break;
				}
				output(c);
			}
			continue;
		}

		if (peek() == '/' && peekNext() == '/') {
			while (!isAtEnd()) {
				char c = peek();
				i++;
				if (c == '\n') {
					break;
				}
			}
			continue;
		}

		if (peek() == '/' && peekNext() == '*') {
			while (!isAtEnd()) {
				char c = peek();
				if (c == '*' && peekNext() == '/') {
					i += 2;
					break;
				}
				i++;
			}
			continue;
		}

		if (isAtEnd()) {
			break;
		}
		output(peek());
		i++;
	}
}

int main() {
	ofstream output("./generated/shaderSources.hpp");

	auto recursiveProcessDirectory = [&](std::string_view path) {
		for (const auto& file : recursive_directory_iterator(path)) {
			const auto& path = file.path();
			const auto extension = path.extension();
			const auto name = path.stem();
			const auto isVert = extension == ".vert";
			const auto isFrag = extension == ".frag";
			if (isVert || isFrag) {
				// Don't know why cmake prings newlines. They aren't output to the console.
				cout << "adding " << path.filename().string() << '\n';
				output << "static const char* ";
				outputCamelCaseToUpperSnakeCase(output, name.string());
				output << "_SHADER_";
				if (isVert) {
					output << "VERT";
				} else if (isFrag) {
					output << "FRAG";
				}

				const auto result = preprocessIncludes(path.string());
				if (!result.has_value()) {
					cerr << result.error();
					return EXIT_FAILURE;
				}
				const auto& source = *result;

				output << "_SOURCE = \"";
				minifyShader(output, source);
				output << "\";\n";
			}
		}
	};
	try {
		recursiveProcessDirectory("./game/Shaders/");
		recursiveProcessDirectory("./engine/gfx2d/Shaders/");
	} catch (const std::filesystem::filesystem_error& error) {
		std::cout << error.what() << '\n';
	}

}
#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "core/math.h"

namespace D {

using ParamValue = std::variant<float, int, bool>;
using ParamList = std::vector<ParamValue>;

struct Symbol {
  char letter;
  ParamList params;

  explicit Symbol(char c) : letter(c) {}
  Symbol(char c, ParamList p) : letter(c), params(std::move(p)) {}
};

using Word = std::vector<Symbol>;

// Build a Word from a plain string (each char -> Symbol with no params).
inline Word w(std::string_view s) {
  Word word;
  word.reserve(s.size());
  for (char c : s) word.emplace_back(c);
  return word;
}

// Extract just the letters of a Word as a std::string.
inline std::string str(const Word& word) {
  std::string s;
  s.reserve(word.size());
  for (const auto& sym : word) s += sym.letter;
  return s;
}

}  // namespace D

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "algorithm/IPlantAlgorithm.h"
#include "core/types.h"

namespace D {

// ── Expression evaluator (float arithmetic over named params) ─────────────────

namespace detail {

inline float toFloat(const ParamValue& v) {
  return std::visit([](auto x) { return static_cast<float>(x); }, v);
}

struct ExprEval {
  std::string_view src;
  size_t pos = 0;
  const std::map<std::string, float>& env;

  void skip() { while (pos < src.size() && src[pos] == ' ') ++pos; }
  char peek() { skip(); return pos < src.size() ? src[pos] : '\0'; }
  char consume() { skip(); return pos < src.size() ? src[pos++] : '\0'; }

  float expr()   { return add(); }
  float add() {
    float v = mul();
    while (peek() == '+' || peek() == '-') {
      char op = consume();
      v = (op == '+') ? v + mul() : v - mul();
    }
    return v;
  }
  float mul() {
    float v = unary();
    while (peek() == '*' || peek() == '/') {
      char op = consume();
      float r = unary();
      v = (op == '*') ? v * r : v / r;
    }
    return v;
  }
  float unary() {
    if (peek() == '-') { consume(); return -primary(); }
    return primary();
  }
  float primary() {
    char c = peek();
    if (c == '(') {
      consume();
      float v = expr();
      consume(); // ')'
      return v;
    }
    if (std::isdigit(c) || c == '.') {
      size_t start = pos;
      while (pos < src.size() && (std::isdigit(src[pos]) || src[pos] == '.'))
        ++pos;
      float v = 0.f;
      std::string s(src.substr(start, pos - start));
      try { v = std::stof(s); } catch (...) {}
      return v;
    }
    if (std::isalpha(c)) {
      std::string name;
      while (pos < src.size() && std::isalpha(src[pos])) name += src[pos++];
      auto it = env.find(name);
      return it != env.end() ? it->second : 0.f;
    }
    return 0.f;
  }
};

// Parse "F(s)[+A(s*0.7)][-A(s*0.7)]" into a Word, evaluating exprs with env.
inline Word parseParametricWord(std::string_view s,
                                const std::map<std::string, float>& env) {
  Word result;
  size_t i = 0;
  while (i < s.size()) {
    if (s[i] == ' ') { ++i; continue; }
    char letter = s[i++];
    ParamList params;
    if (i < s.size() && s[i] == '(') {
      ++i; // '('
      while (i < s.size() && s[i] != ')') {
        size_t depth = 0, start = i;
        while (i < s.size()) {
          if      (s[i] == '(')                    ++depth;
          else if (s[i] == ')' && depth == 0)      break;
          else if (s[i] == ')' && depth > 0)       --depth;
          else if (s[i] == ',' && depth == 0)      break;
          ++i;
        }
        ExprEval ev{s.substr(start, i - start), 0, env};
        params.push_back(ev.expr());
        if (i < s.size() && s[i] == ',') ++i;
      }
      if (i < s.size()) ++i; // ')'
    }
    result.emplace_back(letter, std::move(params));
  }
  return result;
}

} // namespace detail

// ── ParametricLSystemAlgorithm ────────────────────────────────────────────────

class ParametricLSystemAlgorithm : public IPlantAlgorithm {
 public:
  struct PRule {
    char predecessor;
    std::vector<std::string> paramNames; // e.g. {"s"} for A(s)->...
    std::string successorExpr;           // e.g. "F(s)[+A(s*0.7)][-A(s*0.7)]"
  };

  ParametricLSystemAlgorithm(Word axiom, std::vector<PRule> rules,
                              float angle = 25.f)
      : m_axiom(std::move(axiom)),
        m_rules(std::move(rules)),
        m_angle(angle),
        m_current(m_axiom) {}

  ParametricLSystemAlgorithm(const ParametricLSystemAlgorithm& o)
      : m_axiom(o.m_axiom), m_rules(o.m_rules), m_angle(o.m_angle),
        m_current(m_axiom), m_generation(0) {}

  ParametricLSystemAlgorithm(ParametricLSystemAlgorithm&&) = default;

  void step() override {
    m_current = derive(m_current);
    ++m_generation;
  }

  void reset() override {
    m_current = m_axiom;
    m_generation = 0;
  }

  int generation() const override { return m_generation; }
  const Word& current() const override { return m_current; }

  float angle() const { return m_angle; }
  const Word& axiomWord() const { return m_axiom; }
  const std::vector<PRule>& prules() const { return m_rules; }
  const std::map<std::string, float>& globalParams() const { return m_globalParams; }

  void setGlobalParams(std::map<std::string, float> params) {
    m_globalParams = std::move(params);
    m_current = m_axiom;
    for (int i = 0; i < m_generation; ++i)
      m_current = derive(m_current);
  }

 private:
  Word                          m_axiom;
  std::vector<PRule>            m_rules;
  float                         m_angle;
  Word                          m_current;
  int                           m_generation = 0;
  std::map<std::string, float>  m_globalParams;

  Word derive(const Word& current) const {
    Word result;
    result.reserve(current.size() * 4);
    for (const Symbol& sym : current) {
      bool applied = false;
      for (const PRule& rule : m_rules) {
        if (rule.predecessor != sym.letter) continue;
        std::map<std::string, float> env = m_globalParams;
        for (size_t k = 0; k < rule.paramNames.size() && k < sym.params.size(); ++k)
          env[rule.paramNames[k]] = detail::toFloat(sym.params[k]);
        Word produced = detail::parseParametricWord(rule.successorExpr, env);
        result.insert(result.end(), produced.begin(), produced.end());
        applied = true;
        break;
      }
      if (!applied) result.push_back(sym);
    }
    return result;
  }
};

} // namespace D

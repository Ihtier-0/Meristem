#pragma once

#include "algorithm/IPlantAlgorithm.h"
#include "grammar/LSystemGrammar.h"

namespace D {

class D0LSystemAlgorithm final : public IPlantAlgorithm {
 public:
  explicit D0LSystemAlgorithm(LSystemGrammar grammar)
      : m_grammar(std::move(grammar)), m_current(m_grammar.axiom) {}

  void step() override {
    m_current = m_grammar.derive(m_current);
    ++m_generation;
  }

  void reset() override {
    m_current = m_grammar.axiom;
    m_generation = 0;
  }

  int generation() const override { return m_generation; }

  const Word& current() const override { return m_current; }

  const LSystemGrammar& grammar() const { return m_grammar; }

 private:
  LSystemGrammar m_grammar;
  Word m_current;
  int m_generation = 0;
};

}  // namespace D

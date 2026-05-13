#pragma once

#include "algorithm/IPlantAlgorithm.h"
#include "grammar/LSystemGrammar.h"

namespace D {

class D0LSystemAlgorithm : public IPlantAlgorithm {
 public:
  explicit D0LSystemAlgorithm(LSystemGrammar grammar)
      : m_grammar(std::move(grammar)),
        m_structure(StringStructure{m_grammar.axiom}) {}

  void step() override {
    auto& ss = std::get<StringStructure>(m_structure);
    ss.derivation = m_grammar.derive(ss.derivation);
    ++m_generation;
  }

  void reset() override {
    std::get<StringStructure>(m_structure).derivation = m_grammar.axiom;
    m_generation = 0;
  }

  int generation() const override { return m_generation; }

  const PlantStructure& getStructure() const override { return m_structure; }

  const LSystemGrammar& grammar() const { return m_grammar; }

 private:
  LSystemGrammar m_grammar;
  PlantStructure m_structure;
  int m_generation = 0;
};

}  // namespace D

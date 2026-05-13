#pragma once

#include <cstdint>
#include <random>

#include "algorithm/IPlantAlgorithm.h"
#include "grammar/LSystemGrammar.h"

namespace D {

class StochasticLSystemAlgorithm : public IPlantAlgorithm {
 public:
  explicit StochasticLSystemAlgorithm(LSystemGrammar grammar, uint32_t seed = 42)
      : m_grammar(std::move(grammar)),
        m_seed(seed),
        m_rng(seed),
        m_structure(StringStructure{m_grammar.axiom}) {}

  void step() override {
    auto& ss = std::get<StringStructure>(m_structure);
    ss.derivation = m_grammar.deriveStochastic(ss.derivation, m_rng);
    ++m_generation;
  }

  void reset() override {
    std::get<StringStructure>(m_structure).derivation = m_grammar.axiom;
    m_generation = 0;
    m_rng.seed(m_seed);
  }

  int generation() const override { return m_generation; }

  const PlantStructure& getStructure() const override { return m_structure; }

  const LSystemGrammar& grammar() const { return m_grammar; }

  uint32_t seed() const { return m_seed; }
  void setSeed(uint32_t seed) {
    m_seed = seed;
    // Re-derive from axiom for the same number of generations with the new seed
    m_rng.seed(m_seed);
    auto& ss = std::get<StringStructure>(m_structure);
    ss.derivation = m_grammar.axiom;
    for (int i = 0; i < m_generation; ++i)
      ss.derivation = m_grammar.deriveStochastic(ss.derivation, m_rng);
  }

 private:
  LSystemGrammar m_grammar;
  uint32_t       m_seed;
  std::mt19937   m_rng;
  PlantStructure m_structure;
  int            m_generation = 0;
};

}  // namespace D

#pragma once

#include <cstdint>
#include <random>

#include "algorithm/IPlantAlgorithm.h"
#include "grammar/LSystemGrammar.h"

namespace D {

class StochasticLSystemAlgorithm : public IPlantAlgorithm {
 public:
  explicit StochasticLSystemAlgorithm(LSystemGrammar grammar, uint32_t seed = 42)
      : m_grammar(std::move(grammar)), m_seed(seed), m_rng(seed), m_current(m_grammar.axiom) {}

  void step() override {
    m_current = m_grammar.derive(m_current, m_rng);
    ++m_generation;
  }

  void reset() override {
    m_current = m_grammar.axiom;
    m_generation = 0;
    m_rng.seed(m_seed);
  }

  int generation() const override { return m_generation; }

  const Word& current() const override { return m_current; }

  const LSystemGrammar& grammar() const { return m_grammar; }

  uint32_t seed() const { return m_seed; }
  void setSeed(uint32_t seed) {
    m_seed = seed;
    // Re-derive from axiom for the same number of generations with the new seed
    m_rng.seed(m_seed);
    m_current = m_grammar.axiom;
    for (int i = 0; i < m_generation; ++i) m_current = m_grammar.derive(m_current, m_rng);
  }

 private:
  LSystemGrammar m_grammar;
  uint32_t m_seed;
  std::mt19937 m_rng;
  Word m_current;
  int m_generation = 0;
};

}  // namespace D

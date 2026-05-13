#include "grammar/LSystemGrammar.h"

namespace D {

Word LSystemGrammar::derive(const Word& current) const {
  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];
    bool applied = false;

    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;

      if (rule.leftContext &&
          (i == 0 || current[i - 1].letter != *rule.leftContext))
        continue;
      if (rule.rightContext &&
          (i + 1 >= current.size() || current[i + 1].letter != *rule.rightContext))
        continue;

      if (rule.condition && !rule.condition(sym.params)) continue;

      Word produced = rule.successor(sym.params);
      result.insert(result.end(), produced.begin(), produced.end());
      applied = true;
      break;
    }

    if (!applied) result.push_back(sym);
  }

  return result;
}

Word LSystemGrammar::deriveN(int n) const {
  Word current = axiom;
  for (int i = 0; i < n; ++i) current = derive(current);
  return current;
}

Word LSystemGrammar::deriveStochastic(const Word& current, std::mt19937& rng) const {
  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];

    // Collect candidate rules for this symbol
    std::vector<const Rule*> candidates;
    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;
      if (rule.leftContext &&
          (i == 0 || current[i - 1].letter != *rule.leftContext))
        continue;
      if (rule.rightContext &&
          (i + 1 >= current.size() || current[i + 1].letter != *rule.rightContext))
        continue;
      if (rule.condition && !rule.condition(sym.params)) continue;
      candidates.push_back(&rule);
    }

    if (candidates.empty()) {
      result.push_back(sym);
      continue;
    }

    // Pick by probability (weights = rule.probability)
    float total = 0.f;
    for (const Rule* r : candidates) total += r->probability;
    std::uniform_real_distribution<float> dist(0.f, total);
    float pick = dist(rng);

    const Rule* chosen = candidates.back();
    float acc = 0.f;
    for (const Rule* r : candidates) {
      acc += r->probability;
      if (pick < acc) { chosen = r; break; }
    }

    Word produced = chosen->successor(sym.params);
    result.insert(result.end(), produced.begin(), produced.end());
  }

  return result;
}

}  // namespace D

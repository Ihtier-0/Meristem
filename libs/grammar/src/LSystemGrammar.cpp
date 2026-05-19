#include "grammar/LSystemGrammar.h"

namespace D {

// Bracket-aware left context: skip +/-/| and [...] groups going backwards.
// In  F[+A][-A]  the biological parent of each A is F, not '+' or '-'.
static char leftBioContext(const Word& w, size_t i) {
  int j = static_cast<int>(i) - 1;
  while (j >= 0) {
    char c = w[static_cast<size_t>(j)].letter;
    if (c == '+' || c == '-' || c == '|') {
      --j;
    } else if (c == ']') {
      // skip the entire matching [...] group
      int depth = 1; --j;
      while (j >= 0 && depth > 0) {
        char cc = w[static_cast<size_t>(j)].letter;
        if      (cc == ']') ++depth;
        else if (cc == '[') --depth;
        --j;
      }
    } else if (c == '[') {
      --j; // branch open: step past it to the parent symbol
    } else {
      return c;
    }
  }
  return '\0';
}

Word LSystemGrammar::derive(const Word& current) const {
  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];
    bool applied = false;

    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;

      if (rule.leftContext &&
          leftBioContext(current, i) != *rule.leftContext)
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
          leftBioContext(current, i) != *rule.leftContext)
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

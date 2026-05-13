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

}  // namespace D

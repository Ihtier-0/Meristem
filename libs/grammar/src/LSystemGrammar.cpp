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
      int depth = 1;
      --j;
      while (j >= 0 && depth > 0) {
        char cc = w[static_cast<size_t>(j)].letter;
        if (cc == ']')
          ++depth;
        else if (cc == '[')
          --depth;
        --j;
      }
    } else if (c == '[') {
      --j;  // branch open: step past it to the parent symbol
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

      if (rule.leftContext && leftBioContext(current, i) != *rule.leftContext) continue;
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

Word LSystemGrammar::derive(const Word& current, std::mt19937& rng) const {
  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];

    // Collect candidate rules for this symbol
    std::vector<const Rule*> candidates;
    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;
      if (rule.leftContext && leftBioContext(current, i) != *rule.leftContext) continue;
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
      if (pick < acc) {
        chosen = r;
        break;
      }
    }

    Word produced = chosen->successor(sym.params);
    result.insert(result.end(), produced.begin(), produced.end());
  }

  return result;
}

}  // namespace D

// ── Unit tests ────────────────────────────────────────────────────────────────
// Compiled into grammar_tests (DOCTEST_CONFIG_DISABLE is set for the grammar library target).

#include <doctest/doctest.h>

namespace {

// Build a Word from a plain string (letters only, no params).
D::Word w(std::string_view s) {
  D::Word word;
  word.reserve(s.size());
  for (char c : s) word.emplace_back(c);
  return word;
}

// Extract just the letters of a Word as a std::string.
std::string str(const D::Word& word) {
  std::string s;
  s.reserve(word.size());
  for (const auto& sym : word) s += sym.letter;
  return s;
}

}  // namespace

using D::ruleFor;

// ── D0L deterministic derive ──────────────────────────────────────────────────

TEST_CASE("D0L: Lindenmayer algae  A→AB B→A") {
  // Classic example: A AB ABA ABAAB ABAABABA …
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("AB"), ruleFor('B').to("A")};

  D::Word cur = g.axiom;
  cur = g.derive(cur);
  CHECK(str(cur) == "AB");
  cur = g.derive(cur);
  CHECK(str(cur) == "ABA");
  cur = g.derive(cur);
  CHECK(str(cur) == "ABAAB");
  cur = g.derive(cur);
  CHECK(str(cur) == "ABAABABA");
}

TEST_CASE("D0L: symbol with no matching rule stays unchanged") {
  D::LSystemGrammar g;
  g.axiom = w("FAG");
  g.rules = {ruleFor('A').to("B")};  // F and G have no rules

  CHECK(str(g.derive(g.axiom)) == "FBG");
}

TEST_CASE("D0L: first matching rule wins, others ignored") {
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("B"), ruleFor('A').to("C")};  // second must never fire

  CHECK(str(g.derive(g.axiom)) == "B");
}

TEST_CASE("D0L: empty axiom derives to empty") {
  D::LSystemGrammar g;
  g.axiom = {};
  g.rules = {ruleFor('A').to("B")};

  CHECK(g.derive(g.axiom).empty());
}

// ── Context-sensitive rules ───────────────────────────────────────────────────

TEST_CASE("D0L: left context (bracket-aware)  F<A→B") {
  // Axiom  F[+A][-A]
  // Both A's have F as their biological left context (brackets/+/- are skipped).
  D::LSystemGrammar g;
  g.axiom = w("F[+A][-A]");
  g.rules = {ruleFor('A').withLeftContext('F').to("B"), ruleFor('A').to("A")};  // default: stays A

  CHECK(str(g.derive(g.axiom)) == "F[+B][-B]");
}

TEST_CASE("D0L: left context mismatch → default rule fires") {
  // Axiom  G[+A]  — left context of A is G, not F → context rule skipped
  D::LSystemGrammar g;
  g.axiom = w("G[+A]");
  g.rules = {ruleFor('A').withLeftContext('F').to("B"), ruleFor('A').to("X")};  // default

  CHECK(str(g.derive(g.axiom)) == "G[+X]");
}

TEST_CASE("D0L: right context  A>F→B") {
  D::LSystemGrammar g;
  g.axiom = w("AF");
  g.rules = {ruleFor('A').withRightContext('F').to("B"), ruleFor('A').to("A")};  // default

  CHECK(str(g.derive(g.axiom)) == "BF");
}

TEST_CASE("D0L: right context at end of string → default rule fires") {
  D::LSystemGrammar g;
  g.axiom = w("A");  // nothing to the right
  g.rules = {ruleFor('A').withRightContext('F').to("B"), ruleFor('A').to("X")};

  CHECK(str(g.derive(g.axiom)) == "X");
}

// ── Stochastic derive ─────────────────────────────────────────────────────────

TEST_CASE("Stochastic: single rule always fires regardless of RNG") {
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("B").withProbability(1.f)};

  std::mt19937 rng(0);
  for (int i = 0; i < 20; ++i) CHECK(str(g.derive(g.axiom, rng)) == "B");
}

TEST_CASE("Stochastic: same seed produces same output") {
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("B").withProbability(0.5f),
             ruleFor('A').to("C").withProbability(0.5f)};

  auto run = [&](uint32_t seed) {
    std::mt19937 rng(seed);
    D::Word cur = g.axiom;
    for (int i = 0; i < 10; ++i) cur = g.derive(cur, rng);
    return str(cur);
  };

  CHECK(run(42) == run(42));
  CHECK(run(123) == run(123));
}

TEST_CASE("Stochastic: different seeds may produce different outputs") {
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("B").withProbability(0.5f),
             ruleFor('A').to("C").withProbability(0.5f)};

  auto run = [&](uint32_t seed) {
    std::mt19937 rng(seed);
    D::Word cur = g.axiom;
    for (int i = 0; i < 8; ++i) cur = g.derive(cur, rng);
    return str(cur);
  };

  // With 2^8 possible sequences it is extremely unlikely all seeds agree.
  bool anyDiffer = false;
  for (uint32_t s = 1; s < 20 && !anyDiffer; ++s)
    if (run(s) != run(0)) anyDiffer = true;
  CHECK(anyDiffer);
}

TEST_CASE("Stochastic: probability distribution (statistical)") {
  // A→B (p=0.8), A→C (p=0.2)  — over 2000 trials expect ~80% B
  D::LSystemGrammar g;
  g.axiom = w("A");
  g.rules = {ruleFor('A').to("B").withProbability(0.8f),
             ruleFor('A').to("C").withProbability(0.2f)};

  std::mt19937 rng(0);
  int countB = 0;
  constexpr int N = 2000;
  for (int i = 0; i < N; ++i) {
    D::Word out = g.derive(g.axiom, rng);
    if (!out.empty() && out[0].letter == 'B') ++countB;
  }
  // σ ≈ sqrt(2000 * 0.8 * 0.2) ≈ 17.9; allow ±5σ
  CHECK(countB > 1510);
  CHECK(countB < 1690);
}

TEST_CASE("Stochastic: no matching rule → symbol passes through") {
  D::LSystemGrammar g;
  g.axiom = w("XAY");
  g.rules = {ruleFor('A').to("B").withProbability(1.f)};  // X and Y have no rules

  std::mt19937 rng(0);
  CHECK(str(g.derive(g.axiom, rng)) == "XBY");
}

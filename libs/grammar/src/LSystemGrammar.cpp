#include "grammar/LSystemGrammar.h"

#include <cassert>
#include <map>
#include <tuple>

#include <spdlog/spdlog.h>

namespace D {

namespace {

// Left context in Biological mode.
//
// Scan leftward from position i, applying three rules:
//   1. ignore chars    — skip over them linearly.
//   2. pop  (']')      — a sibling branch is to the left; skip the entire
//                        matched [...] block and continue on the parent axis.
//   3. push ('[')      — we just crossed the opening of our own branch;
//                        the parent symbol is to the left, so keep going.
//   4. anything else   — return it as the left context.
char leftCtx(const Word& w, size_t i, std::string_view ignore, std::optional<char> push,
             std::optional<char> pop) {
  size_t j = i;
  while (j > 0) {
    char c = w[--j].letter;
    if (ignore.find(c) != std::string_view::npos) continue;
    if (pop && c == *pop) {
      // skip the sibling branch to the left
      int depth = 1;
      while (j > 0 && depth > 0) {
        char b = w[--j].letter;
        if (pop && b == *pop) ++depth;
        if (push && b == *push) --depth;
      }
      continue;
    }
    if (push && c == *push) continue;  // crossed our own branch-open
    return c;
  }
  return '\0';
}

// Right context in Biological mode.
//
// Scan rightward from position i, applying three rules:
//   1. ignore chars    — skip over them linearly.
//   2. push ('[')      — a sub-branch starts; skip the entire [...] block,
//                        then continue on the same axis.
//   3. pop  (']')      — end of our branch.
//                        includeSiblings=false -> no right context (stop).
//                        includeSiblings=true  -> cross the boundary, continue
//                        on the parent axis (Houdini "Context Includes Siblings").
//   4. anything else   — return it as the right context.
char rightCtx(const Word& w, size_t i, std::string_view ignore, std::optional<char> push,
              std::optional<char> pop, bool includeSiblings) {
  size_t j = i + 1;
  while (j < w.size()) {
    char c = w[j].letter;
    if (ignore.find(c) != std::string_view::npos) {
      ++j;
      continue;
    }
    if (push && c == *push) {
      // skip the sub-branch
      int depth = 1;
      ++j;
      while (j < w.size() && depth > 0) {
        char b = w[j].letter;
        if (push && b == *push) ++depth;
        if (pop && b == *pop) --depth;
        ++j;
      }
      continue;
    }
    if (pop && c == *pop) {
      if (!includeSiblings) return '\0';
      ++j;
      continue;  // cross the branch boundary, continue on parent axis
    }
    return c;
  }
  return '\0';
}

}  // namespace

// ── Grammar methods ───────────────────────────────────────────────────────────

bool LSystemGrammar::valid() const {
  // push/pop must not appear in ignore — they have structural roles.
  if (push && ignore.find(*push) != std::string::npos) return false;
  if (pop && ignore.find(*pop) != std::string::npos) return false;

  // Context-char validation only applies in Biological mode.
  if (contextMode == ContextMode::Biological) {
    for (const Rule& r : rules) {
      auto bad = [&](char c) {
        return ignore.find(c) != std::string::npos || (push && c == *push) || (pop && c == *pop);
      };
      if (r.leftContext && bad(*r.leftContext)) return false;
      if (r.rightContext && bad(*r.rightContext)) return false;
    }
  }

  for (size_t i = 0; i < rules.size(); ++i) {
    for (size_t k = i + 1; k < rules.size(); ++k) {
      if (rules[i].predecessor == rules[k].predecessor &&
          rules[i].leftContext == rules[k].leftContext &&
          rules[i].rightContext == rules[k].rightContext && !rules[i].condition &&
          !rules[k].condition && fuzzyEqual(rules[i].probability, 1.f) &&
          fuzzyEqual(rules[k].probability, 1.f))
        spdlog::warn("LSystemGrammar: rule '{}' is shadowed by an earlier rule",
                     rules[k].predecessor);
    }
  }

  // Check that probabilities sum to 1 for stochastic groups.
  // Only rules with p < 1 are stochastic; rules with p == 1 are deterministic
  // and are already covered by the shadowed-rule check above.
  // Conditional rules are skipped — their guards may exclude some alternatives at runtime.
  using GroupKey = std::tuple<char, std::optional<char>, std::optional<char>>;
  std::map<GroupKey, float> probSums;
  for (const Rule& r : rules) {
    if (r.condition) continue;
    if (fuzzyEqual(r.probability, 1.f)) continue;  // deterministic — not stochastic
    probSums[{r.predecessor, r.leftContext, r.rightContext}] += r.probability;
  }
  for (const auto& [key, sum] : probSums) {
    if (!fuzzyEqual(sum, 1.f)) {
      spdlog::warn(
          "LSystemGrammar: rules for '{}' have probabilities summing to {:.4f} (expected 1.0)",
          std::get<0>(key), sum);
      return false;
    }
  }

  return true;
}

Word LSystemGrammar::derive(const Word& current) const {
  assert(valid());

  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];
    bool applied = false;

    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;

      if (rule.leftContext) {
        char lc = (contextMode == ContextMode::Biological) ? leftCtx(current, i, ignore, push, pop)
                                                           : (i > 0 ? current[i - 1].letter : '\0');
        if (lc != *rule.leftContext) continue;
      }

      if (rule.rightContext) {
        char rc = (contextMode == ContextMode::Biological)
                      ? rightCtx(current, i, ignore, push, pop, includeSiblings)
                      : (i + 1 < current.size() ? current[i + 1].letter : '\0');
        if (rc != *rule.rightContext) continue;
      }

      if (rule.condition && !rule.condition(sym.params)) continue;

      assert(rule.successor && "rule has no successor (missing .to(...) call)");
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
  assert(valid());

  Word result;
  result.reserve(current.size() * 2);

  for (size_t i = 0; i < current.size(); ++i) {
    const Symbol& sym = current[i];

    // Collect candidate rules for this symbol
    std::vector<const Rule*> candidates;
    for (const Rule& rule : rules) {
      if (rule.predecessor != sym.letter) continue;

      if (rule.leftContext) {
        char lc = (contextMode == ContextMode::Biological) ? leftCtx(current, i, ignore, push, pop)
                                                           : (i > 0 ? current[i - 1].letter : '\0');
        if (lc != *rule.leftContext) continue;
      }

      if (rule.rightContext) {
        char rc = (contextMode == ContextMode::Biological)
                      ? rightCtx(current, i, ignore, push, pop, includeSiblings)
                      : (i + 1 < current.size() ? current[i + 1].letter : '\0');
        if (rc != *rule.rightContext) continue;
      }

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

    assert(chosen->successor && "rule has no successor (missing .to(...) call)");
    Word produced = chosen->successor(sym.params);
    result.insert(result.end(), produced.begin(), produced.end());
  }

  return result;
}

}  // namespace D

// ── Unit tests ────────────────────────────────────────────────────────────────
// Compiled into apps/tests (DOCTEST_CONFIG_DISABLE is set for the grammar library target).

#include <doctest/doctest.h>

TEST_SUITE("LSystemGrammar/D0L") {
  TEST_CASE("Lindenmayer algae  A->AB, B->A") {
    // L-system:
    //   Alphabet: {A, B}
    //   Axiom:    A
    //   Rules:    A -> AB
    //             B -> A
    // Derivation: A -> AB -> ABA -> ABAAB -> ABAABABA
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {D::ruleFor('A').to("AB"), D::ruleFor('B').to("A")};

    D::Word cur = g.axiom;
    cur = g.derive(cur);
    CHECK(D::str(cur) == "AB");
    cur = g.derive(cur);
    CHECK(D::str(cur) == "ABA");
    cur = g.derive(cur);
    CHECK(D::str(cur) == "ABAAB");
    cur = g.derive(cur);
    CHECK(D::str(cur) == "ABAABABA");
  }

  TEST_CASE("symbol with no matching rule stays unchanged") {
    // Axiom: FAG
    // Rules: A -> B   (F and G have no rule -> pass through)
    // Step 1: FAG -> FBG
    D::LSystemGrammar g;
    g.axiom = D::w("FAG");
    g.rules = {D::ruleFor('A').to("B")};
    CHECK(D::str(g.derive(g.axiom)) == "FBG");
  }

  TEST_CASE("first matching rule wins, others ignored") {
    // Axiom: A
    // Rules: A -> B   (matched first)
    //        A -> C   (never reached)
    // Step 1: A -> B
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {D::ruleFor('A').to("B"), D::ruleFor('A').to("C")};
    CHECK(D::str(g.derive(g.axiom)) == "B");
  }

  TEST_CASE("empty axiom derives to empty") {
    // Axiom: (empty)
    // Rules: A -> B
    // Step 1: (empty) -> (empty)
    D::LSystemGrammar g;
    g.rules = {D::ruleFor('A').to("B")};
    CHECK(g.derive(g.axiom).empty());
  }

  TEST_CASE("no rules — all symbols pass through unchanged") {
    // Axiom: ABC
    // Rules: (none — identity derivation)
    // Step 1: ABC -> ABC
    D::LSystemGrammar g;
    g.axiom = D::w("ABC");
    CHECK(D::str(g.derive(g.axiom)) == "ABC");
  }

}  // TEST_SUITE("LSystemGrammar/D0L")

TEST_SUITE("LSystemGrammar/Biological left context") {
  TEST_CASE("F<A->B: ignore symbols and branch-open skipped") {
    // Axiom: F[+A][-A], ignore="+-", push='[', pop=']'
    // Rules: F < A -> B
    //        A     -> A   (default)
    // Left of first A:  skip '+', skip '[' (branch-open) -> F -> rule fires.
    // Left of second A: skip '-', skip '[' (branch-open),
    //                   skip [+A] sibling branch -> F -> rule fires.
    // Step 1: F[+A][-A] -> F[+B][-B]
    D::LSystemGrammar g;
    g.ignore = "+-";
    g.push = '[';
    g.pop = ']';
    g.axiom = D::w("F[+A][-A]");
    g.rules = {D::ruleFor('A').withLeftContext('F').to("B"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "F[+B][-B]");
  }

  TEST_CASE("mismatch -> default rule fires") {
    // Axiom: G[+A], ignore="+-"
    // Rules: F < A -> B   (left context is G, not F -> mismatch)
    //        A     -> X   (default fires)
    // Step 1: G[+A] -> G[+X]
    D::LSystemGrammar g;
    g.ignore = "+-";
    g.push = '[';
    g.pop = ']';
    g.axiom = D::w("G[+A]");
    g.rules = {D::ruleFor('A').withLeftContext('F').to("B"), D::ruleFor('A').to("X")};
    CHECK(D::str(g.derive(g.axiom)) == "G[+X]");
  }

}  // TEST_SUITE("LSystemGrammar/Biological left context")

TEST_SUITE("LSystemGrammar/Biological right context") {
  TEST_CASE("A>F->B: direct neighbour") {
    // Axiom: AF
    // Rules: A > F -> B   (F is the immediate right neighbour)
    // Step 1: AF -> BF
    D::LSystemGrammar g;
    g.axiom = D::w("AF");
    g.rules = {D::ruleFor('A').withRightContext('F').to("B"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "BF");
  }

  TEST_CASE("A>F->B: ignore symbol skipped") {
    // Axiom: A+F, ignore="+"
    // Rules: A > F -> B   ('+' is skipped, right context is F)
    // Step 1: A+F -> B+F
    D::LSystemGrammar g;
    g.ignore = "+";
    g.axiom = D::w("A+F");
    g.rules = {D::ruleFor('A').withRightContext('F').to("B"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "B+F");
  }

  TEST_CASE("A>C->X: sub-branch skipped, C on main axis reachable") {
    // Axiom: A[B]C, push='[', pop=']'
    // Rules: A > C -> X   ([B] is a sub-branch and is skipped; right context of A is C)
    // Step 1: A[B]C -> X[B]C
    D::LSystemGrammar g;
    g.push = '[';
    g.pop = ']';
    g.axiom = D::w("A[B]C");
    g.rules = {D::ruleFor('A').withRightContext('C').to("X"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "X[B]C");
  }

  TEST_CASE("end of branch -> no right context, default fires") {
    // Axiom: F[A]B, push='[', pop=']'
    // Rules: A > B -> X   (A is inside a branch; ']' ends the branch -> no right context)
    //        A     -> A   (default fires)
    // Step 1: F[A]B -> F[A]B
    D::LSystemGrammar g;
    g.push = '[';
    g.pop = ']';
    g.axiom = D::w("F[A]B");
    g.rules = {D::ruleFor('A').withRightContext('B').to("X"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "F[A]B");
  }

  TEST_CASE("end of string -> no right context, default fires") {
    // Axiom: A
    // Rules: A > F -> B   (A is at end of string -> no right context)
    //        A     -> X   (default fires)
    // Step 1: A -> X
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {D::ruleFor('A').withRightContext('F').to("B"), D::ruleFor('A').to("X")};
    CHECK(D::str(g.derive(g.axiom)) == "X");
  }

  // includeSiblings (Houdini "Context Includes Siblings"):
  // false -> right-context search stops at ']' (default).
  // true  -> ']' is crossed; search continues on the parent axis.
  TEST_CASE("includeSiblings=false (default): ']' stops right-context search") {
    // Axiom: [A]Q[B], push='[', pop=']'
    // Rules: A > Q -> X   (A is inside a branch; without includeSiblings it cannot see Q)
    //        A     -> A   (default fires)
    // Step 1: [A]Q[B] -> [A]Q[B]
    D::LSystemGrammar g;
    g.push = '[';
    g.pop = ']';
    g.axiom = D::w("[A]Q[B]");
    g.rules = {D::ruleFor('A').withRightContext('Q').to("X"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "[A]Q[B]");
  }

  TEST_CASE("includeSiblings=true: right-context crosses branch boundary") {
    // Axiom: [A]Q[B], push='[', pop=']', includeSiblings=true
    // Rules: A > Q -> X   (']' is crossed; Q is the right context of A -> rule fires)
    // Step 1: [A]Q[B] -> [X]Q[B]
    D::LSystemGrammar g;
    g.push = '[';
    g.pop = ']';
    g.includeSiblings = true;
    g.axiom = D::w("[A]Q[B]");
    g.rules = {D::ruleFor('A').withRightContext('Q').to("X"), D::ruleFor('A').to("A")};
    CHECK(D::str(g.derive(g.axiom)) == "[X]Q[B]");
  }

}  // TEST_SUITE("LSystemGrammar/Biological right context")

TEST_SUITE("LSystemGrammar/Strict mode") {
  TEST_CASE("'[' is matchable as left context") {
    // Axiom: [A
    // Rules: [ < A -> B   (Strict: '[' is a plain char, matched as left context)
    //        A     -> X   (default)
    // Step 1: [A -> [B
    D::LSystemGrammar g;
    g.contextMode = D::ContextMode::Strict;
    g.axiom = D::w("[A");
    g.rules = {D::ruleFor('A').withLeftContext('[').to("B"), D::ruleFor('A').to("X")};
    CHECK(D::str(g.derive(g.axiom)) == "[B");
  }

  TEST_CASE("']' is matchable as right context") {
    // Axiom: A]
    // Rules: A > ] -> B   (Strict: ']' is a plain char, matched as right context)
    //        A     -> X   (default)
    // Step 1: A] -> B]
    D::LSystemGrammar g;
    g.contextMode = D::ContextMode::Strict;
    g.axiom = D::w("A]");
    g.rules = {D::ruleFor('A').withRightContext(']').to("B"), D::ruleFor('A').to("X")};
    CHECK(D::str(g.derive(g.axiom)) == "B]");
  }

}  // TEST_SUITE("LSystemGrammar/Strict mode")

TEST_SUITE("LSystemGrammar/valid()") {
  TEST_CASE("no ignore, no push/pop — any context char is valid") {
    // '+' and 'F' are not in ignore and not push/pop -> valid context chars.
    D::LSystemGrammar g;
    g.rules = {
        D::ruleFor('A').withLeftContext('F').to("B"),
        D::ruleFor('A').withRightContext('+').to("C"),
    };
    CHECK(g.valid());
  }

  TEST_CASE("context char in ignore -> invalid") {
    // '+' is in ignore -> cannot be a context char in Biological mode.
    D::LSystemGrammar g;
    g.ignore = "+-|";
    g.rules = {D::ruleFor('A').withLeftContext('+').to("B")};
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("context char is push -> invalid") {
    // '[' is the push symbol -> cannot be a context char in Biological mode.
    D::LSystemGrammar g;
    g.push = '[';
    g.rules = {D::ruleFor('A').withLeftContext('[').to("B")};
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("context char is pop -> invalid") {
    // ']' is the pop symbol -> cannot be a context char in Biological mode.
    D::LSystemGrammar g;
    g.pop = ']';
    g.rules = {D::ruleFor('A').withRightContext(']').to("B")};
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("context char not in ignore, not push/pop -> valid") {
    // 'F' is not in ignore and not push/pop -> valid.
    D::LSystemGrammar g;
    g.ignore = "+-|";
    g.push = '[';
    g.pop = ']';
    g.rules = {D::ruleFor('A').withLeftContext('F').to("B")};
    CHECK(g.valid());
  }

  TEST_CASE("Strict — bracket as context char is valid") {
    // In Strict mode push/pop have no structural role -> '[' is a valid context char.
    D::LSystemGrammar g;
    g.push = '[';
    g.contextMode = D::ContextMode::Strict;
    g.rules = {D::ruleFor('A').withLeftContext('[').to("B")};
    CHECK(g.valid());
  }

  TEST_CASE("probabilities sum to 1 -> valid") {
    // 0.6 + 0.4 = 1.0 exactly -> no warning, valid.
    D::LSystemGrammar g;
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.6f),
        D::ruleFor('A').to("C").withProbability(0.4f),
    };
    CHECK(g.valid());
  }

  TEST_CASE("probabilities do not sum to 1 -> invalid") {
    // 0.6 + 0.3 = 0.9 != 1.0 -> invalid.
    D::LSystemGrammar g;
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.6f),
        D::ruleFor('A').to("C").withProbability(0.3f),
    };
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("single rule with p=1 -> valid") {
    // One rule, p=1.0 -> sum is exactly 1.0 -> valid.
    D::LSystemGrammar g;
    g.rules = {D::ruleFor('A').to("B").withProbability(1.f)};
    CHECK(g.valid());
  }

  TEST_CASE("push in ignore -> invalid") {
    // push symbol ('[') is also in ignore -> structural conflict -> invalid.
    D::LSystemGrammar g;
    g.push = '[';
    g.ignore = "[+-";
    g.rules = {D::ruleFor('A').to("B")};
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("pop in ignore -> invalid") {
    // pop symbol (']') is also in ignore -> structural conflict -> invalid.
    D::LSystemGrammar g;
    g.pop = ']';
    g.ignore = "+-]";
    g.rules = {D::ruleFor('A').to("B")};
    CHECK_FALSE(g.valid());
  }

  TEST_CASE("push/pop not in ignore -> valid") {
    // push and pop are set but absent from ignore -> no conflict -> valid.
    D::LSystemGrammar g;
    g.push = '[';
    g.pop = ']';
    g.ignore = "+-|";
    g.rules = {D::ruleFor('A').to("B")};
    CHECK(g.valid());
  }

  TEST_CASE("Strict mode — probability check still runs") {
    // Probability validation must not be skipped in Strict mode.
    // 0.6 + 0.3 = 0.9 != 1.0 -> invalid even in Strict.
    D::LSystemGrammar g;
    g.contextMode = D::ContextMode::Strict;
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.6f),
        D::ruleFor('A').to("C").withProbability(0.3f),
    };
    CHECK_FALSE(g.valid());
  }

}  // TEST_SUITE("LSystemGrammar/valid()")

TEST_SUITE("LSystemGrammar/Stochastic") {
  TEST_CASE("single rule always fires regardless of RNG") {
    // Axiom: A
    // Rules: A -> B  (p=1.0)
    // With p=1.0 there is only one candidate; RNG has no effect.
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {D::ruleFor('A').to("B").withProbability(1.f)};
    std::mt19937 rng(0);
    for (int i = 0; i < 20; ++i) CHECK(D::str(g.derive(g.axiom, rng)) == "B");
  }

  TEST_CASE("same seed produces same output") {
    // Axiom: A
    // Rules: A -> B  (p=0.5)
    //        A -> C  (p=0.5)
    // Stochastic derive is deterministic given the same seed.
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.5f),
        D::ruleFor('A').to("C").withProbability(0.5f),
    };
    auto run = [&](uint32_t seed) {
      std::mt19937 rng(seed);
      D::Word cur = g.axiom;
      for (int i = 0; i < 10; ++i) cur = g.derive(cur, rng);
      return D::str(cur);
    };
    CHECK(run(42) == run(42));
    CHECK(run(123) == run(123));
  }

  TEST_CASE("different seeds may produce different outputs") {
    // Axiom: A
    // Rules: A -> B  (p=0.5)
    //        A -> C  (p=0.5)
    // Verifies the RNG is actually used: if it were ignored, every seed would
    // produce the same string.  After 8 steps the word has 256 symbols;
    // at least one of 20 seeds must differ from seed 0.
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.5f),
        D::ruleFor('A').to("C").withProbability(0.5f),
    };
    auto run = [&](uint32_t seed) {
      std::mt19937 rng(seed);
      D::Word cur = g.axiom;
      for (int i = 0; i < 8; ++i) cur = g.derive(cur, rng);
      return D::str(cur);
    };
    std::string baseline = run(0);
    bool anyDiffer = false;
    for (uint32_t s = 1; s < 20 && !anyDiffer; ++s)
      if (run(s) != baseline) anyDiffer = true;
    CHECK(anyDiffer);
  }

  TEST_CASE("probability distribution (statistical)") {
    // Axiom: A
    // Rules: A -> B  (p=0.8)
    //        A -> C  (p=0.2)
    // Verifies probability weights are applied: p=0.8 must produce ~80% B,
    // not 50% (equal weights) or 100% (first rule always wins).
    //   n=2000, p=0.8, q=0.2
    //   E = n*p               = 2000*0.8          = 1600
    //   sigma = sqrt(n*p*q)   = sqrt(2000*0.8*0.2) = sqrt(320) ~= 17.9
    //   bounds: E +/- 5*sigma ~= 1600 +/- 90       -> [1510, 1690]
    // False-positive probability at +/-5*sigma: < 0.0001%
    D::LSystemGrammar g;
    g.axiom = D::w("A");
    g.rules = {
        D::ruleFor('A').to("B").withProbability(0.8f),
        D::ruleFor('A').to("C").withProbability(0.2f),
    };
    std::mt19937 rng(0);
    int countB = 0;
    for (int i = 0; i < 2000; ++i) {
      D::Word out = g.derive(g.axiom, rng);
      if (!out.empty() && out[0].letter == 'B') ++countB;
    }
    CHECK(countB > 1510);  // E - 5*sigma
    CHECK(countB < 1690);  // E + 5*sigma
  }

  TEST_CASE("no matching rule -> symbol passes through") {
    // Axiom: XAY
    // Rules: A -> B  (p=1.0)   (X and Y have no rule -> pass through)
    // Step 1: XAY -> XBY
    D::LSystemGrammar g;
    g.axiom = D::w("XAY");
    g.rules = {D::ruleFor('A').to("B").withProbability(1.f)};
    std::mt19937 rng(0);
    CHECK(D::str(g.derive(g.axiom, rng)) == "XBY");
  }

}  // TEST_SUITE("LSystemGrammar/Stochastic")

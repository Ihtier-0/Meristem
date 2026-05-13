#include "ViewerUI.h"

#include <imgui.h>

#include "examples.h"

namespace D {

// ── Helpers ──────────────────────────────────────────────────────────────────

static std::string wordToString(const Word& word) {
  std::string s;
  s.reserve(word.size());
  for (const auto& sym : word) s += sym.letter;
  return s;
}

static Word stringToWord(std::string_view s) {
  Word w;
  w.reserve(s.size());
  for (char c : s) w.emplace_back(c);
  return w;
}

// ── ViewerUI ─────────────────────────────────────────────────────────────────

ViewerUI::ViewerUI(OpenGLRenderer& renderer)
    : m_renderer(renderer),
      m_grammar(examples::binaryTree()),
      m_turtle(m_grammar.angle),
      m_angleOverride(m_grammar.angle),
      m_stepLen(m_turtle.step()) {
  m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  m_mesh = buildMesh(m_turtle, m_algo->getStructure());

  auto axiomStr = wordToString(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.reserve(m_grammar.rules.size());
  for (const auto& rule : m_grammar.rules) {
    RuleEdit re;
    re.predecessor[0] = rule.predecessor;
    re.probability = rule.probability;
    auto str = wordToString(rule.successor({}));
    auto slen = std::min(str.size(), sizeof(re.successor) - 1);
    std::copy_n(str.begin(), slen, re.successor);
    m_ruleEdits.push_back(re);
  }
}

void ViewerUI::draw() {
  m_renderer.setClearColor(m_bgColor);

  float nextY = 10.f;
  drawControlPanel(nextY);
  drawGrammarPanel(nextY);
  drawSettingsPanel(nextY);
}

void ViewerUI::rebuildMesh() {
  m_turtle.setAngle(m_angleOverride);
  m_turtle.setStep(m_stepLen);
  m_mesh = buildMesh(m_turtle, m_algo->getStructure());
}

void ViewerUI::switchAlgo(AlgoType type) {
  m_algoType = type;

  LSystemGrammar g = (type == AlgoType::D0L)
      ? examples::binaryTree()
      : examples::stochasticPlant();
  m_grammar = g;

  if (type == AlgoType::D0L)
    m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  else
    m_algo = std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));

  m_angleOverride = m_grammar.angle;
  m_turtle.setAngle(m_angleOverride);

  // Rebuild grammar panel state
  auto axiomStr = wordToString(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::fill(std::begin(m_axiomBuf), std::end(m_axiomBuf), '\0');
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.clear();
  for (const auto& rule : m_grammar.rules) {
    RuleEdit re;
    re.predecessor[0] = rule.predecessor;
    re.probability = rule.probability;
    auto str = wordToString(rule.successor({}));
    auto slen = std::min(str.size(), sizeof(re.successor) - 1);
    std::copy_n(str.begin(), slen, re.successor);
    m_ruleEdits.push_back(re);
  }

  rebuildMesh();
}

void ViewerUI::applyGrammar() {
  m_grammar.axiom = stringToWord(m_axiomBuf);
  m_grammar.angle = m_angleOverride;
  m_grammar.rules.clear();
  for (const auto& re : m_ruleEdits) {
    if (re.predecessor[0] == '\0') continue;
    Rule rule;
    rule.predecessor = re.predecessor[0];
    rule.probability = re.probability;
    std::string succStr = re.successor;
    rule.successor = [succStr](std::span<const ParamValue>) -> Word {
      return stringToWord(succStr);
    };
    m_grammar.rules.push_back(std::move(rule));
  }

  if (m_algoType == AlgoType::D0L)
    m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  else
    m_algo = std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));

  rebuildMesh();
}

void ViewerUI::drawControlPanel(float& nextY) {
  constexpr float kW = 280.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("L-System", nullptr, ImGuiWindowFlags_NoResize);

  // Algorithm type combo
  static const char* kAlgoNames[] = {"D0L (deterministic)", "Stochastic"};
  int currentItem = static_cast<int>(m_algoType);
  ImGui::SetNextItemWidth(-1);
  if (ImGui::Combo("##algo", &currentItem, kAlgoNames, 2)) {
    switchAlgo(static_cast<AlgoType>(currentItem));
  }

  ImGui::Separator();

  ImGui::Text("Generation: %d", m_algo->generation());
  ImGui::Text("Symbols:    %zu",
              std::get<StringStructure>(m_algo->getStructure()).derivation.size());

  ImGui::Separator();

  if (m_algoType == AlgoType::Stochastic) {
    if (ImGui::InputInt("Seed", &m_seed)) {
      if (m_seed < 0) m_seed = 0;
      static_cast<StochasticLSystemAlgorithm*>(m_algo.get())
          ->setSeed(static_cast<uint32_t>(m_seed));
      rebuildMesh();
    }
  }

  if (ImGui::SliderFloat("Angle",    &m_angleOverride, 5.f,    90.f))   rebuildMesh();
  if (ImGui::SliderFloat("Step len", &m_stepLen,       0.1f,   10.f))   rebuildMesh();
  if (ImGui::SliderFloat("Zoom",     &m_zoom,          0.1f,   20.f))   m_renderer.setZoom(m_zoom);
  if (ImGui::SliderFloat("Pan X",    &m_panX,         -100.f, 100.f))   m_renderer.setPan(m_panX, m_panY);
  if (ImGui::SliderFloat("Pan Y",    &m_panY,         -100.f, 100.f))   m_renderer.setPan(m_panX, m_panY);

  ImGui::Separator();
  if (ImGui::Button("Step")) { m_algo->step(); rebuildMesh(); }
  ImGui::SameLine();
  if (ImGui::Button("Reset")) { m_algo->reset(); rebuildMesh(); }

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

void ViewerUI::drawGrammarPanel(float& nextY) {
  constexpr float kW = 280.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("Grammar", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::SetNextItemWidth(-1);
  ImGui::InputText("##axiom", m_axiomBuf, sizeof(m_axiomBuf));
  ImGui::SameLine(0, 0);
  ImGui::Text(" Axiom");

  ImGui::Separator();

  const bool isStochastic = (m_algoType == AlgoType::Stochastic);

  for (int i = 0; i < static_cast<int>(m_ruleEdits.size()); ++i) {
    ImGui::PushID(i);
    ImGui::SetNextItemWidth(22);
    ImGui::InputText("##pred", m_ruleEdits[i].predecessor, 2);
    ImGui::SameLine();
    ImGui::TextUnformatted("->");
    ImGui::SameLine();
    float succW = isStochastic ? 110.f : 180.f;
    ImGui::SetNextItemWidth(succW);
    ImGui::InputText("##succ", m_ruleEdits[i].successor, sizeof(m_ruleEdits[i].successor));
    if (isStochastic) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(44.f);
      ImGui::InputFloat("##prob", &m_ruleEdits[i].probability, 0.f, 0.f, "%.2f");
    }
    ImGui::SameLine();
    if (ImGui::Button("X")) { m_ruleEdits.erase(m_ruleEdits.begin() + i); --i; }
    ImGui::PopID();
  }

  if (ImGui::Button("+ Rule")) m_ruleEdits.emplace_back();
  ImGui::Separator();
  if (ImGui::Button("Apply")) applyGrammar();

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

void ViewerUI::drawSettingsPanel(float& nextY) {
  constexpr float kW = 280.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::ColorEdit3("Line color",       reinterpret_cast<float*>(&m_lineColor));
  ImGui::ColorEdit3("Background color", reinterpret_cast<float*>(&m_bgColor));

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

}  // namespace D

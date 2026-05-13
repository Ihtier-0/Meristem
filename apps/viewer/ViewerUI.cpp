#include "ViewerUI.h"

#include <imgui.h>

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

ViewerUI::ViewerUI(LSystemGrammar& grammar, D0LSystemAlgorithm& algo,
                   TurtleBuilder2D& turtle, Mesh& mesh, OpenGLRenderer& renderer)
    : m_grammar(grammar),
      m_algo(algo),
      m_turtle(turtle),
      m_mesh(mesh),
      m_renderer(renderer),
      m_angleOverride(grammar.angle),
      m_stepLen(turtle.step()) {
  auto axiomStr = wordToString(grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.reserve(grammar.rules.size());
  for (const auto& rule : grammar.rules) {
    RuleEdit re;
    re.predecessor[0] = rule.predecessor;
    auto str = wordToString(rule.successor({}));
    auto slen = std::min(str.size(), sizeof(re.successor) - 1);
    std::copy_n(str.begin(), slen, re.successor);
    m_ruleEdits.push_back(re);
  }
}

void ViewerUI::draw() {
  drawControlPanel();
  drawGrammarPanel();
}

void ViewerUI::rebuildMesh() {
  m_turtle.setAngle(m_angleOverride);
  m_turtle.setStep(m_stepLen);
  m_mesh = buildMesh(m_turtle, m_algo.getStructure());
}

void ViewerUI::applyGrammar() {
  m_grammar.axiom = stringToWord(m_axiomBuf);
  m_grammar.rules.clear();
  for (const auto& re : m_ruleEdits) {
    if (re.predecessor[0] == '\0') continue;
    Rule rule;
    rule.predecessor = re.predecessor[0];
    std::string succStr = re.successor;
    rule.successor = [succStr](std::span<const ParamValue>) -> Word {
      return stringToWord(succStr);
    };
    m_grammar.rules.push_back(std::move(rule));
  }
  m_algo = D0LSystemAlgorithm(m_grammar);
  rebuildMesh();
}

void ViewerUI::drawControlPanel() {
  ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({260, 0}, ImGuiCond_Always);
  ImGui::Begin("L-System", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::Text("Generation: %d", m_algo.generation());
  ImGui::Text("Symbols:    %zu",
              std::get<StringStructure>(m_algo.getStructure()).derivation.size());
  ImGui::Separator();

  if (ImGui::Button("Step")) {
    m_algo.step();
    rebuildMesh();
  }
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    m_algo.reset();
    rebuildMesh();
  }

  ImGui::Separator();
  if (ImGui::SliderFloat("Angle", &m_angleOverride, 5.f, 90.f))   rebuildMesh();
  if (ImGui::SliderFloat("Step len", &m_stepLen, 0.1f, 10.f))     rebuildMesh();
  if (ImGui::SliderFloat("Zoom",  &m_zoom,  0.1f, 20.f))          m_renderer.setZoom(m_zoom);
  if (ImGui::SliderFloat("Pan X", &m_panX, -100.f, 100.f))        m_renderer.setPan(m_panX, m_panY);
  if (ImGui::SliderFloat("Pan Y", &m_panY, -100.f, 100.f))        m_renderer.setPan(m_panX, m_panY);

  ImGui::End();
}

void ViewerUI::drawGrammarPanel() {
  ImGui::SetNextWindowPos({280, 10}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({340, 0}, ImGuiCond_Always);
  ImGui::Begin("Grammar", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::SetNextItemWidth(-1);
  ImGui::InputText("##axiom", m_axiomBuf, sizeof(m_axiomBuf));
  ImGui::SameLine(0, 0);
  ImGui::Text(" Axiom");

  ImGui::Separator();

  for (int i = 0; i < static_cast<int>(m_ruleEdits.size()); ++i) {
    ImGui::PushID(i);

    ImGui::SetNextItemWidth(24);
    ImGui::InputText("##pred", m_ruleEdits[i].predecessor, 2);
    ImGui::SameLine();
    ImGui::TextUnformatted("->");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(210);
    ImGui::InputText("##succ", m_ruleEdits[i].successor, sizeof(m_ruleEdits[i].successor));
    ImGui::SameLine();
    if (ImGui::Button("X")) {
      m_ruleEdits.erase(m_ruleEdits.begin() + i);
      --i;
    }

    ImGui::PopID();
  }

  if (ImGui::Button("+ Rule")) m_ruleEdits.emplace_back();

  ImGui::Separator();
  if (ImGui::Button("Apply")) applyGrammar();

  ImGui::End();
}

}  // namespace D

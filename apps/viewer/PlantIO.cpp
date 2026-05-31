#include "PlantIO.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include "TreeCanvas.h"

namespace D {

namespace {

constexpr int kFormatVersion = 1;

QString algoKeyword(int t) {
  switch (static_cast<TreeCanvas::AlgoType>(t)) {
    case TreeCanvas::AlgoType::D0L: return "d0l";
    case TreeCanvas::AlgoType::Stochastic: return "stochastic";
    case TreeCanvas::AlgoType::ContextSensitive: return "context-1l";
    case TreeCanvas::AlgoType::ContextSensitive2L: return "context-2l";
    case TreeCanvas::AlgoType::Parametric: return "parametric";
    case TreeCanvas::AlgoType::ContextFlower: return "context-flower";
  }
  return "d0l";
}

std::optional<int> algoFromKeyword(const QString& k) {
  if (k == "d0l") return static_cast<int>(TreeCanvas::AlgoType::D0L);
  if (k == "stochastic") return static_cast<int>(TreeCanvas::AlgoType::Stochastic);
  if (k == "context-1l") return static_cast<int>(TreeCanvas::AlgoType::ContextSensitive);
  if (k == "context-2l") return static_cast<int>(TreeCanvas::AlgoType::ContextSensitive2L);
  if (k == "parametric") return static_cast<int>(TreeCanvas::AlgoType::Parametric);
  if (k == "context-flower") return static_cast<int>(TreeCanvas::AlgoType::ContextFlower);
  return std::nullopt;
}

char firstChar(const QString& s) { return s.isEmpty() ? '\0' : s[0].toLatin1(); }

}  // namespace

QString serializePlant(const TreeCanvas& c) {
  QJsonObject root;
  root["meristem"] = "plant";
  root["version"] = kFormatVersion;
  root["algo"] = algoKeyword(static_cast<int>(c.algoType()));
  root["angle"] = c.angle();
  root["step"] = c.stepLen();
  root["seed"] = c.seed();

  if (c.algoType() == TreeCanvas::AlgoType::Parametric) {
    QJsonObject p;
    p["axiom"] = QString::fromLatin1(c.paramAxiomBuf());
    QJsonArray rules;
    for (const auto& r : c.paramRuleEdits()) {
      if (r.predecessor[0] == '\0') continue;
      QJsonObject ro;
      ro["pred"] = QString(QChar(r.predecessor[0]));
      ro["params"] = QString::fromLatin1(r.paramNames);
      ro["expr"] = QString::fromLatin1(r.successorExpr);
      rules.append(ro);
    }
    p["rules"] = rules;
    QJsonArray defs;
    for (const auto& d : c.paramDefs()) {
      if (d.name[0] == '\0') continue;
      QJsonObject obj;
      obj["name"] = QString::fromLatin1(d.name);
      obj["value"] = static_cast<double>(d.value);
      defs.append(obj);
    }
    p["params"] = defs;
    root["parametric"] = p;
  } else {
    QJsonObject g;
    g["axiom"] = QString::fromLatin1(c.axiomBuf());
    const auto ctx = c.contextEdit();
    g["ignore"] = QString::fromLatin1(ctx.ignore);
    if (ctx.push[0]) g["push"] = QString(QChar(ctx.push[0]));
    if (ctx.pop[0]) g["pop"] = QString(QChar(ctx.pop[0]));
    g["siblings"] = ctx.includeSiblings;
    g["strict"] = ctx.strictMode;
    QJsonArray rules;
    for (const auto& r : c.ruleEdits()) {
      if (r.predecessor[0] == '\0') continue;
      QJsonObject ro;
      ro["pred"] = QString(QChar(r.predecessor[0]));
      if (r.leftContext[0]) ro["left"] = QString::fromLatin1(r.leftContext);
      if (r.rightContext[0]) ro["right"] = QString::fromLatin1(r.rightContext);
      ro["succ"] = QString::fromLatin1(r.successor);
      if (r.probability < 0.999f || r.probability > 1.001f)
        ro["prob"] = static_cast<double>(r.probability);
      rules.append(ro);
    }
    g["rules"] = rules;
    root["grammar"] = g;
  }

  return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

std::optional<PlantDoc> parsePlant(const QString& text, QString* error) {
  QJsonParseError perr;
  const QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &perr);
  if (doc.isNull() || !doc.isObject()) {
    if (error) *error = perr.errorString();
    return std::nullopt;
  }
  const QJsonObject root = doc.object();

  PlantDoc d;
  if (auto a = algoFromKeyword(root.value("algo").toString())) d.algoType = *a;
  d.angle = root.value("angle").toDouble(d.angle);
  d.step = root.value("step").toDouble(d.step);
  d.seed = root.value("seed").toInt(d.seed);

  if (root.contains("grammar")) {
    const QJsonObject g = root.value("grammar").toObject();
    d.axiom = g.value("axiom").toString().toStdString();
    d.ignore = g.value("ignore").toString().toStdString();
    if (g.contains("push")) { d.push = firstChar(g.value("push").toString()); d.hasPush = d.push != '\0'; }
    if (g.contains("pop")) { d.pop = firstChar(g.value("pop").toString()); d.hasPop = d.pop != '\0'; }
    d.includeSiblings = g.value("siblings").toBool(false);
    d.strict = g.value("strict").toBool(false);
    for (const QJsonValue& v : g.value("rules").toArray()) {
      const QJsonObject ro = v.toObject();
      PlantDoc::Rule r;
      r.pred = firstChar(ro.value("pred").toString());
      r.left = ro.value("left").toString().toStdString();
      r.right = ro.value("right").toString().toStdString();
      r.succ = ro.value("succ").toString().toStdString();
      r.prob = static_cast<float>(ro.value("prob").toDouble(1.0));
      if (r.pred != '\0') d.rules.push_back(r);
    }
  }

  if (root.contains("parametric")) {
    const QJsonObject p = root.value("parametric").toObject();
    d.paramAxiom = p.value("axiom").toString().toStdString();
    for (const QJsonValue& v : p.value("rules").toArray()) {
      const QJsonObject ro = v.toObject();
      PlantDoc::PRule r;
      r.pred = firstChar(ro.value("pred").toString());
      r.params = ro.value("params").toString().toStdString();
      r.expr = ro.value("expr").toString().toStdString();
      if (r.pred != '\0') d.paramRules.push_back(r);
    }
    for (const QJsonValue& v : p.value("params").toArray()) {
      const QJsonObject obj = v.toObject();
      PlantDoc::PDef pd;
      pd.name = obj.value("name").toString().toStdString();
      pd.value = static_cast<float>(obj.value("value").toDouble());
      if (!pd.name.empty()) d.paramDefs.push_back(pd);
    }
  }

  return d;
}

bool savePlantFile(const TreeCanvas& c, const QString& path, QString* error) {
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    if (error) *error = f.errorString();
    return false;
  }
  QTextStream out(&f);
  out << serializePlant(c);
  return true;
}

bool loadPlantFile(TreeCanvas& c, const QString& path, QString* error) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (error) *error = f.errorString();
    return false;
  }
  QTextStream in(&f);
  const QString text = in.readAll();
  auto doc = parsePlant(text, error);
  if (!doc) return false;
  c.loadPlant(*doc);
  return true;
}

}  // namespace D

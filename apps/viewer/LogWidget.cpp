#include "LogWidget.h"

#include <QCheckBox>
#include <QFont>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <spdlog/spdlog.h>

namespace D {

LogWidget::LogWidget(QWidget* parent) : QWidget(parent) {
  auto* lay = new QVBoxLayout(this);
  lay->setContentsMargins(4, 4, 4, 4);
  lay->setSpacing(4);

  // ── Toolbar ───────────────────────────────────────────────────────────────────

  auto* toolbar = new QWidget;
  auto* tbLay   = new QHBoxLayout(toolbar);
  tbLay->setContentsMargins(0, 0, 0, 0);

  auto* verboseCheck = new QCheckBox("Verbose");
  connect(verboseCheck, &QCheckBox::toggled, this, [](bool on) {
    spdlog::set_level(on ? spdlog::level::debug : spdlog::level::info);
  });
  tbLay->addWidget(verboseCheck);
  tbLay->addStretch();

  auto* clearBtn = new QPushButton("Clear");
  tbLay->addWidget(clearBtn);

  lay->addWidget(toolbar);

  // ── Log output ────────────────────────────────────────────────────────────────

  m_log = new QPlainTextEdit;
  m_log->setReadOnly(true);
  m_log->setMaximumBlockCount(2000);

  QFont font;
  font.setFamily("Consolas");
  font.setPointSize(9);
  font.setStyleHint(QFont::Monospace);
  m_log->setFont(font);

  lay->addWidget(m_log);

  connect(clearBtn, &QPushButton::clicked, m_log, &QPlainTextEdit::clear);
}

void LogWidget::appendMessage(const QString& msg) {
  if (!isVisible()) return;
  m_log->appendPlainText(msg);
}

}  // namespace D

//
// Modified for MABXII integration
//
#pragma once

#include <QWidget>
#include <vector>

#include "selfdrive/ui/ui.h"

class CustomControlUI : public QWidget {
  Q_OBJECT

public:
  CustomControlUI(QWidget *parent = 0) : QWidget(parent) {}
  void updateState(const UIState &s);
  void clear();


protected:
  struct ControlStatus {
    std::vector<QString> texts;

    bool equal(const ControlStatus &other) const {
      return texts == other.texts;
    }
  };

  void paintEvent(QPaintEvent*) override;
  ControlStatus getControlStatus(const SubMaster &sm, uint64_t started_frame);

  ControlStatus status = {};
};
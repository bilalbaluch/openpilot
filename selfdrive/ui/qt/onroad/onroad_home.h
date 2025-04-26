#pragma once

#include "selfdrive/ui/qt/onroad/alerts.h"
#include "selfdrive/ui/qt/onroad/customcontrol.h"
#include "selfdrive/ui/qt/onroad/annotated_camera.h"

class OnroadWindow : public QWidget {
  Q_OBJECT

public:
  OnroadWindow(QWidget* parent = 0);

private:
  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);
  OnroadAlerts *alerts;
  CustomControlUI *customControl;
  AnnotatedCameraWidget *nvg;
  QColor bg = bg_colors[STATUS_DISENGAGED];
  QHBoxLayout* split;
  QWidget *customControlBox;

private slots:
  void offroadTransition(bool offroad);
  void updateState(const UIState &s);
};
//
// Modified for MABXII integration
//

#include "selfdrive/ui/qt/onroad/customcontrol.h"

#include <QPainter>
#include <map>
#include <string>

#include "selfdrive/ui/qt/util.h"

void CustomControlUI::updateState(const UIState &s) {
  ControlStatus p = getControlStatus(*(s.sm), s.scene.started_frame);
  if (!status.equal(p)) {
    status = p;
    update();
  }
}

void CustomControlUI::clear() {
  status = {};
  update();
}

CustomControlUI::ControlStatus CustomControlUI::getControlStatus(const SubMaster &sm, uint64_t started_frame) {
  const cereal::ControlsState::Reader &cs = sm["controlsState"].getControlsState();
  const uint64_t controls_frame = sm.rcv_frame("controlsState");

  // times are float32
  const float start = cs.getProfileStartTime();
  const float time = cs.getProfileCurrentTime();
  const float elapsed = time - start;

  // MABXII status and acceleration values
  const bool mabxii_active = cs.getMabxiiActive();
  const float target_accel = cs.getTargetAccel();
  const float accel = cs.getProfileActualAccel();
  const std::string history = cs.getProfileHistory().cStr();

  // car state stuff
  const auto car_state = sm["carState"].getCarState();
  float v_ego = car_state.getVEgo();
  float a_ego = car_state.getAEgo();

  ControlStatus p;
  if (controls_frame >= started_frame) {  // Don't get old status
    if (cs.getCustomProfileEnabled()) {
      p.texts.push_back(tr("MABXII Control Mode"));
    } else {
      p.texts.push_back(tr("Stock OP Mode"));
    }

    if (cs.getEnabled()) {
      p.texts.push_back(tr("Engaged"));
    } else {
      p.texts.push_back(tr("Disengaged"));
    }

    if (cs.getProfileRunning()) {
      if (mabxii_active) {
        p.texts.push_back(tr("MABXII Active"));
        p.texts.push_back(QString::fromStdString("Target Accel: " + QString::number(target_accel, 'f', 2).toStdString() + " m/s²"));
      } else {
        p.texts.push_back(tr("MABXII Inactive - Waiting for CAN Message"));
      }
    } else {
      p.texts.push_back(tr("Control Stopped"));
    }

    p.texts.push_back(QString::fromStdString("Current Accel: " + QString::number(accel, 'f', 2).toStdString() + " m/s²"));
    p.texts.push_back(QString::fromStdString("Vehicle Accel: " + QString::number(a_ego, 'f', 2).toStdString() + " m/s²"));
    p.texts.push_back(QString::fromStdString("Vehicle Speed: " + QString::number(v_ego * 3.6, 'f', 1).toStdString() + " km/h"));
    p.texts.push_back(QString::fromStdString("Elapsed Time: " + QString::number(elapsed, 'f', 2).toStdString() + " s"));
    
    if (!history.empty()) {
      p.texts.push_back(QString::fromStdString("\nRecent Commands:"));
      p.texts.push_back(QString::fromStdString(history));
    }
  } else {
    p.texts.push_back(tr("Unknown"));
    p.texts.push_back(tr("Unknown"));
  }

  return p;
}

void CustomControlUI::paintEvent(QPaintEvent *event) {
  int h = 600;
  int w = 600;
  int margin = 40;
  int radius = 20;
  int font_size = 24;

  // align to centre-right of screen
  QRect r = QRect(width() - w - margin, (height() - h) / 2, w, h);

  QPainter p(this);

  // drawing background
  p.setPen(Qt::NoPen);
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
  p.setBrush(QColor(0x15, 0x15, 0x15, 0x80));
  p.drawRoundedRect(r, radius, radius);

  // drawing text
  p.setPen(Qt::white);
  p.setRenderHint(QPainter::TextAntialiasing);
  p.setFont(InterFont(font_size, QFont::DemiBold));

  if (status.texts.size() > 0) {
    p.drawText(r.adjusted(10, 10, 0, 0), Qt::AlignLeft, status.texts[0]);
  }
  if (status.texts.size() > 1) {
    p.drawText(r.adjusted(10, 50, 0, 0), Qt::AlignLeft, status.texts[1]);
  }

  for (int i = 2; i < status.texts.size(); i++) {
    p.drawText(r.adjusted(10, 50 + i * 40, 0, 0), Qt::AlignLeft, status.texts[i]);
  }
}
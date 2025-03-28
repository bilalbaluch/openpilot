//
// Created by geeth on 7/20/24.
// Modified by bilal on 14/03/25.
//

#include "selfdrive/ui/qt/onroad/profile.h"

#include <QPainter>
#include <map>
#include <string>

#include "selfdrive/ui/qt/util.h"

void Profile::updateState(const UIState &s) {
  ProfileStatus p = getProfileStatus(*(s.sm), s.scene.started_frame);
  if (!profile.equal(p)) {
    profile = p;
    update();
  }
}

void Profile::clear() {
  profile = {};
  update();
}

Profile::ProfileStatus Profile::getProfileStatus(const SubMaster &sm, uint64_t started_frame) {
  const cereal::ControlsState::Reader &cs = sm["controlsState"].getControlsState();
  const uint64_t controls_frame = sm.rcv_frame("controlsState");

  // times are float32
  const float start = cs.getProfileStartTime();
  const float time = cs.getProfileCurrentTime();
  const float elapsed = time - start;

  // profile
  const std::string plan = cs.getProfilePlan().cStr();
  const uint8_t stage = cs.getProfileStage();
  const float accel = cs.getProfileActualAccel();
  const std::string history = cs.getProfileHistory().cStr();

  // Get the currently selected profile
  const std::string profile_plan = cs.getCustomProfilePlan().cStr();  

  // car state stuff
  const auto car_state = sm["carState"].getCarState();
  float v_ego = car_state.getVEgo();
  float a_ego = car_state.getAEgo();

  ProfileStatus p;
  if (controls_frame >= started_frame) {  // Don't get old profile.
    if (cs.getCustomProfileEnabled()) {
      p.texts.push_back(tr("Custom Profile Mode"));
      
      // Map profile numbers to names
      std::map<std::string, QString> profile_names = {
        {"0", tr("Step 1 & -2")},
        {"1", tr("Step 1.5 & -1.5")},
        {"2", tr("Step 2 & -1")},
        {"3", tr("Step 2 & -0.5")},        
        {"4", tr("Step 0.5 & -2.5 (1)")},
        {"5", tr("Step 0.5 & -2.5 (2)")},
        {"6", tr("Step 0.75 & -1.75")},
        {"7", tr("Step 1.25 & -1.25")},
        {"8", tr("Step 1.75 & -0.75")},
        {"9", tr("Step 0.25 & -2.25 (1)")},
        {"10", tr("Step 0.25 & -2.25 (2)")},
        {"11", tr("UDDS (1)")},
        {"12", tr("UDDS (2)")},
        {"13", tr("UDDS (3)")},
        {"14", tr("UDDS (4)")},
        {"15", tr("UDDS (5)")},
        {"16", tr("UDDS (6)")},
        {"17", tr("UDDS (7)")},
        {"18", tr("UDDS (8)")},
        {"19", tr("UDDS (9)")},
        {"20", tr("US06 (1)")},
        {"21", tr("US06 (2)")},
        {"22", tr("US06 (3)")},
        {"23", tr("US06 (4)")},
        {"24", tr("US06 (5)")},
        {"25", tr("US06 (6)")},
        {"26", tr("US06 (7)")},
        {"27", tr("US06 (8)")},
        {"28", tr("US06 (9)")},
        {"29", tr("US06 (10)")}
      };
      // You can add more profiles here (in case of addition/removal of profile, kindly also update other files: 
      // "../selfdrive/ui/qt/offroad/settings.cc" & "../selfdrive/controls/controlsd.py")
      if (profile_names.find(profile_plan) != profile_names.end()) {
        p.texts.push_back(profile_names[profile_plan]);
      } else {
        p.texts.push_back(tr("Unknown Profile"));
      }
    } else {
      p.texts.push_back(tr("Stock OP Mode"));
      p.texts.push_back(tr("Disengaged"));
    }

    if (cs.getEnabled()) {
      p.texts.push_back(tr("Engaged"));
    } else if (!cs.getCustomProfileEnabled()) {
      p.texts.push_back(tr("Disengaged"));
    }

    if (cs.getProfileRunning()) {
      p.texts.push_back(tr("Profile Running"));
    } else {
      p.texts.push_back(tr("Profile Stopped"));
    }

    p.texts.push_back(QString::fromStdString("Plan: " + plan));
    p.texts.push_back(QString::fromStdString("Stage: " + std::to_string(stage + 1)));
    p.texts.push_back(QString::fromStdString("Accel: " + QString::number(accel, 'f', 2).toStdString()));
    p.texts.push_back(QString::fromStdString("Elapsed Time: " + QString::number(elapsed, 'f', 2).toStdString()));
    p.texts.push_back(QString::fromStdString("V_Ego: " + QString::number(v_ego, 'f', 2).toStdString()));
    p.texts.push_back(QString::fromStdString("A_Ego: " + QString::number(a_ego, 'f', 2).toStdString()));
    p.texts.push_back(QString::fromStdString("\nHistory: "));
    p.texts.push_back(QString::fromStdString(history));
  } else {
    p.texts.push_back(tr("Unknown"));
    p.texts.push_back(tr("Unknown"));
  }

  return p;
}

void Profile::paintEvent(QPaintEvent *event) {
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

  if (profile.texts.size() > 0) {
    p.drawText(r.adjusted(10, 10, 0, 0), Qt::AlignLeft, profile.texts[0]);
  }
  if (profile.texts.size() > 1) {
    p.drawText(r.adjusted(10, 50, 0, 0), Qt::AlignLeft, profile.texts[1]);
  }

  for (int i = 2; i < profile.texts.size(); i++) {
    p.drawText(r.adjusted(10, 50 + i * 40, 0, 0), Qt::AlignLeft, profile.texts[i]);
  }
}
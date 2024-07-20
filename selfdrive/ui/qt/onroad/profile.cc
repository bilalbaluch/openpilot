//
// Created by geeth on 7/20/24.
//

#include "selfdrive/ui/qt/onroad/profile.h"

#include <QPainter>
#include <map>

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

  ProfileStatus p = {};
  if (controls_frame >= started_frame) {  // Don't get old profile.
    if (cs.getCustomProfileEnabled()) {
      p = {tr("Enabled")};
    } else {
      p = {tr("Disabled")};
    }
  } else {
    p = {tr("Disabled?")};
  }

  return p;

}

void Profile::paintEvent(QPaintEvent *event) {

  int h = 200;
  int w = 300;

  int margin = 40;
  int radius = 20;

  // align to center-right of screen
  QRect r = QRect(width() - w - margin, (height() - h) / 2, w, h);

  QPainter p(this);

  // drawing background
  p.setPen(Qt::NoPen);
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
  p.setBrush(QColor(0x15, 0x15, 0x15, 0xf1));
  p.drawRoundedRect(r, radius, radius);

  // drawing text
  p.setPen(Qt::white);
  p.setRenderHint(QPainter::TextAntialiasing);
  p.setFont(InterFont(74, QFont::DemiBold));
  p.drawText(r, Qt::AlignCenter, profile.text1);
}



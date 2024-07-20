#include "selfdrive/ui/qt/onroad/onroad_home.h"

#include <QPainter>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "selfdrive/ui/qt/util.h"

OnroadWindow::OnroadWindow(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout  = new QVBoxLayout(this);
  main_layout->setMargin(UI_BORDER_SIZE);
  QStackedLayout *stacked_layout = new QStackedLayout;
  stacked_layout->setStackingMode(QStackedLayout::StackAll);
  main_layout->addLayout(stacked_layout);

  nvg = new AnnotatedCameraWidget(VISION_STREAM_ROAD, this);

  QWidget * split_wrapper = new QWidget;
  split = new QHBoxLayout(split_wrapper);
  split->setContentsMargins(0, 0, 0, 0);
  split->setSpacing(0);
  split->addWidget(nvg);

  if (getenv("DUAL_CAMERA_VIEW")) {
    CameraWidget *arCam = new CameraWidget("camerad", VISION_STREAM_ROAD, true, this);
    split->insertWidget(0, arCam);
  }

  stacked_layout->addWidget(split_wrapper);

  alerts = new OnroadAlerts(this);
  alerts->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  stacked_layout->addWidget(alerts);

  profileInfo = new Profile(this);
  profileInfo->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  stacked_layout->addWidget(profileInfo);


//  // Box showing information about the current custom profile
//  profileInfoBox = new QWidget(this);
//  profileInfoBox->setStyleSheet("background-color: rgba(0, 0, 0, 50); border: 2px solid black;");
//
//  // Set geometry for bottom-right corner
//  profileInfoBox->setFixedSize(100, 50);
//
//
//  QVBoxLayout *boxLayout = new QVBoxLayout(profileInfoBox);
//  boxLayout->setContentsMargins(0, 0, 0, 0);
//  QLabel *profileInfoLabel = new QLabel("Profile Info", profileInfoBox);
//  boxLayout->addWidget(profileInfoLabel);
//
//  stacked_layout->addWidget(profileInfoBox);


  // setup stacking order
  alerts->raise();
  profileInfo->raise();
  //profileInfoBox->raise(); // place the info at the top of the stack

  setAttribute(Qt::WA_OpaquePaintEvent);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &OnroadWindow::updateState);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &OnroadWindow::offroadTransition);
}

void OnroadWindow::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);

  // update the geometry of the profile info box
  profileInfoBox->move(width() - profileInfoBox->width(), height() - profileInfoBox->height());
}


void OnroadWindow::updateState(const UIState &s) {
  if (!s.scene.started) {
    return;
  }

  profileInfo->updateState(s);
  alerts->updateState(s);
  nvg->updateState(s);

  QColor bgColor = bg_colors[s.status];
  if (bg != bgColor) {
    // repaint border
    bg = bgColor;
    update();
  }
}

void OnroadWindow::offroadTransition(bool offroad) {
  alerts->clear();
  profileInfo->clear();
}

void OnroadWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.fillRect(rect(), QColor(bg.red(), bg.green(), bg.blue(), 255));
}

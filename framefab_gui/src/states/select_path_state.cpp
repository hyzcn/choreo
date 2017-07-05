//
// Created by yijiangh on 6/28/17.
//
//#include "framefab_gui/states/scanning_state.h"

#include <ros/console.h>
#include <framefab_gui/states/select_path_state.h>
#include <framefab_gui/states/system_init_state.h>
#include <framefab_gui/states/path_planning_state.h>
#include <framefab_gui/states/process_planning_state.h>
//#include <QtConcurrent/QtConcurrentRun>

void framefab_gui::SelectPathState::onStart(FrameFabWidget& gui)
{
  gui.setText("Select Path State.\nPlease select the desired path to be planned in selection window.\nClick <Accept> to continue. ");
  gui.setButtonsEnabled(false);
  selected_path_id_ = -1;

  selectPath(gui);
}

void framefab_gui::SelectPathState::onExit(FrameFabWidget& gui) {}

void framefab_gui::SelectPathState::onNext(FrameFabWidget& gui)
{
  gui.setButtonsEnabled(true);
  selected_path_id_ = gui.select_path().getSelectedValue();
  gui.select_path().close();

  ROS_INFO_STREAM("select path state finished! Selected Path: " << selected_path_id_);
  Q_EMIT newStateAvailable(new ProcessPlanningState(selected_path_id_));
}

void framefab_gui::SelectPathState::onBack(FrameFabWidget& gui)
{
  gui.select_path().cleanUpVisual();
  Q_EMIT newStateAvailable(new SystemInitState());
}

void framefab_gui::SelectPathState::onReset(FrameFabWidget& gui)
{
  gui.select_path().cleanUpVisual();
  Q_EMIT newStateAvailable(new SystemInitState());
}

void framefab_gui::SelectPathState::selectPath(FrameFabWidget& gui)
{
  gui.select_path().show();
  gui.select_path().loadParameters();
}


//
// Created by yijiangh on 6/22/17.
//

#ifndef FRAMEFAB_GUI_SELECT_PROCESS_PLAN_STATE_H
#define FRAMEFAB_GUI_SELECT_PROCESS_PLAN_STATE_H

#include <framefab_gui/gui_state.h>
#include <framefab_gui/framefab_widget.h>
#include <ros/ros.h>

namespace framefab_gui
{

class SelectPlanState : public GuiState
{
  Q_OBJECT
 public:
  // Entry and exit classes
  virtual void onStart(FrameFabWidget& gui);
  virtual void onExit(FrameFabWidget& gui);

  // Handlers for the fixed buttons
  virtual void onNext(FrameFabWidget& gui);
  virtual void onBack(FrameFabWidget& gui);
  virtual void onReset(FrameFabWidget& gui);

 protected:
  void selectPlan(FrameFabWidget& gui);

  void simulateAll(FrameFabWidget& gui);
  void simulateOne(const int& plan_id, FrameFabWidget& gui);

 private Q_SLOTS:
  void finishSelection();

 private:
  int selected_plan_id_;
  std::vector<int> plan_ids_;
};
}

#endif //FRAMEFAB_GUI_SELECT_PROCESS_PLAN_STATE_H

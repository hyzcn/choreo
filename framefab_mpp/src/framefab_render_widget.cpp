//
// Created by yijiangh on 4/13/17.
//

#include <boost/shared_ptr.hpp>

// ROS msgs
#include <visualization_msgs/Marker.h>
#include <std_msgs/Bool.h>
#include <geometry_msgs/PoseArray.h>

//MoveIt
#include <moveit/robot_model_loader/robot_model_loader.h>
#include <moveit/planning_scene_monitor/planning_scene_monitor.h>

#include <moveit_msgs/DisplayRobotState.h>
#include <moveit_msgs/DisplayTrajectory.h>
#include <tf_conversions/tf_eigen.h>
#include <eigen_conversions/eigen_msg.h>

// Qt
#include <QtCore>
#include <QFileDialog>
#include <QString>

// framefab
#include <framefab_rviz_panel.h>
#include <util/global_functions.h>

namespace framefab
{

FrameFabRenderWidget::FrameFabRenderWidget( QWidget* parent )
    : ptr_framefab_(NULL),
      parent_(parent),
      unit_conversion_scale_factor_(1)
{
  ROS_INFO("FrameFab Render Widget started.");

  // readParameters
  readParameters();

  // TODO: does this rate belongs to a node?
  rate_ = new ros::Rate(10.0);
  //rate->sleep();

  display_pose_publisher_ = node_handle_.advertise<moveit_msgs::DisplayTrajectory>(
      "/move_group/display_planned_path", 1 ,true);

  ROS_INFO("init ptr_wf_collision");
  ptr_wire_frame_collision_objects_ = boost::make_shared<wire_frame::WireFrameCollisionObjects>();

  // TODO: move this motion topic into external ros parameters
  // should figure out a way to extract rviz nodehandle
  // the member variable now seems not to be the rviz one.
  // check:
  //  ROS_INFO(ros::this_node::getName().c_str());
  //  std::string a = node_handle_.getNamespace();
  //  ROS_INFO(a.c_str());
  std::ostringstream motion_topic;
  motion_topic << "/rviz_ubuntu_24663_1410616146488788559" << "/monitored_planning_scene";

  ptr_planning_scene_monitor_ =
      boost::make_shared<planning_scene_monitor::PlanningSceneMonitor>("robot_description");

  ptr_planning_scene_monitor_->startPublishingPlanningScene(
      planning_scene_monitor::PlanningSceneMonitor::UPDATE_SCENE, motion_topic.str());

  //planning_scene_monitor_->startSceneMonitor(motion_topic.str());
  //params

  ref_pt_x_ = 300;
  ref_pt_y_ = 0;
  ref_pt_z_ = 0;
}

FrameFabRenderWidget::~FrameFabRenderWidget()
{
  framefab::safeDelete(ptr_framefab_);
}

bool FrameFabRenderWidget::readParameters()
{
  // render widget topic name parameters
  node_handle_.param("display_pose_topic", display_pose_topic_, std::string("/framelinks"));

  // rviz render parameters
  node_handle_.param("/framefab_mpp/wire_frame_collision_start_vertex_color_r", start_color_.r, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_start_vertex_color_g", start_color_.g, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_start_vertex_color_b", start_color_.b, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_start_vertex_color_a", start_color_.a, float(0.0));

  node_handle_.param("/framefab_mpp/wire_frame_collision_end_vertex_color_r", end_color_.r, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_end_vertex_color_g", end_color_.g, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_end_vertex_color_b", end_color_.b, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_end_vertex_color_a", end_color_.a, float(0.0));

  node_handle_.param("/framefab_mpp/wire_frame_collision_cylinder_color_r", cylinder_color_.r, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_cylinder_color_g", cylinder_color_.g, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_cylinder_color_b", cylinder_color_.b, float(0.0));
  node_handle_.param("/framefab_mpp/wire_frame_collision_cylinder_color_a", cylinder_color_.a, float(0.0));

  node_handle_.param("/framefab_mpp/wire_frame_collision_cylinder_radius", collision_cylinder_radius_, float(0.0001));
  return true;
}

void FrameFabRenderWidget::displayPoses()
{
}

void FrameFabRenderWidget::advanceRobot()
{
  // init main computation class - FrameFab here
  ROS_INFO("Renderwidget: advance robot called");

  // init framefab
  safeDelete(ptr_framefab_);
  ptr_framefab_ = new FrameFab(node_handle_);

  ptr_framefab_->debug();
}

void FrameFabRenderWidget::setValue(int i)
{

}

void FrameFabRenderWidget::setScaleFactor(QString unit_info)
{
  Q_EMIT(sendLogInfo(QString("-----------MODEL UNIT-----------")));
  Q_EMIT(sendLogInfo(unit_info));

  if(QString("millimeter") == unit_info)
  {
    node_handle_.param("/framefab_mpp/unit_conversion_millimeter_to_meter", unit_conversion_scale_factor_, float(1));
  }

  if(QString("centimeter") == unit_info)
  {
    node_handle_.param("/framefab_mpp/unit_conversion_centimeter_to_meter", unit_conversion_scale_factor_, float(1));
  }

  if(QString("inch") == unit_info)
  {
    node_handle_.param("/framefab_mpp/unit_conversion_inch_to_meter", unit_conversion_scale_factor_, float(1));
  }

  if(QString("foot") == unit_info)
  {
    node_handle_.param("/framefab_mpp/unit_conversion_foot_to_meter", unit_conversion_scale_factor_, float(1));
  }

  Q_EMIT(sendLogInfo(QString("Convert to meter - scale factor %1").arg(unit_conversion_scale_factor_)));

  // if collision objects exists, rebuild
  if(0 != ptr_wire_frame_collision_objects_->sizeOfCollisionObjectsList())
  {
    ROS_INFO("rebuilding model");
    Q_EMIT(sendLogInfo(QString("Model rebuilt for update on unit.")));
    this->constructCollisionObjects();
  }
}

void FrameFabRenderWidget::setRefPoint(double ref_pt_x, double ref_pt_y, double ref_pt_z)
{
  Q_EMIT(sendLogInfo(QString("-----------REF POINT-----------")));
  Q_EMIT(sendLogInfo(QString("(%1, %2, %3)").arg(ref_pt_x).arg(ref_pt_y).arg(ref_pt_z)));

  ref_pt_x_ = ref_pt_x;
  ref_pt_y_ = ref_pt_y;
  ref_pt_z_ = ref_pt_z;

  // if collision objects exists, rebuild
  if(0 != ptr_wire_frame_collision_objects_->sizeOfCollisionObjectsList())
  {
    ROS_INFO("rebuilding model");
    Q_EMIT(sendLogInfo(QString("Model rebuilt for update on Ref Point.")));
    this->constructCollisionObjects();
  }
}

void FrameFabRenderWidget::constructCollisionObjects()
{
  //--------------- load wireframe collision objects start -----------------------
  ptr_wire_frame_collision_objects_->constructCollisionObjects(
      ptr_planning_scene_monitor_->getPlanningScene()->getPlanningFrame(),
      unit_conversion_scale_factor_, collision_cylinder_radius_,
      ref_pt_x_, ref_pt_y_, ref_pt_z_);

  wire_frame::MoveitLinearMemberCollisionObjectsListPtr ptr_collision_objects
      = ptr_wire_frame_collision_objects_->getCollisionObjectsList();

  // TODO: based on PlanningSceneMonitor API doc, shouldn't use getPlanningScene,
  // since this results in unsafe scene pointer
  moveit_msgs::PlanningScene scene;
  ptr_planning_scene_monitor_->getPlanningScene()->getPlanningSceneMsg(scene);
  planning_scene::PlanningScenePtr ptr_current_scene = ptr_planning_scene_monitor_->getPlanningScene();

  ptr_current_scene->removeAllCollisionObjects();

  // TODO: planning scene not cleaned up in second time input case, ref issue #21
  for (int i=0; i < ptr_collision_objects->size(); i++)
  {
    scene.world.collision_objects.push_back((*ptr_collision_objects)[i]->start_vertex_collision);
    scene.world.collision_objects.push_back((*ptr_collision_objects)[i]->edge_cylinder_collision);
    scene.world.collision_objects.push_back((*ptr_collision_objects)[i]->end_vertex_collision);

    ptr_current_scene->setObjectColor((*ptr_collision_objects)[i]->edge_cylinder_collision.id.c_str(), cylinder_color_);
    ptr_current_scene->setObjectColor((*ptr_collision_objects)[i]->start_vertex_collision.id.c_str(), start_color_);
    ptr_current_scene->setObjectColor((*ptr_collision_objects)[i]->end_vertex_collision.id.c_str(), end_color_);
  }
  //--------------- load wireframe collision objects end -----------------------

  scene.is_diff = 1;
  ptr_planning_scene_monitor_->newPlanningSceneMessage(scene);

  rate_->sleep();
}

void FrameFabRenderWidget::readFile()
{
  QString filename = QFileDialog::getOpenFileName(
      this,
      tr("Open File"),
      "$HOME/Documents",
      tr("pwf Files (*.pwf)"));

  if(filename.isEmpty())
  {
    ROS_ERROR("Read Model Failed!");
    return;
  }
  else
  {
    ROS_INFO("Open Model: success.");
  }

  // compatible with paths in Chinese
  QTextCodec *code = QTextCodec::codecForName("gd18030");
  QTextCodec::setCodecForLocale(code);
  QByteArray byfilename = filename.toLocal8Bit();

  ptr_wire_frame_collision_objects_.reset();
  ptr_wire_frame_collision_objects_ = boost::make_shared<wire_frame::WireFrameCollisionObjects>();

  //--------------- load wireframe linegraph end -----------------------
  if (filename.contains(".obj") || filename.contains(".OBJ"))
  {
    ptr_wire_frame_collision_objects_->LoadFromOBJ(byfilename.data());
  }
  else
  {
    ptr_wire_frame_collision_objects_->LoadFromPWF(byfilename.data());
  }

  if (0 == ptr_wire_frame_collision_objects_->SizeOfVertList())
  {
    Q_EMIT(sendLogInfo(QString("Input frame empty, no links to draw.")));
    return;
  }

  ROS_INFO("LineGraph Built: success.");
  //--------------- load wireframe linegraph end -----------------------

  this->constructCollisionObjects();

  QString parse_msg =
            "Nodes: "     + QString::number(ptr_wire_frame_collision_objects_->SizeOfVertList()) + "\n"
          + "Links: "    + QString::number(ptr_wire_frame_collision_objects_->SizeOfEdgeList()) + "\n"
          + "Pillars: "  + QString::number(ptr_wire_frame_collision_objects_->SizeOfPillar()) + "\n"
          + "Ceilings: " + QString::number(ptr_wire_frame_collision_objects_->SizeOfCeiling());

  Q_EMIT(sendLogInfo(QString("-----------MODEL INPUT-----------")));
  Q_EMIT(sendLogInfo(parse_msg));
  Q_EMIT(sendLogInfo(QString("factor scale: %1").arg(unit_conversion_scale_factor_)));

  ROS_INFO("model loaded successfully");
}

} /* namespace framefab */
#include <ros/ros.h>
#include <ros/console.h>

#include <boost/tuple/tuple.hpp>

#include <framefab_msgs/TaskSequenceProcessing.h>
#include <framefab_task_sequence_processor/unit_process.h>
#include <framefab_task_sequence_processor/task_sequence_processor.h>

bool processTaskSequenceCallback(framefab_msgs::TaskSequenceProcessingRequest& req,
             framefab_msgs::TaskSequenceProcessingResponse& res)
{
  framefab_task_sequence_processing::TaskSequenceProcessor ts_processor;
  ts_processor.setParams(req.model_params, req.task_sequence_params);

  switch (req.action)
  {
    case framefab_msgs::TaskSequenceProcessing::Request::PROCESS_TASK_AND_MARKER:
    {
      if(!(ts_processor.createCandidatePoses() && ts_processor.createEnvCollisionObjs()))
      {
        ROS_ERROR("[Task Seq Process] Could not process input task sequence!");
        res.succeeded = false;
        return false;
      }
      break;
    }

    default:
    {
      ROS_ERROR("[Task Seq Process] Unrecognized task sequence processing request");
      res.succeeded = false;
      return false;
    }
  }

  std::vector<framefab_task_sequence_processing_utils::UnitProcess> process_array =
      ts_processor.getCandidatePoses();

  std::vector<moveit_msgs::CollisionObject> env_objs =
      ts_processor.getEnvCollisionObjs();

  for(auto unit_process : process_array)
  {
    res.process.push_back(unit_process.asElementCandidatePoses());
  }

  for(auto env_obj : env_objs)
  {
    res.env_collision_objs.push_back(env_obj);
  }

  res.succeeded = true;
  return true;
}

int main(int argc, char** argv)
{

  ros::init(argc, argv, "task_sequence_processor");
  ros::NodeHandle nh;

  ros::ServiceServer task_sequence_processing_server =
      nh.advertiseService<framefab_msgs::TaskSequenceProcessingRequest, framefab_msgs::TaskSequenceProcessingResponse>(
          "task_sequence_processing", boost::bind(processTaskSequenceCallback, _1, _2));

  ROS_INFO("[task seq processor] %s server ready to service requests.", task_sequence_processing_server.getService().c_str());
  ros::spin();

  return 0;
}

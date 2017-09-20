#include "ros/ros.h"
#include "std_msgs/String.h"
#include "dobot/SetCmdTimeout.h"
#include "dobot/SetQueuedCmdClear.h"
#include "dobot/SetQueuedCmdStartExec.h"
#include "dobot/SetQueuedCmdForceStopExec.h"
#include "dobot/GetDeviceVersion.h"

#include "dobot/SetEndEffectorParams.h"
#include "dobot/SetEndEffectorSuctionCup.h"
#include "dobot/SetPTPJointParams.h"
#include "dobot/SetPTPCoordinateParams.h"
#include "dobot/SetPTPJumpParams.h"
#include "dobot/SetPTPCommonParams.h"
#include "dobot/SetPTPCmd.h"

#include "dobot/MoveArm.h"

static ros::NodeHandle *m_pNodeHandle = NULL;

bool moveArm(dobot::MoveArm::Request &req, dobot::MoveArm::Response &res){
    ROS_INFO("moveArm() is called. src_x=%f, src_y=%f, dst_x=%f, dst_y=%f", req.src_x, req.src_y, req.dst_x, req.dst_y);

    ROS_INFO("moveArm() : m_pNodeHandle = %lx", m_pNodeHandle);
    ros::ServiceClient client;

    // Set end effector parameters for turning it on
    dobot::SetEndEffectorSuctionCup srvawem;
    srvawem.request.enableCtrl = true;
    srvawem.request.suck = true;
    // srvawem.request.isQueued = false;
    // srvawem.request.queuedCmdIndex = null;
    client = m_pNodeHandle->serviceClient<dobot::SetEndEffectorSuctionCup>("/DobotServer/SetEndEffectorSuctionCup");
    client.call(srvawem);
    ROS_INFO("SetEndEffecotrSuctionCup1 = %d", srvawem.response.result);

    client = m_pNodeHandle->serviceClient<dobot::SetPTPCmd>("/DobotServer/SetPTPCmd");
    dobot::SetPTPCmd srv;
    bool result = true;

    // The first point ( source positon )
    do {
        srv.request.ptpMode = 1;
        srv.request.x = req.src_x;
        srv.request.y = req.src_y;
        srv.request.z = 0;
        srv.request.r = 0;
        client.call(srv);
        ROS_INFO("SetPTPCmd1:result = %d", srv.response.result);
        if (srv.response.result == 0) {
            result = true;
            break;
        }     
        ros::spinOnce();
        if (ros::ok() == false) {
            result = false;
            break;
        }
    } while (1);


    // The second point ( destination position )
    do {
        srv.request.ptpMode = 1;
        srv.request.x = req.dst_x;
        srv.request.y = req.dst_y;
        srv.request.z = 0;
        srv.request.r = 0;
        client.call(srv);
        ROS_INFO("SetPTPCmd2:result = %d", srv.response.result);
        if (srv.response.result == 0) {
            result = true;
            break;
        }
        ros::spinOnce();
        if (ros::ok() == false) {
            result = false;
            break;
        }
    } while (1);
    // ros::spinOnce();

    // Set end effector parameters for turning it off 
    srvawem.request.enableCtrl = false;
    srvawem.request.suck = false;

    client = m_pNodeHandle->serviceClient<dobot::SetEndEffectorSuctionCup>("/DobotServer/SetEndEffectorSuctionCup");
    client.call(srvawem);
    ROS_INFO("SetEndEffecotrSuctionCup2 = %d", srvawem.response.result);

    ROS_INFO("result bool = %d", result );
    res.result = result;
    return true;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "DobotClient");
    ros::NodeHandle n;
    m_pNodeHandle = &n; //YSK

    ROS_INFO("main() : n = %lx, m_pNodeHandle = %lx", &n, m_pNodeHandle);

    ros::ServiceClient client;

    // SetCmdTimeout
    client = n.serviceClient<dobot::SetCmdTimeout>("/DobotServer/SetCmdTimeout");
    dobot::SetCmdTimeout srv1;
    srv1.request.timeout = 3000;
#if 0
// YSK
    if (client.call(srv1) == false) {
        ROS_ERROR("Failed to call SetCmdTimeout. Maybe DobotServer isn't started yet!");
        return -1;
    }
#else
    ROS_INFO("[YKS] This is debug mode. Please comment-in a SetCmdTimeout check.");
#endif

    // Clear the command queue
    client = n.serviceClient<dobot::SetQueuedCmdClear>("/DobotServer/SetQueuedCmdClear");
    dobot::SetQueuedCmdClear srv2;
    client.call(srv2);

    // Start running the command queue
    client = n.serviceClient<dobot::SetQueuedCmdStartExec>("/DobotServer/SetQueuedCmdStartExec");
    dobot::SetQueuedCmdStartExec srv3;
    client.call(srv3);

    // Get device version information
    client = n.serviceClient<dobot::GetDeviceVersion>("/DobotServer/GetDeviceVersion");
    dobot::GetDeviceVersion srv4;
    client.call(srv4);
    if (srv4.response.result == 0) {
        ROS_INFO("Device version:%d.%d.%d", srv4.response.majorVersion, srv4.response.minorVersion, srv4.response.revision);
    } else {
        ROS_ERROR("Failed to get device version information!");
    }

    // Set end effector parameters
    client = n.serviceClient<dobot::SetEndEffectorParams>("/DobotServer/SetEndEffectorParams");
    dobot::SetEndEffectorParams srv5;
    srv5.request.xBias = 70;
    srv5.request.yBias = 0;
    srv5.request.zBias = 0;
    client.call(srv5);

#if 0
// YSK
    // Set end effector parameters AWEM
    client = n.serviceClient<dobot::SetEndEffectorSuctionCup>("/DobotServer/SetEndEffectorSuctionCup");
    dobot::SetEndEffectorSuctionCup srvawem;
    srvawem.request.enableCtrl = false;
    srvawem.request.suck = false;
    // srvawem.request.isQueued = false;
    // srvawem.request.queuedCmdIndex = null;
    client.call(srvawem);
#endif

    // Set PTP joint parameters
    do {
        client = n.serviceClient<dobot::SetPTPJointParams>("/DobotServer/SetPTPJointParams");
        dobot::SetPTPJointParams srv;

        for (int i = 0; i < 4; i++) {
            srv.request.velocity.push_back(100);
        }
        for (int i = 0; i < 4; i++) {
            srv.request.acceleration.push_back(100);
        }
        client.call(srv);
    } while (0);

    // Set PTP coordinate parameters
    do {
        client = n.serviceClient<dobot::SetPTPCoordinateParams>("/DobotServer/SetPTPCoordinateParams");
        dobot::SetPTPCoordinateParams srv;

        srv.request.xyzVelocity = 100;
        srv.request.xyzAcceleration = 100;
        srv.request.rVelocity = 100;
        srv.request.rAcceleration = 100;
        client.call(srv);
    } while (0);

    // Set PTP jump parameters
    do {
        client = n.serviceClient<dobot::SetPTPJumpParams>("/DobotServer/SetPTPJumpParams");
        dobot::SetPTPJumpParams srv;

        srv.request.jumpHeight = 20;
        srv.request.zLimit = 200;
        client.call(srv);
    } while (0);

    // Set PTP common parameters
    do {
        client = n.serviceClient<dobot::SetPTPCommonParams>("/DobotServer/SetPTPCommonParams");
        dobot::SetPTPCommonParams srv;

        srv.request.velocityRatio = 50;
        srv.request.accelerationRatio = 50;
        client.call(srv);
    } while (0);

#if 0
// YSK
    client = n.serviceClient<dobot::SetPTPCmd>("/DobotServer/SetPTPCmd");
    dobot::SetPTPCmd srv;

    while (ros::ok()) { // YSK
        ROS_INFO("hello");
        // The first point
        do {
            srv.request.ptpMode = 1;
            srv.request.x = 200;
            srv.request.y = 200;
            srv.request.z = 0;
            srv.request.r = 0;
            client.call(srv);
            if (srv.response.result == 0) {
                break;
            }     
            ros::spinOnce();
            if (ros::ok() == false) {
                break;
            }
        } while (1);


        // The first point
        do {
            srv.request.ptpMode = 1;
            srv.request.x = 250;
            srv.request.y = 0;
            srv.request.z = 0;
            srv.request.r = 0;
            client.call(srv);
            if (srv.response.result == 0) {
                break;
            }
            ros::spinOnce();
            if (ros::ok() == false) {
                break;
            }
        } while (1);
  
        ros::spinOnce();
    } // YSK
#else

    ros::ServiceServer serviceServer = n.advertiseService("MoveArm", moveArm );
    ros::spin();
    
#endif

    return 0;
}


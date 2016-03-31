#ifndef _DEVICE_TOPOLOGY_H
#define _DEVICE_TOPOLOGY_H
#include "ns3/core-module.h"
#include "ns3/propagation-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

// Ad-Hoc by default
class DeviceTopology{
public:
    DeviceTopology();
    virtual ~DeviceTopology(){};
    NetDeviceContainer Create(NodeContainer& nodes, int n = 3);
    NetDeviceContainer CreateInfra(NodeContainer& nodes, int n = 3);
private:
    int ssid_suffix;
};
#endif

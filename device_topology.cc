#include "device_topology.h"

using namespace ns3;

DeviceTopology::DeviceTopology(){
    ssid_suffix = 1;
}



NetDeviceContainer DeviceTopology::Create(NodeContainer& nodes, int n){
    nodes.Create (n);

    // 2. Place nodes somehow, this is required by every wireless simulation
    for (int i = 0; i < n; ++i)
    {
      nodes.Get (i)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
    }

    // 3. Create propagation loss matrix
    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
    lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
    for (int i=1; i < n; i++){
//	for (int j = i + 1; j < n; j++)
//	if (i==1) continue;
	lossModel->SetLoss (nodes.Get (0)->GetObject<MobilityModel>(), nodes.Get (i)->GetObject<MobilityModel>(), 50); // set symmetric loss 0 <-> 1 to 50 dB
    }
    // 4. Create & setup wifi channel
    Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
    wifiChannel->SetPropagationLossModel (lossModel);
    wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

    // 5. Install wireless devices
    WifiHelper wifi;
    
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", 
				"DataMode",StringValue ("DsssRate2Mbps"), 
				"ControlMode",StringValue ("DsssRate1Mbps"));
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel);
    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);
    return devices;
}

NetDeviceContainer DeviceTopology::CreateInfra(NodeContainer& nodes, int n){

    nodes.Create (n);

    // 2. Place nodes somehow, this is required by every wireless simulation
    for (int i = 0; i < n; ++i)
    {
	fprintf(stderr, "%d\n",i);
      nodes.Get (i)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
    }

    // 3. Create propagation loss matrix
    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
    lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
    for (int i=1; i < n ; i++){
//	for (int j = i + 1; j < n; j++)
//	if (i==1) continue;
	lossModel->SetLoss (nodes.Get (0)->GetObject<MobilityModel>(), nodes.Get (i)->GetObject<MobilityModel>(), 50); // set symmetric loss 0 <-> 1 to 50 dB
    }
    // 4. Create & setup wifi channel
    Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
    wifiChannel->SetPropagationLossModel (lossModel);
    wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

    // 5. Install wireless devices
    WifiHelper wifi;
    std::string ssidString ("infrastructure-network-");
    std::stringstream ss;
    ss << ssid_suffix++;
    ssidString += ss.str ();
    Ssid ssid = Ssid (ssidString);
    
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
				"DataMode",StringValue ("DsssRate2Mbps"), 
				"ControlMode",StringValue ("DsssRate1Mbps"));

    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel);
// pcapit
//    wifiPhy.EnablePcap ("rtscts-pcap-node" , nodes);
    NodeContainer sta_nodes ( nodes.Get(1), nodes.Get(2));
    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::StaWifiMac",
		    "Ssid", SsidValue (ssid),
		    "ActiveProbing", BooleanValue (false)
		    );

    NetDeviceContainer staDevices = wifi.Install (wifiPhy, wifiMac, sta_nodes);
    
    wifiMac.SetType ("ns3::ApWifiMac",
		    "Ssid", SsidValue (ssid),
		    "BeaconGeneration", BooleanValue (true),
		    "BeaconInterval", TimeValue(Seconds(2.5)));

    NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, nodes.Get(0));
    NetDeviceContainer devices ( apDevice, staDevices);
    return devices;

//    Ssid ssid = Ssid (ssidString);
//    wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
//    // setup stas
//    macInfra.SetType ("ns3::StaWifiMac",
//		    "Ssid", SsidValue (ssid),
//		    "ActiveProbing", BooleanValue (false));
//    NetDeviceContainer staDevices = wifi.Install (wifiChannel, macInfra, stas);
//    // setup ap.
//    macInfra.SetType ("ns3::ApWifiMac",
//		    "Ssid", SsidValue (ssid),
//		    "BeaconGeneration", BooleanValue (true),
//		    "BeaconInterval", TimeValue(Seconds(2.5)));
//    NetDeviceContainer apDevices = wifi.Install (wifiChannel, macInfra, backbone.Get (i));
//
//    wifiPhy.SetChannel (wifiChannel);
//    WifiMacHelper wifiMac;
//    wifiMac.SetType ("ns3::AdhocWifiMac"); // use ad-hoc MAC
//    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);
//    return devices;

}
std::vector<NetDeviceContainer> DeviceTopology::CreateInfra(
	std::vector<NodeContainer*> node_vector, 
	std::vector<int> nodes_count,
	int neigh_matrix[N_NODES][N_NODES])
{
    for (int i = 0; i < node_vector.size(); i++) {
    	node_vector[i]->Create(nodes_count[i]);

	for (int j = 0; j < nodes_count[i]; ++j)
	{
	    fprintf(stderr, "%d\n",i);
	  node_vector[i]->Get (j)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
	}

    }
    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
    lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
    int nn = NodeList::GetNNodes();
    for (int i = 0; i < nn; ++i)
	for (int j = i; j < nn; j ++)
	    if ( neigh_matrix[i][j] != 0){
		lossModel->SetLoss (NodeList::GetNode(i)->GetObject<MobilityModel>(), NodeList::GetNode(j)->GetObject<MobilityModel>(), 50);
	    }

    Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
    wifiChannel->SetPropagationLossModel (lossModel);
    wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

    std::vector<NetDeviceContainer> devices_v = {};
    for (int i = 0; i < node_vector.size(); i++) {
	NodeContainer& nodes = *node_vector[i];
	WifiHelper wifi;
	std::string ssidString ("infrastructure-network-");
	std::stringstream ss;
	ss << ssid_suffix++;
	ssidString += ss.str ();
	Ssid ssid = Ssid (ssidString);
	
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
				    "DataMode",StringValue ("DsssRate2Mbps"), 
				    "ControlMode",StringValue ("DsssRate1Mbps"));

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	wifiPhy.SetChannel (wifiChannel);

    //    wifiPhy.EnablePcap ("rtscts-pcap-node" , nodes);

	NodeContainer sta_nodes ( nodes.Get(1), nodes.Get(2));
	WifiMacHelper wifiMac;
	wifiMac.SetType ("ns3::StaWifiMac",
			"Ssid", SsidValue (ssid),
			"ActiveProbing", BooleanValue (false)
			);

	NetDeviceContainer staDevices = wifi.Install (wifiPhy, wifiMac, sta_nodes);
	
	wifiMac.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid),
			"BeaconGeneration", BooleanValue (true),
			"BeaconInterval", TimeValue(Seconds(2.5)));

	NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, nodes.Get(0));
	NetDeviceContainer devices ( apDevice, staDevices);

	devices_v.push_back(devices);
    }
    return devices_v;

}

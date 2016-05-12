/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */
#include "ns3/core-module.h"
#include "ns3/propagation-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include <vector>

using namespace ns3;
using namespace std;
/// Run single 10 seconds experiment with enabled or disabled RTS/CTS mechanism
void experiment (bool enableCtsRts)
{
    const int nodes_total = 9;
    vector<int> nodes_count = {3, 3, 3};
    int neigh_matrix[nodes_total][nodes_total] =
    // should be triangular matrix with 1 on diagonal, values below diagonal will be ignored, represented in 1D so it is easier to pass
    { 
    { 1, 1, 1, 1, 1, 1, 0, 0, 0}, 
    { 0, 1, 1, 1, 1, 1, 0, 0, 0}, 
    { 0, 0, 1, 1, 1, 1, 0, 0, 0},
    { 0, 0, 0, 1, 1, 1, 1, 1, 1},
    { 0, 0, 0, 0, 1, 1, 1, 1, 1},
    { 0, 0, 0, 0, 0, 1, 1, 1, 1},
    { 0, 0, 0, 0, 0, 0, 1, 1, 1},
    { 0, 0, 0, 0, 0, 0, 0, 1, 1},
    { 0, 0, 0, 0, 0, 0, 0, 0, 1},
    };
    const int start_time = 1;
    const int end_time = 101;
    const int total_time = end_time - start_time;
    // 0. Enable or disable CTS/RTS
    UintegerValue ctsThr = (enableCtsRts ? UintegerValue (10) : UintegerValue (2500));
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

    NodeContainer nodes1, nodes2, nodes3;
    std::vector<NodeContainer*> node_vector;
// it means there should be 3 networks with 3 nodes each
    node_vector = { &nodes1, &nodes2, &nodes3 };
    int ssid_suffix = 1;
    for (int i = 0; i < node_vector.size(); i++) {
    	node_vector[i]->Create(nodes_count[i]);

	for (int j = 0; j < nodes_count[i]; ++j)
	{
	  node_vector[i]->Get (j)->AggregateObject (CreateObject<ConstantPositionMobilityModel> ());
	}

    }
    Ptr<MatrixPropagationLossModel> lossModel = CreateObject<MatrixPropagationLossModel> ();
    lossModel->SetDefaultLoss (200); // set default loss to 200 dB (no link)
    int nn = NodeList::GetNNodes();
//    cout << nn << endl;
    for (int i = 0; i < nn; ++i){
	for (int j = i; j < nn; ++j){
	    if ( neigh_matrix[i][j] != 0){
		lossModel->SetLoss (NodeList::GetNode(i)->GetObject<MobilityModel>(),
				    NodeList::GetNode(j)->GetObject<MobilityModel>(), 50);
	    }
	}
    }

    Ptr<YansWifiChannel> wifiChannel = CreateObject <YansWifiChannel> ();
    wifiChannel->SetPropagationLossModel (lossModel);
    wifiChannel->SetPropagationDelayModel (CreateObject <ConstantSpeedPropagationDelayModel> ());

    std::vector<NetDeviceContainer> devices_v = {};
    for (int i = 0; i < node_vector.size(); i++) {
	NodeContainer* nodes = node_vector[i];
	WifiHelper wifi;
	std::string ssidString ("infrastructure-network-");
	std::stringstream ss;
	ss << ssid_suffix++;
	ssidString += ss.str ();
	Ssid ssid = Ssid (ssidString);
        bool run_ac = false; // dont change, doesnt work lul
	if (run_ac) {
                wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
              wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue ("VhtMcs9"),
                                    "ControlMode", StringValue ("VhtMcs0")
            );
        } else {

            wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
              wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue ("DsssRate2Mbps"),
                                    "ControlMode", StringValue ("DsssRate1Mbps")
            );

        }
	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	wifiPhy.SetChannel (wifiChannel);

    //    wifiPhy.EnablePcap ("rtscts-pcap-node" , nodes);
        if (run_ac){
            wifiPhy.Set ("ShortGuardEnabled", BooleanValue(true));
            wifiPhy.Set ("TxAntennas", UintegerValue (2));
            wifiPhy.Set ("RxAntennas", UintegerValue (2));
            wifiPhy.Set ("GreenfieldEnabled",BooleanValue(true));
        }
	NodeContainer sta_nodes;
	for (int j = 1; j < nodes->GetN(); j ++){
	    sta_nodes.Add(nodes->Get(j));
	}
	WifiMacHelper wifiMac;
        wifiMac.SetType ("ns3::StaWifiMac",
                      "Ssid", SsidValue (ssid),
                      "ActiveProbing", BooleanValue (false));
	NetDeviceContainer staDevices = wifi.Install (wifiPhy, wifiMac, sta_nodes);

	wifiMac.SetType ("ns3::ApWifiMac",
			"Ssid", SsidValue (ssid),
			"BeaconGeneration", BooleanValue (true),
			"BeaconInterval", TimeValue(Seconds(2.5)));

	NetDeviceContainer apDevice = wifi.Install (wifiPhy, wifiMac, nodes->Get(0));
	NetDeviceContainer devices ( apDevice, staDevices);

	devices_v.push_back(devices);
    }
    for (int i = 0; i < node_vector.size(); i++) {
    
	NetDeviceContainer devices = devices_v[i];
	NodeContainer* nodes = node_vector[i];
	// uncomment the following to have athstats output
	// AthstatsHelper athstats;
	// athstats.EnableAthstats(enableCtsRts ? "rtscts-athstats-node" : "basic-athstats-node" , nodes);

	// 6. Install TCP/IP stack & assign IP addresses
	InternetStackHelper internet;
	internet.Install (*nodes);
	Ipv4AddressHelper ipv4;
	char addr0[50];
	char addr1[50];
	sprintf(addr0,"10.0.%d.0",i);
	sprintf(addr1,"10.0.%d.1",i);
	ipv4.SetBase (addr0, "255.255.255.0");
	ipv4.Assign (devices);

	// 7. Install applications	
        ApplicationContainer cbrApps;
	uint16_t cbrPort = 12345;

	OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address (addr1), cbrPort));
	onOffHelper.SetAttribute ("PacketSize", UintegerValue (1400));
	onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
	onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
//	onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::NormalRandomVariable[Mean=0.5|Variance=0.3|Bound=0.1]"));
//	onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::NormalRandomVariable[Mean=0.5|Variance=0.3|Bound=0.1]"));
//	onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ExponentialRandomVariable[Mean=0.04]"));
//	onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.04]"));

	// flow 1:  node 0 -> node 1
        char data_rate1[50];
        char data_rate2[50];
	sprintf(data_rate1,"5%d000bps",2*i);
	sprintf(data_rate2,"5%d0%d0bps",2*i+1,2*i+1);
	onOffHelper.SetAttribute ("DataRate", StringValue (data_rate1));
	onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.0+i)));
	cbrApps.Add (onOffHelper.Install (nodes->Get (1))); 

	// flow 2:  node 2 -> node 1
	/** \internal
	* The slightly different start times and data rates are a workaround
	* for \bugid{388} and \bugid{912}
	*/
	onOffHelper.SetAttribute ("DataRate", StringValue (data_rate2));
	onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.1+i)));
	cbrApps.Add (onOffHelper.Install (nodes->Get (2))); 

	/** \internal
	* We also use separate UDP applications that will send a single
	* packet before the CBR flows start. 
	* This is a workaround for the lack of perfect ARP, see \bugid{187}
	*/
	uint16_t  echoPort = 9;
	UdpEchoClientHelper echoClientHelper (Ipv4Address (addr1), echoPort);
	echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
	echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10));
	ApplicationContainer pingApps;

	// again using different start times to workaround Bug 388 and Bug 912
	echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (0.01+0.1*i)));
	pingApps.Add (echoClientHelper.Install (nodes->Get (1))); 
	echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (0.06+0.1*i)));
	pingApps.Add (echoClientHelper.Install (nodes->Get (2)));
    }
    // 8. Install FlowMonitor on all nodes
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // 9. Run simulation for 10 seconds
    Simulator::Stop (Seconds (end_time));
    Simulator::Run ();

    // 10. Print per flow statistics
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // first 6 FlowIds are for ECHO apps, we don't want to display them
      if (i->first >6)
	{
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	  std::cout << "Flow " << i->first - 6 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
//	  std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
//	  std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
	  std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / total_time / 1000 / 1000  << " Mbps\n";
//	  std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
//	  std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / total_time / 1000 / 1000  << " Mbps\n";
	}
    }
    if (enableCtsRts)
        monitor->SerializeToXmlFile("Stats_enabled.xml",true,true);
    else
        monitor->SerializeToXmlFile("Stats_disabled.xml",true,true);
        
    // 11. Cleanup
    Simulator::Destroy ();
}

int main (int argc, char **argv)
{
    std::cout << "Hidden station experiment with RTS/CTS disabled:\n" << std::flush;
    experiment (false);
    std::cout << "------------------------------------------------\n";
    std::cout << "Hidden station experiment with RTS/CTS enabled:\n";
    experiment (true);

    return 0;
}

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

/*
 * Classical hidden terminal problem and its RTS/CTS solution.
 *
 * Topology: [node 0] <-- -50 dB --> [node 1] <-- -50 dB --> [node 2]
 * 
 * This example illustrates the use of 
 *  - Wifi in ad-hoc mode
 *  - Matrix propagation loss model
 *  - Use of OnOffApplication to generate CBR stream 
 *  - IP flow monitor
 */
#include "ns3/core-module.h"
#include "ns3/propagation-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/wifi-module.h"
#include "device_topology.h"

using namespace ns3;
/// Run single 10 seconds experiment with enabled or disabled RTS/CTS mechanism
void experiment (bool enableCtsRts)
{
/*
#define N_NODES 9
    int neigh_matrix[N_NODES][N_NODES] = 
// should be triangular matrix with 1 on diagonal, values below diagonal will be ignored
// TODO: fully implement it
{ 
{ 1, 1, 1, 1, 1, 1, 0, 0, 0}, // first 3 nodes dont see last 3
{ 0, 1, 1, 1, 1, 1, 0, 0, 0},
{ 0, 0, 1, 1, 1, 1, 0, 0, 0},
{ 0, 0, 0, 1, 1, 1, 1, 1, 1},
{ 0, 0, 0, 0, 1, 1, 1, 1, 1},
{ 0, 0, 0, 0, 0, 1, 1, 1, 1},
{ 0, 0, 0, 0, 0, 0, 1, 1, 1},
{ 0, 0, 0, 0, 0, 0, 0, 1, 1},
{ 0, 0, 0, 0, 0, 0, 0, 0, 1},
};

*/
    // 0. Enable or disable CTS/RTS
    UintegerValue ctsThr = (enableCtsRts ? UintegerValue (100) : UintegerValue (2200));
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

    NodeContainer nodes;
    DeviceTopology topo = DeviceTopology();
    // creates one wifi network with infrastructure (first node is ap)
    NetDeviceContainer devices = topo.CreateInfra(nodes, 3);
    // would create ad-hoc network with 3 nodes
    //NetDeviceContainer devices = topo.Create(nodes, 3);
    /*
	TODO:
	make topology and flows configurable more easily
	until 8. it should be replaced with something like this:
	{devices1,devices2,devices3} = topo.createinfra({(nodes1),(nodes2),(nodes3)}, neigh_matrix)
	
	for d in range(0,3):
	    assignIP (10.0.d.0,devices(d))
	    Install_cbr_and_echo(nodes(3*d + 1),nodes(3*d),300xMBps, 1400)
	    Install_cbr_and_echo(nodes(3*d + 2),nodes(3*d),300xMBps, 1400)
		
    */

    // uncomment the following to have athstats output
    // AthstatsHelper athstats;
    // athstats.EnableAthstats(enableCtsRts ? "rtscts-athstats-node" : "basic-athstats-node" , nodes);

    // 6. Install TCP/IP stack & assign IP addresses
    InternetStackHelper internet;
    internet.Install (nodes);
    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.0.0.0", "255.255.255.0");
    ipv4.Assign (devices);

    // 7. Install applications: two CBR streams each saturating the channel 
    ApplicationContainer cbrApps;
    uint16_t cbrPort = 12345;
    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address ("10.0.0.1"), cbrPort));
    onOffHelper.SetAttribute ("PacketSize", UintegerValue (1400));
    onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    // flow 1:  node 0 -> node 1
    onOffHelper.SetAttribute ("DataRate", StringValue ("3000000bps"));
    onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.000000)));
    cbrApps.Add (onOffHelper.Install (nodes.Get (1))); 

    // flow 2:  node 2 -> node 1
    /** \internal
    * The slightly different start times and data rates are a workaround
    * for \bugid{388} and \bugid{912}
    */
    onOffHelper.SetAttribute ("DataRate", StringValue ("3001100bps"));
    onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (1.001)));
    cbrApps.Add (onOffHelper.Install (nodes.Get (2))); 

    /** \internal
    * We also use separate UDP applications that will send a single
    * packet before the CBR flows start. 
    * This is a workaround for the lack of perfect ARP, see \bugid{187}
    */
    uint16_t  echoPort = 9;
    UdpEchoClientHelper echoClientHelper (Ipv4Address ("10.0.0.1"), echoPort);
    echoClientHelper.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
    echoClientHelper.SetAttribute ("PacketSize", UintegerValue (10));
    ApplicationContainer pingApps;

    // again using different start times to workaround Bug 388 and Bug 912
    echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (0.001)));
    pingApps.Add (echoClientHelper.Install (nodes.Get (1))); 
    echoClientHelper.SetAttribute ("StartTime", TimeValue (Seconds (0.006)));
    pingApps.Add (echoClientHelper.Install (nodes.Get (2)));

    // 8. Install FlowMonitor on all nodes
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    // 9. Run simulation for 10 seconds
    Simulator::Stop (Seconds (10));
    Simulator::Run ();

    // 10. Print per flow statistics
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // first 2 FlowIds are for ECHO apps, we don't want to display them
      //
      // Duration for throughput measurement is 9.0 seconds, since 
      //   StartTime of the OnOffApplication is at about "second 1"
      // and 
      //   Simulator::Stops at "second 10".
      if (i->first >2)
	{
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
	  std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
	  std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
	  std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
	  std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
	  std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
	  std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 9.0 / 1000 / 1000  << " Mbps\n";
	}
    }

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

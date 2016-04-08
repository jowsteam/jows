# jows #
Hidden terminal problem and its QoS impact

### Quick Start: ###

 * Download and build newest ns3-allinone
~~~~
wget https://www.nsnam.org/release/ns-allinone-3.25.tar.bz2
tar xjf ns-allinone-3.25.tar.bz2
cd ns-allinone-3.25/ns-3.25
CXXFLAGS="-std=c++11" ./waf configure
~~~~
 * Copy this repo into scratch folder and run
~~~~
git clone --branch V1.1 https://github.com/jowsteam/jows.git scratch/jows
./waf --run jows
~~~~


### First results: ###
 * Network not saturated: *
~~~~
Hidden station experiment with RTS/CTS disabled:
Flow 1 (10.0.0.2 -> 10.0.0.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.203728 Mbps
Flow 2 (10.0.0.3 -> 10.0.0.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.20487 Mbps
Flow 3 (10.0.1.2 -> 10.0.1.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.0746368 Mbps
Flow 4 (10.0.1.3 -> 10.0.1.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.0750176 Mbps
Flow 5 (10.0.2.2 -> 10.0.2.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.159555 Mbps
Flow 6 (10.0.2.3 -> 10.0.2.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.160698 Mbps
------------------------------------------------
Hidden station experiment with RTS/CTS enabled:
Flow 1 (10.0.0.2 -> 10.0.0.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.203728 Mbps
Flow 2 (10.0.0.3 -> 10.0.0.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.20487 Mbps
Flow 3 (10.0.1.2 -> 10.0.1.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.159555 Mbps
Flow 4 (10.0.1.3 -> 10.0.1.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.160698 Mbps
Flow 5 (10.0.2.2 -> 10.0.2.1)
  TxOffered:  0.203728 Mbps
  Throughput: 0.159555 Mbps
Flow 6 (10.0.2.3 -> 10.0.2.1)
  TxOffered:  0.20487 Mbps
  Throughput: 0.160698 Mbps
~~~~
 
* Network saturated:
~~~~
Hidden station experiment with RTS/CTS disabled:
Flow 1 (10.0.0.2 -> 10.0.0.1)
  TxOffered:  2.03995 Mbps
  Throughput: 0.755507 Mbps
Flow 2 (10.0.0.3 -> 10.0.0.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0.743702 Mbps
Flow 3 (10.0.1.2 -> 10.0.1.1)
  TxOffered:  2.03956 Mbps
  Throughput: 0 Mbps
Flow 4 (10.0.1.3 -> 10.0.1.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0.0102816 Mbps
Flow 5 (10.0.2.2 -> 10.0.2.1)
  TxOffered:  2.03956 Mbps
  Throughput: 0.706765 Mbps
Flow 6 (10.0.2.3 -> 10.0.2.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0.698006 Mbps
------------------------------------------------
Hidden station experiment with RTS/CTS enabled:
Flow 1 (10.0.0.2 -> 10.0.0.1)
  TxOffered:  2.03995 Mbps
  Throughput: 0.697245 Mbps
Flow 2 (10.0.0.3 -> 10.0.0.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0.729232 Mbps
Flow 3 (10.0.1.2 -> 10.0.1.1)
  TxOffered:  2.03956 Mbps
  Throughput: 0 Mbps
Flow 4 (10.0.1.3 -> 10.0.1.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0 Mbps
Flow 5 (10.0.2.2 -> 10.0.2.1)
  TxOffered:  2.03956 Mbps
  Throughput: 0.692294 Mbps
Flow 6 (10.0.2.3 -> 10.0.2.1)
  TxOffered:  2.04071 Mbps
  Throughput: 0.663734 Mbps
~~~~

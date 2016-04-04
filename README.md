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
git clone https://github.com/jowsteam/jows.git scratch/jows
./waf --run jows
~~~~

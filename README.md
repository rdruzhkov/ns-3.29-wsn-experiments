# Requirements
- ns-3.29

# wsn-experiments

## AODV hello flood attack

### Running simulation

#### With wifi defice energy model

1. Place aodv-hello-flood.cc to scratch directory.
2. ./waf
3. --run aodv-hello-flood

#### With AODV routing protocol energy model

1. Set up AODV routing protocol energy model.
2. Place aodv-hello-flood-2.cc to scratch directory.
3. ./waf
4. ./waf --run aodv-hello-flood-2

## AODV black hole attack

### Running simulation

1. Place aodv-routing-protocol_blackhole.h and aodv-routing-protocol_blackhole.cc files into src/aodv/model directory.
2. Remove "_blackhole" from file names of files placed.
3. Place blackhole.cc to scratch directory.
4. ./waf
5. ./waf --run blackhole

## AODV routing protocol energy model

### Setting up

1. Place files aodv-energy-model-helper-* files to src/aodv/helper directory.
2. Place files aodv-energy-model.* and aodv-routing-protocol.* to src/aodv/model directory.
3. Edit src/aodv/wscript to add new files to next building.

Example of usage is in aodv-hello-flood/aodv-hello-flood-2.cc file.

## AODV IPS

### Hello flood prevention

#### Setting up

Use files from directory "fsm-based-ips".

1. Set up AODV routing protocol energy model.
2. Place files all files to src/aodv/model directory.
3. Edit src/aodv/wscript to add new files to next building.

#### Running simulation

1. Place aodv-hello-flood/aodv-hello-flood-3 to scratch directory.
2. ./waf
3. ./waf --run aodv-hello-flood-3

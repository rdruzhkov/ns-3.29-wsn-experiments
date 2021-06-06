# wsn-energy-drain-attacks

Work in progress.  

## Where to place modified files for traffic redirection simulation
Note: For other simulations use original aodv module files.

experimental-finite-state-machine.cc, experimental-finite-state-machine.h, aodv-routing-protocol.cc, aodv-routing-protocol.h - must be placed at src/aodv/model/.  
  
wscript - must be placed in src/aodv/

## Running simulations
./waf  
./waf aodv-hello-flood  
./waf worm-whole-attack  // Not working :(  
./waf traffic-redirection
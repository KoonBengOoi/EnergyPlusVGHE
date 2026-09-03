# EnergyPlus

This GradientsGTM project provides the modeling of increase-in-stable-ground-temperatures with-depth, below the near-surface temperatures which are governed by the Kusuda formula. Using a typical thermal diffusivity of 1e-6 m2/s for clay, the Kusuda fluctuations of the annual-average-surface-temperature decays exponentially to insignificance (<1% fluctuations) at about 15 meters, which is provided as a default. The new source codes GradientsGTM.cc/.hh are added to the GroundTemperatureModeling folder

Files submitted: 
GradientsGTM.cc provides a Blendwidth (defaults to +/- 2.5m) between the fluctuating near-surface and stable-ground-temperatures. EnergyPlus users provide the thermal gradients (K/m) for an extensible number of segments (upper and lower levels) till the full length (H meters) of the borehole of the vertical ground heat exchanger (VGHE). The A and N fields for this new Site:GroundTemperature:undisturbed:GradientSegments are added to the Energy+.idd.in file. 
 
GradientsGTM.cc computes the weighted-average-temperature of the borehole in a GetHybridGroundTemp function. This Farfield temperature is used by a new dynamic cast in the modified Vertical.cc in the GroundHeatExchangers folder. 

GradientsGTM.unit.cc is included 

Empirical verification: 
In Victoria, Australia, Melbourne University recorded measurements at close vi cinity suburbs at around 25m (Burnley campus) and 50m (Mulgrave Vic 3170). These provide thermal gradients of about 0.05K/m to be used for analysis of simulated results without and with GradientsGTM . 

Uses of GroundTemperatureModeling wth GradientsGTM:
Modeling with GradientsGTM will enable: 
   (1) more precise designs of the depth of VGHE. This is expected to be shorter, thus contribute to a reduction in expensive borehole drilling cost, 
   (2) better prediction of the summer recharge needed to ensure sustainble ground heat source for the life of ground coupled heat pumps for winter heating    

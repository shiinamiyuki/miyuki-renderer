# Miscellaneous	

This document contains some random though about how the project should evolve.

## Designs 

It's worth noticing that in current update, for each class of graph node, there is a corresponding class in `miyuki/core`. I did this deliberately since I have the intend to port the renderer onto GPU. None of us would like to see the device code containing tons of  `operator new()` and virtual function calls and RTTI and exceptions running on GPU. (Would you? ) 
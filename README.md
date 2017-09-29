# kelvinlets
implement paper Regularized Kelvinlets: Sculpting Brushes based on Fundamental Solutions of Elasticity    
Status:    
now basic grab, twist , scale, and pinch brush are implemented.    
multiscale iterpolation is not implemented yet.    
pinch still has some bug.   
there's no control to define whether only front faces will be deformed or both front and back faces will be deformed.   
brush circle is not displayed.    


Usage:    
Compile it and load plugin.    
Create a plane in maya.    
Then Execute mel scirpt:    
if (`KelvinLetBrushCtx -q -ex test `) delete UI test;    
KelvinLetBrushCtx -brushMode "Grab" -meshToManipulate "pPLane1" -pressure 0.1 -radiusScale 1 -possonRation 0.4 "test";    
setToolTo test;    
note:  brushMode can be Grab, Scale, Twist and Pinch.

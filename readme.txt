Roeurn Tourn
Jan Lorenz Soliman

We ran our code on 255x12. For some reason it would not run on all machines. 
The final picture from the GPU had a few specs because we did not implement
mutexes.

Time CPU (1 bunny):
  Time rasterizing triangles
   1.100000 seconds

  Using Time Command
    real  0m2.562s
    user  0m0.917s

Time CPU (25 bunnies):
  Time rasterizing triangles
   5.260000 seconds

  Using Time Command
    real  0m7.451s
    user  0m5.942s

Time GPU (25 bunnies): 
  Time rasterizing triangles
   1.030000 seconds

  Using Time Command
    real 0m2.062s
    user  0m1.010s

Average Error per color for each pixel
R: -0.00033 G: 0.0008 B: 0.0004



VARIABLES [ x, y ]
MODES [ m1 ]
STATESPACE 
  m1[[0, 4], [0, 4]]
INITIAL 
  m1{x>=2.5/\x<=3/\y=0}
FLOW
  m1{x_d=x-y}{y_d=x+y}
JUMP
UNSAFE
  m1{x<=2}

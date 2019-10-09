VARIABLES [ x1, x2 ]
MODES [ m1 ]
STATESPACE 
  m1[[0, 4], [0, 4]]
INITIAL 
  m1{x1>=2.5/\x1<=3/\x2=0}
FLOW
  m1{x1_d=-x1-x2}{x2_d=x1-x2}[-1+1i[1,0][0,1]]
JUMP
UNSAFE
  m1{x1>3\/x2>3}

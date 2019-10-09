VARIABLES [x1,x2]
MODES [ m1 ]
STATESPACE 
  m1[[0,2],[0,2]]
INITIAL 
  m1{x1=0/\x2=0}
FLOW
  m1{x1_d=1}{x2_d=1}
JUMP
UNSAFE
  m1{x1>=1.5/\x2<=1}


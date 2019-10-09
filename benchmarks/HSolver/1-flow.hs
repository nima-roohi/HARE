VARIABLES [x1,x2,x3]
MODES [ m1 ]
STATESPACE 
  m1[[0,2],[0,2],[0,4]]
INITIAL 
  m1{x1>=0/\x1<=1/\x2=0/\x3=0}
FLOW
  m1{x1_d=1}{x2_d=1}{x3_d=1}
JUMP
UNSAFE
  m1{x1>=0/\x1<=2/\x2>1/\x2<=2/\x3>0/\x3<1}

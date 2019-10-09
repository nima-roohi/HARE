VARIABLES [x1,x2,x3]
MODES [ m1]
STATESPACE
  m1[[1,5],[1,5],[0,4]]
INITIAL
  m1{x1>=4/\x1<=4.5/\x2=1/\x3=0}
FLOW
  m1{x1_d=-5.5*x2+x2*x2}{x2_d=6*x1-x1*x1}{x3_d=1}
JUMP
UNSAFE
  m1{x1>=1/\x1<2/\x2>2/\x2<3/\x3>=2/\x3<=4}

VARIABLES [x1,x2]
MODES [ m1 ]
STATESPACE
  m1[[-4,4],[-4,4]]
INITIAL
  m1{x1=3/\x2=3}
FLOW
  m1{x1_d=x2}{x2_d=-1.5*(x1^2 - 1)*x2 - x1}
UNSAFE
  m1{[x1>-0.1/\x1<0.1/\x2>-0.1/\x2<0.1]}

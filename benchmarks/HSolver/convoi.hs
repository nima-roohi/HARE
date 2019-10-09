(*x1, x2, x3, and x4 represent gap v_r, v_l and a_r, respectively*)
VARIABLES [x1,x2,x3,x4]
MODES [ m1 ]
STATESPACE
  m1[[0,4],[0,2],[0,2],[-2,-0.5]]
INITIAL
  m1{x1=1/\x2=2/\x3=2/\x4=-0.5}
FLOW
  m1{x1_d=x3-x2}{x2_d=x4}{x3_d>=-2/\x3_d<=-0.5}{x4_d=-3*x4-3*(x2-x3)+(x1-x2-10)}
JUMP
UNSAFE
  m1{x1<=0}


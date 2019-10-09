(*x1, x2, x3, and x4 represent v_r, v_l, a_r, and gap, respectively*)
VARIABLES [x1,x2,x3,x4]
MODES [ m1 ]
STATESPACE
  m1[[0,30],[0,30],[-2,5],[-10,0]]
INITIAL
  m1{ -0.8522*x1-0.1478*x2-0.3177*x3+x4>0}
FLOW
  m1{x1_d=x3}{x2_d=0}{x3_d=-4*x1+3*x2-3*x3+x4}{x4_d=-x1+x2}
    [0.[1.,1.,0.,1.]][-0.3177[-0.8522,-0.1478,-0.3177,1.]][-1.3412+1.1615i[0.3177,-0.3177,1.,0.][-3.1476,2.1476,0.,1.]]
JUMP
UNSAFE
  m1{x4<=-10}

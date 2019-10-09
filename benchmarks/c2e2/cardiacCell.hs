VARIABLES [ u, v]
MODES [ m0, m1 ]
STATESPACE 
  m0[[-100,0.5], [-100,100]]
  m1[[0,100], [-100,100]]
INITIAL 
  m0{u=0 /\ v=0}
FLOW
  m0{u_d=-0.9*u*u-u*u*u-0.9*u-v+1}{v_d=u-2*v}
  m1{u_d=-0.9*u*u-u*u*u-0.9*u-v}{v_d=u-2*v}
JUMP
  m0->m1{u>=0.5 /\ u'=u /\ v'=v}
  m1->m0{u<=0 /\ u'=u /\ v'=v}
UNSAFE
  m0{u>=2.5}
  m1{u>=2.5}

VARIABLES [ x, z]
MODES [ m0, m1 ]
STATESPACE 
  m0[[65,75], [0,10]]
  m1[[65,75], [0,10]]
INITIAL 
  m0{x>=68 /\ x<=69}
FLOW
  m0{x_d=40-0.5*x}{z_d=1}
  m1{x_d=30-0.5*x}{z_d=1}
JUMP
  m0->m1{x>=75 /\ x'=x /\ z'=z}
  m1->m0{x<=65 /\ x'=x /\ z'=z}
UNSAFE
  m0{x<=66}
  m1{x<=66}

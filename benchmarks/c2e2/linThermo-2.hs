VARIABLES [ x]
MODES [ m0, m1 ]
STATESPACE 
  m0[[65,75]]
  m1[[65,75]]
INITIAL 
  m1{x>=68 /\ x<=72}
FLOW
  m0{x_d=40-0.5*x}
  m1{x_d=30-0.5*x}
JUMP
  m0->m1{x>=75 /\ x'=x}
  m1->m0{x<=65 /\ x'=x}
UNSAFE
  m0{x<=63}
  m1{x<=63}

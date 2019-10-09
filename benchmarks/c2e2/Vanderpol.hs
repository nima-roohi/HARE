VARIABLES [ x, y]
MODES [ m0 ]
STATESPACE 
  m0[[-100, 100], [-100, 100]]
INITIAL 
  m0{x>=1.0 /\ x<=1.2 /\ y>=1.0 /\ y<=1.2}
FLOW
  m0{x_d=y}{y_d=-x*x*y - x - y}
JUMP
UNSAFE
  m0{x>=5}

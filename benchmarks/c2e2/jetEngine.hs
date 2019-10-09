VARIABLES [ x, y]
MODES [ m0 ]
STATESPACE 
  m0[[-100, 100], [-100, 100]]
INITIAL 
  m0{x>=0.8 /\ x<=1.2 /\ y>=0.8 /\ y<=1.2}
FLOW
  m0{x_d=-y-1.5*x*x-0.5*x*x*x-0.5}{y_d=3*x-y}
JUMP
UNSAFE
  m0{x>=10}

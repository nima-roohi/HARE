(* contributed by Luca Geretti *)
VARIABLES [ x, t]
MODES [ m1 ]
STATESPACE 
  m1[[-3, 3], [0, 10]]
INITIAL 
  m1{x=0/\t=0}
FLOW
  m1{x_d=COS(t)}{t_d=1}
JUMP
UNSAFE
  m1{x<=-2 \/ x>=2}

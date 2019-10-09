VARIABLES [x,y]
MODES [n,m]
STATESPACE
  n[[0,1],[0,1]]
  m[[0,1],[0,1]]
INITIAL
  n{x<0.5}
FLOW
  n{x_d=0}{y_d=1}
  m{x_d=0}{y_d=1}
JUMP
  n->m{x'=x/\y'=y+0.1}
  m->n{x'=x/\y'=y}
UNSAFE
  n{x>0.6}

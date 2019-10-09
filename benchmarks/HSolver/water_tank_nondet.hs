(* contributed by Luca Geretti, adapted by Tomas Dzetkulic *)
VARIABLES [x,a]
MODES 
  [ opening,opened,closing,closed ]
STATESPACE
  opening[[0,10],[-1,2]] (* valve opening *)
  opened[[0,10],[-1,2]] (* valve opened *)
  closing[[0,10],[-1,2]] (* valve closing *)
  closed[[0,10],[-1,2]] (* valve closed *)
INITIAL
  opened{x=6/\a=1}
FLOW
  opening{a<=1/\x_d=-0.02*x+0.3*a}{a_d=0.25}
  opened{x<=8.05/\x_d=-0.02*x+0.3}{a_d=0}
  closing{a>=0/\x_d=-0.02*x+0.3*a}{a_d=-0.25}
  closed{x>=5.45/\x_d=-0.02*x}{a_d=0}
JUMP
  opening->opened{[a=1]/\[x'=x/\a'=1]}
  opened->closing{[x>=7.95]/\[x'=x/\a'=1]}
  closing->closed{[a=0]/\[x'=x/\a'=0]}
  closed->opening{[x<=5.55]/\[x'=x/\a'=0]}
UNSAFE
  opening{x>=8.25\/x<=5.25}
  opened{x>=8.25\/x<=5.25}
  closing{x>=8.25\/x<=5.25}
  closed{x>=8.25\/x<=5.25}


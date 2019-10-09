(*This is obtained from carSR by increasing the unsafe area BUT
keeping all the jumps. An error trajectory is NOT found.*)
VARIABLES [x,gamma,c]
MODES [ m1,m2,m3,m4,m5 ]
STATESPACE
  m1[[-1,1],[-0.8,0.8],[0,3]] (* go_ahead, straight_ahead *)
  m2[[-4,-1],[-0.8,0.8],[0,3]] (* left_border, in_canal *)
  m3[[1,4],[-0.8,0.8],[0,3]] (* right_border *)
  m4[[-1,1],[-0.8,0.8],[0,3]] (* correct_left *)
  m5[[-1,1],[-0.8,0.8],[0,3]] (* correct_right *)
INITIAL
  m1{x>=-1/\x<=1/\4*gamma>=-PI/\4*gamma<=PI/\c=0}
FLOW
  m1{x_d=-2*SIN(gamma) /\ -1 <= x /\ x <= 1}{gamma_d=0}{c_d=0}
  m2{x_d=-2*SIN(gamma) /\ x <= -1}{4*gamma_d=-PI}{c_d=1}
  m3{x_d=-2*SIN(gamma) /\ x >= 1}{4*gamma_d=PI}{c_d=1}
  m4{x_d=-2*SIN(gamma) /\ -1 <= x /\ x <= 1}{4*gamma_d=PI}{c_d=-2}
  m5{x_d=-2*SIN(gamma) /\ -1 <= x /\ x <= 1}{4*gamma_d=-PI}{c_d=-2}
JUMP
  m1->m2{x<=-1/\x'=x/\c'=0/\gamma'=gamma}
  m1->m3{x>=1/\x'=x/\c'=0/\gamma'=gamma}
  m2->m4{x>=-1/\x'=x/\gamma'=gamma/\c'=c}
  m3->m5{x<=-1/\x'=x/\gamma'=gamma/\c'=c}
  m4->m1{c<=0/\x'=x/\gamma'=gamma/\c'=c}
  m4->m3{x>=1/\x'=x/\c'=0/\gamma'=gamma}
  m5->m1{c<=0/\x'=x/\gamma'=gamma/\c'=c}
  m5->m2{x<=-1/\x'=x/\c'=0/\gamma'=gamma}
UNSAFE
  m2{x<=-4}


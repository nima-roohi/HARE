VARIABLES [sx, vx, ax, vy, omega, sy]
MODES [ m0, m1, m2, m3, m4, m5, m6 ]
STATESPACE 
  m0[[-100,-10], [-100,100], [-100,100], [-100,100], [-100,100], [-100,100]]
  m1[[-100,100], [-100,100], [-100,100], [-100,100], [-100,100], [-100,12]]
  m2[[-100,100], [-100,100], [-100,100], [0.05,100], [-100,100], [-100,100]]
  m3[[-100,100], [-100,100], [-100,100], [-100,-0.05], [-100,100], [-100,100]]
  m4[[-100,100], [-100,100], [-100,100], [-100,100], [-100,100], [3.5,100]]
  m5[[-100,10], [-100,100], [-100,100], [-100,100], [-100,100], [-100,100]]
  m6[[-100,100], [-100,100], [-100,100], [-100,100], [-100,100], [-100,100]]
INITIAL 
  m0{sx >= -15.0 /\ sx <= -14.95 /\ vx >= 3.25 /\ vx <= 3.3 /\ ax = 0 /\ vy = 0 /\ omega = 0 /\ sy = 0}
FLOW
  m0{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.01*sx -0.01*10.3 + 0.3*2.8 - 0.3*vx - 0.5*ax}{vy_d=-2*vy}{omega_d=-2*omega}{sy_d=0.1*vy}
  m1{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.5*vx+1.4-0.5*ax}{vy_d=2.5*3 - 0.15*3*omega + 0.5 - 0.025*sy - 0.05*vy}{omega_d=3 - 3*0.05*omega + 0.2-0.01*sy}{sy_d=0.1*vy}
  m2{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.5*vx+1.4-0.5*ax}{vy_d=-0.1*2.5*omega + 0.5 - 0.025*sy - 0.05*vy}{omega_d=-0.1*omega + 0.2 - 0.01*sy}{sy_d=0.1*vy}
  m3{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.5*vx+1.4-0.5*ax}{vy_d=-0.1*2.5*omega + 0.5 - 0.025*sy - 0.05*vy}{omega_d=-0.1*omega + 0.2 - 0.01*sy}{sy_d=0.1*vy}
  m4{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.5*vx+1.4-0.5*ax}{vy_d=-2.5*3 - 0.15*3*omega + 0.5 - 0.025*sy - 0.05*vy}{omega_d=-3 - 3*0.05*omega + 0.2-0.01*sy}{sy_d=0.1*vy}
  m5{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.01*sx + 0.01*10.3 + 0.3*2.8 - 0.3*vx - 0.5*ax}{vy_d=-2*vy}{omega_d=-2*omega}{sy_d=0.1*vy}
  m6{sx_d=vx-2.5}{vx_d=0.1*ax}{ax_d=-0.5*vx+1.4-0.5*ax}{vy_d=-2*vy}{omega_d=-2*omega}{sy_d=0.1*vy}
JUMP
  m0->m1{sx >= -10   /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
  m1->m2{sy >=  12   /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
  m4->m3{sy <=  3.5  /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
  m5->m4{sx >=  10   /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
  m2->m5{vy <=  0.05 /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
  m3->m6{vy >= -0.05 /\ sx' = sx /\ vx' = vx /\ ax' = ax /\ vy' = vy /\ omega' = omega /\ sy' = sy}
UNSAFE
  m0 {vx >= 3.5}
  m1 {vx >= 3.5}
  m2 {vx >= 3.5}
  m3 {vx >= 3.5}
  m4 {vx >= 3.5}
  m5 {vx >= 3.5}
  m6 {vx >= 3.5}

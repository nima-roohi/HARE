VARIABLES [height,hfiltered,clock]
MODES [m1,m2,m3,m4,m5]
STATESPACE 
  m1[[-100,100],[-100,100],[0,1]] (* in_tolerance *)
  m2[[-100,100],[-100,100],[0,1]] (* up_s ... stopped *)
  m3[[-100,100],[-100,100],[0,1]] (* down_s *) 
  m4[[-100,100],[-100,100],[0,1]] (* up_d ... driving *)
  m5[[-100,100],[-100,100],[0,1]] (* down_d *)
INITIAL
  m1{height=0/\hfiltered=0/\clock=0}
FLOW (* jumps are enforced by the bound [0, 1] on the clock *)
  m1{height_d>=-1/\height_d<=1}{2*hfiltered_d=height-hfiltered}{clock_d=1}
  m2{height_d>=0/\height_d<=3}{2*hfiltered_d=height-hfiltered}{clock_d=1} (* air is pressed into suspension *)
  m3{height_d>=-3/\height_d<=0}{2*hfiltered_d=height-hfiltered}{clock_d=1} (* air is let out of suspensions *)
  m4{height_d>=0/\height_d<=3}{2*hfiltered_d=height-hfiltered}{clock_d=1} (* air is pressed into suspension *)
  m5{height_d>=-3/\height_d<=0}{2*hfiltered_d=height-hfiltered}{clock_d=1} (* air is let out of suspensions *)
JUMP
  m1->m1{[[[hfiltered>=-40/\hfiltered<=20/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]]\/[[hfiltered>=-10/\hfiltered<=10/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]]]}
  m1->m2{[hfiltered<-40/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m1->m3{[hfiltered>20/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m1->m4{[hfiltered<-10/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m1->m5{[hfiltered>10/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m2->m1{[hfiltered>=-6/\hfiltered<=20/\clock=1]/\[height'=height/\hfiltered'=0/\clock'=0]}
  m2->m2{[hfiltered<-6/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m2->m3{[hfiltered>20/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m3->m1{[hfiltered>=-40/\hfiltered<=16/\clock=1]/\[height'=height/\hfiltered'=0/\clock'=0]}
  m3->m2{[hfiltered<-40/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m3->m3{[hfiltered>16/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m4->m1{[hfiltered>=-6/\hfiltered<=10/\clock=1]/\[height'=height/\hfiltered'=0/\clock'=0]}
  m4->m4{[hfiltered<-6/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m4->m5{[hfiltered>10/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m5->m1{[hfiltered>=-10/\hfiltered<=6/\clock=1]/\[height'=height/\hfiltered'=0/\clock'=0]}
  m5->m4{[hfiltered<-10/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
  m5->m5{[hfiltered>6/\clock=1]/\[height'=height/\hfiltered'=hfiltered/\clock'=0]}
UNSAFE
  m1{[height<-47\/height>27]}
  m2{[height<-47\/height>27]}
  m3{[height<-47\/height>27]}
  m4{[height<-47\/height>27]}
  m5{[height<-47\/height>27]}


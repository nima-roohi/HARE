VARIABLES [xx,yy,vx,vy] (* [xx,yy] is the position on the map [vx,vy] is the speed *)
MODES [m2,m3,m4,m5,m6,m7,m8]
(* navigation map modes: *)

(*  unsafe  m2   m3  *) 
(*    m4    m5   m6  *)
(*    m7    m8  safe *)

(* target speed: *)
(* m3 m6: (0,-1) *)
(* m2 m4 m7 m8: (1,0) *)
(* m5: (0.707,-0.707) *)

STATESPACE
  m2[[1,2],[2,3],[-2,2],[-2,2]]
  m3[[2,3],[2,3],[-2,2],[-2,2]]
  m4[[0,1],[1,2],[-2,2],[-2,2]]
  m5[[1,2],[1,2],[-2,2],[-2,2]]
  m6[[2,3],[1,2],[-2,2],[-2,2]]
  m7[[0,1],[0,1],[-2,2],[-2,2]]
  m8[[1,2],[0,1],[-2,2],[-2,2]]
INITIAL
  m6{vx>=-0.3/\vx<=0.3/\vy>=-0.3/\vy<0}
FLOW
  m2{xx_d=vx}{yy_d=vy}{vx_d=-1.2*(vx-1)+0.1*vy}{vy_d=0.1*(vx-1)-1.2*vy}
  m3{xx_d=vx}{yy_d=vy}{vx_d=-1.2*vx+0.1*(vy+1)}{vy_d=0.1*vx-1.2*(vy+1)}
  m4{xx_d=vx}{yy_d=vy}{vx_d=-1.2*(vx-1)+0.1*vy}{vy_d=0.1*(vx-1)-1.2*vy}
  m5{xx_d=vx}{yy_d=vy}{vx_d=-1.2*(vx-0.707)+0.1*(vy+0.707)}{vy_d=0.1*(vx-0.707)-1.2*(vy+0.707)}
  m6{xx_d=vx}{yy_d=vy}{vx_d=-1.2*vx+0.1*(vy+1)}{vy_d=0.1*vx-1.2*(vy+1)}
  m7{xx_d=vx}{yy_d=vy}{vx_d=-1.2*(vx-1)+0.1*vy}{vy_d=0.1*(vx-1)-1.2*vy}
  m8{xx_d=vx}{yy_d=vy}{vx_d=-1.2*(vx-1)+0.1*vy}{vy_d=0.1*(vx-1)-1.2*vy}
JUMP
  m2->m3{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m2->m5{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m3->m2{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m3->m6{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m4->m5{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m4->m7{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m5->m2{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m5->m4{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m5->m6{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m5->m8{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m6->m3{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m6->m5{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m7->m4{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m7->m8{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m8->m5{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
  m8->m7{xx'=xx/\yy'=yy/\vx'=vx/\vy'=vy}
UNSAFE (* if we reach border of unsafe box *)
  m2{xx=1} 
  m4{yy=2}

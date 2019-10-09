(set-logic QF_NRA_ODE)
(declare-fun y () Real [0.200000, 2.000000])
(declare-fun x () Real [2.000000, 3.000000])
(declare-fun vy () Real [-2.000000, 1.600000])
(declare-fun vx () Real [-2.000000, 2.000000])
(declare-fun y_0_0 () Real [0.200000, 2.000000])
(declare-fun y_0_t () Real [0.200000, 2.000000])
(declare-fun y_1_0 () Real [0.200000, 2.000000])
(declare-fun y_1_t () Real [0.200000, 2.000000])
(declare-fun y_2_0 () Real [0.200000, 2.000000])
(declare-fun y_2_t () Real [0.200000, 2.000000])
(declare-fun y_3_0 () Real [0.200000, 2.000000])
(declare-fun y_3_t () Real [0.200000, 2.000000])
(declare-fun y_4_0 () Real [0.200000, 2.000000])
(declare-fun y_4_t () Real [0.200000, 2.000000])
(declare-fun x_0_0 () Real [2.000000, 3.000000])
(declare-fun x_0_t () Real [2.000000, 3.000000])
(declare-fun x_1_0 () Real [2.000000, 3.000000])
(declare-fun x_1_t () Real [2.000000, 3.000000])
(declare-fun x_2_0 () Real [2.000000, 3.000000])
(declare-fun x_2_t () Real [2.000000, 3.000000])
(declare-fun x_3_0 () Real [2.000000, 3.000000])
(declare-fun x_3_t () Real [2.000000, 3.000000])
(declare-fun x_4_0 () Real [2.000000, 3.000000])
(declare-fun x_4_t () Real [2.000000, 3.000000])
(declare-fun vy_0_0 () Real [-2.000000, 1.600000])
(declare-fun vy_0_t () Real [-2.000000, 1.600000])
(declare-fun vy_1_0 () Real [-2.000000, 1.600000])
(declare-fun vy_1_t () Real [-2.000000, 1.600000])
(declare-fun vy_2_0 () Real [-2.000000, 1.600000])
(declare-fun vy_2_t () Real [-2.000000, 1.600000])
(declare-fun vy_3_0 () Real [-2.000000, 1.600000])
(declare-fun vy_3_t () Real [-2.000000, 1.600000])
(declare-fun vy_4_0 () Real [-2.000000, 1.600000])
(declare-fun vy_4_t () Real [-2.000000, 1.600000])
(declare-fun vx_0_0 () Real [-2.000000, 2.000000])
(declare-fun vx_0_t () Real [-2.000000, 2.000000])
(declare-fun vx_1_0 () Real [-2.000000, 2.000000])
(declare-fun vx_1_t () Real [-2.000000, 2.000000])
(declare-fun vx_2_0 () Real [-2.000000, 2.000000])
(declare-fun vx_2_t () Real [-2.000000, 2.000000])
(declare-fun vx_3_0 () Real [-2.000000, 2.000000])
(declare-fun vx_3_t () Real [-2.000000, 2.000000])
(declare-fun vx_4_0 () Real [-2.000000, 2.000000])
(declare-fun vx_4_t () Real [-2.000000, 2.000000])
(declare-fun time_0 () Real [0.000000, 2.000000])
(declare-fun time_1 () Real [0.000000, 2.000000])
(declare-fun time_2 () Real [0.000000, 2.000000])
(declare-fun time_3 () Real [0.000000, 2.000000])
(declare-fun time_4 () Real [0.000000, 2.000000])
(declare-fun mode_0 () Real [1.000000, 7.000000])
(declare-fun mode_1 () Real [1.000000, 7.000000])
(declare-fun mode_2 () Real [1.000000, 7.000000])
(declare-fun mode_3 () Real [1.000000, 7.000000])
(declare-fun mode_4 () Real [1.000000, 7.000000])
(define-ode flow_1 ((= d/dt[vx] (+ -2 (* vx 2))) (= d/dt[vy] (* vy 2)) (= d/dt[x] vx) (= d/dt[y] vy)))
(define-ode flow_2 ((= d/dt[vx] (+ -2 (* vx 2))) (= d/dt[vy] (* vy 2)) (= d/dt[x] vx) (= d/dt[y] vy)))
(define-ode flow_3 ((= d/dt[vx] (+ -2 (* vx 2))) (= d/dt[vy] (* vy 2)) (= d/dt[x] vx) (= d/dt[y] vy)))
(define-ode flow_4 ((= d/dt[vx] (+ 2 (* vx 2))) (= d/dt[vy] (+ -2 (* vy 2))) (= d/dt[x] vx) (= d/dt[y] vy)))
(define-ode flow_5 ((= d/dt[vx] (+ 2 (* vx 2))) (= d/dt[vy] (+ 2 (* vy 2))) (= d/dt[x] vx) (= d/dt[y] vy)))
(define-ode flow_6 ((= d/dt[vx] 0) (= d/dt[vy] 0) (= d/dt[x] 0) (= d/dt[y] 0)))
(define-ode flow_7 ((= d/dt[vx] 0) (= d/dt[vy] 0) (= d/dt[x] 0) (= d/dt[y] 0)))
(assert (and (or (= mode_4 7) (= mode_4 6) (= mode_4 5) (= mode_4 4) (= mode_4 3) (= mode_4 2)) (and (= vx_0_0 1) (= (* 5 y_0_0) 1) (= (* 5 x_0_0) 11) (= vy_0_0 -1)) (= mode_0 1) (= [vx_0_t vy_0_t x_0_t y_0_t] (integral 0. time_0 [vx_0_0 vy_0_0 x_0_0 y_0_0] flow_1)) (= mode_0 1) (forall_t 1 [0 time_0] (>= (- 0 vy_0_t) 0)) (>= (- 0 vy_0_t) 0) (>= (- 0 vy_0_0) 0) (forall_t 1 [0 time_0] (>= (- 0 vx_0_t) -1)) (>= (- 0 vx_0_t) -1) (>= (- 0 vx_0_0) -1) (forall_t 1 [0 time_0] (>= (- 0 y_0_t) -1)) (>= (- 0 y_0_t) -1) (>= (- 0 y_0_0) -1) (forall_t 1 [0 time_0] (>= (- 0 x_0_t) -3)) (>= (- 0 x_0_t) -3) (>= (- 0 x_0_0) -3) (forall_t 1 [0 time_0] (>= vy_0_t -1)) (>= vy_0_t -1) (>= vy_0_0 -1) (forall_t 1 [0 time_0] (>= x_0_t 2)) (>= x_0_t 2) (>= x_0_0 2) (forall_t 1 [0 time_0] (>= y_0_t 0)) (>= y_0_t 0) (>= y_0_0 0) (forall_t 1 [0 time_0] (>= (* 2 vx_0_t) 1)) (>= (* 2 vx_0_t) 1) (>= (* 2 vx_0_0) 1) (= mode_1 2) (>= (* 20 x_0_t) 49) (>= (- 0 x_0_t) -3) (= (* 5 y_0_t) 1) (= (* 2 vx_0_t) 1) (= vy_1_0 -1) (= (* 2 vx_1_0) 1) (= (* 5 y_1_0) 1) (= (- x_0_t x_1_0) 0) (= vy_0_t -1) (= [vx_1_t vy_1_t x_1_t y_1_t] (integral 0. time_1 [vx_1_0 vy_1_0 x_1_0 y_1_0] flow_2)) (= mode_1 2) (forall_t 2 [0 time_1] (>= (- 0 vy_1_t) 0)) (>= (- 0 vy_1_t) 0) (>= (- 0 vy_1_0) 0) (forall_t 2 [0 time_1] (>= (* -2 vx_1_t) -1)) (>= (* -2 vx_1_t) -1) (>= (* -2 vx_1_0) -1) (forall_t 2 [0 time_1] (>= (- 0 y_1_t) -1)) (>= (- 0 y_1_t) -1) (>= (- 0 y_1_0) -1) (forall_t 2 [0 time_1] (>= (- 0 x_1_t) -3)) (>= (- 0 x_1_t) -3) (>= (- 0 x_1_0) -3) (forall_t 2 [0 time_1] (>= vy_1_t -1)) (>= vy_1_t -1) (>= vy_1_0 -1) (forall_t 2 [0 time_1] (>= x_1_t 2)) (>= x_1_t 2) (>= x_1_0 2) (forall_t 2 [0 time_1] (>= y_1_t 0)) (>= y_1_t 0) (>= y_1_0 0) (forall_t 2 [0 time_1] (>= vx_1_t 0)) (>= vx_1_t 0) (>= vx_1_0 0) (= mode_2 3) (>= (* 20 x_1_t) 49) (>= (- 0 x_1_t) -3) (= vx_1_t 0) (= (* 5 y_1_t) 1) (= vy_2_0 -1) (= vx_2_0 0) (= (* 5 y_2_0) 1) (= (- x_1_t x_2_0) 0) (= vy_1_t -1) (= [vx_2_t vy_2_t x_2_t y_2_t] (integral 0. time_2 [vx_2_0 vy_2_0 x_2_0 y_2_0] flow_3)) (= mode_2 3) (forall_t 3 [0 time_2] (>= (- 0 vy_2_t) -2)) (>= (- 0 vy_2_t) -2) (>= (- 0 vy_2_0) -2) (forall_t 3 [0 time_2] (>= (- 0 vx_2_t) 0)) (>= (- 0 vx_2_t) 0) (>= (- 0 vx_2_0) 0) (forall_t 3 [0 time_2] (>= (- 0 y_2_t) -1)) (>= (- 0 y_2_t) -1) (>= (- 0 y_2_0) -1) (forall_t 3 [0 time_2] (>= (- 0 x_2_t) -3)) (>= (- 0 x_2_t) -3) (>= (- 0 x_2_0) -3) (forall_t 3 [0 time_2] (>= vy_2_t -2)) (>= vy_2_t -2) (>= vy_2_0 -2) (forall_t 3 [0 time_2] (>= x_2_t 2)) (>= x_2_t 2) (>= x_2_0 2) (forall_t 3 [0 time_2] (>= y_2_t 0)) (>= y_2_t 0) (>= y_2_0 0) (forall_t 3 [0 time_2] (>= vx_2_t -2)) (>= vx_2_t -2) (>= vx_2_0 -2) (= mode_3 4) (>= (+ (* -10 vx_2_t) (* 20 x_2_t)) 57) (>= (* -5 vx_2_t) 4) (>= (- 0 x_2_t) -3) (>= vx_2_t -2) (>= x_2_t 2) (= y_2_t 1) (= (* 5 vy_3_0) 3) (= (- vx_2_t vx_3_0) 0) (= y_3_0 1) (= (- x_2_t x_3_0) 0) (= (* 5 vy_2_t) 3) (= [vx_3_t vy_3_t x_3_t y_3_t] (integral 0. time_3 [vx_3_0 vy_3_0 x_3_0 y_3_0] flow_4)) (= mode_3 4) (forall_t 4 [0 time_3] (>= (- 0 vy_3_t) -2)) (>= (- 0 vy_3_t) -2) (>= (- 0 vy_3_0) -2) (forall_t 4 [0 time_3] (>= (- 0 vx_3_t) -2)) (>= (- 0 vx_3_t) -2) (>= (- 0 vx_3_0) -2) (forall_t 4 [0 time_3] (>= (- 0 y_3_t) -2)) (>= (- 0 y_3_t) -2) (>= (- 0 y_3_0) -2) (forall_t 4 [0 time_3] (>= (- 0 x_3_t) -3)) (>= (- 0 x_3_t) -3) (>= (- 0 x_3_0) -3) (forall_t 4 [0 time_3] (>= vy_3_t -2)) (>= vy_3_t -2) (>= vy_3_0 -2) (forall_t 4 [0 time_3] (>= x_3_t 2)) (>= x_3_t 2) (>= x_3_0 2) (forall_t 4 [0 time_3] (>= y_3_t 1)) (>= y_3_t 1) (>= y_3_0 1) (forall_t 4 [0 time_3] (>= vx_3_t -2)) (>= vx_3_t -2) (>= vx_3_0 -2) (= mode_4 5) (>= (+ (- 0 vy_3_t) (* 3 y_3_t)) 4) (>= (+ (* -5 vy_3_t) (* 5 y_3_t)) 2) (>= (+ (- (* -10 vy_3_t) (* 10 vx_3_t)) (* 20 y_3_t)) 31) (>= (- (+ (* 5 vy_3_t) (* 5 vx_3_t)) (* 10 y_3_t)) -27) (>= (- (* 5 vy_3_t) (* 10 y_3_t)) -27) (>= vy_3_t -2) (>= vx_3_t -2) (>= y_3_t 1) (>= (- 0 y_3_t) -2) (>= (- 0 vx_3_t) -2) (= (- vy_3_t vy_4_0) 0) (= (- vx_3_t vx_4_0) 0) (= (- y_3_t y_4_0) 0) (= x_4_0 2) (= x_3_t 2) (= [vx_4_t vy_4_t x_4_t y_4_t] (integral 0. time_4 [vx_4_0 vy_4_0 x_4_0 y_4_0] flow_5)) (= mode_4 5) (forall_t 5 [0 time_4] (>= (- 0 vy_4_t) -2)) (>= (- 0 vy_4_t) -2) (>= (- 0 vy_4_0) -2) (forall_t 5 [0 time_4] (>= (- 0 vx_4_t) -2)) (>= (- 0 vx_4_t) -2) (>= (- 0 vx_4_0) -2) (forall_t 5 [0 time_4] (>= (- 0 y_4_t) -2)) (>= (- 0 y_4_t) -2) (>= (- 0 y_4_0) -2) (forall_t 5 [0 time_4] (>= (- 0 x_4_t) -2)) (>= (- 0 x_4_t) -2) (>= (- 0 x_4_0) -2) (forall_t 5 [0 time_4] (>= vy_4_t -2)) (>= vy_4_t -2) (>= vy_4_0 -2) (forall_t 5 [0 time_4] (>= x_4_t 1)) (>= x_4_t 1) (>= x_4_0 1) (forall_t 5 [0 time_4] (>= y_4_t 1)) (>= y_4_t 1) (>= y_4_0 1) (forall_t 5 [0 time_4] (>= vx_4_t -2)) (>= vx_4_t -2) (>= vx_4_0 -2)))
(check-sat)
(exit)

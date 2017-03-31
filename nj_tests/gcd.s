.test gcd 81 153 - 9
.test gcd 6  9   - 3

; while (n1 != n2) {
;     if (n1 != n2) {
;         n1 -= n2
;     } else {
;         n2 -= n1;
;     }
; }
; return n1

.func gcd
	nj_frame		2
	nj_inst_jmp		L2

.L1
    nj_inst_lget	1	; n2
	nj_inst_lget	0	; n1
	nj_inst_gt			; n1 > n2
	nj_inst_jmp		L3
	
	; n2 -= n1
    nj_inst_lget	1	; n2
	nj_inst_lget	0	; n1
	nj_inst_sub
	nj_inst_lset	1	; n2 = n2 - n1
	nj_inst_jmp		L2

.L3
	; n1 -= n2
    nj_inst_lget	0	; n1
	nj_inst_lget	1	; n2
	nj_inst_sub
	nj_inst_lset	0	; n1 = n1 - n2

.L2
    nj_inst_lget	1	; n2
	nj_inst_lget	0	; n1
	nj_inst_ne			; n1 != n2
	nj_jmp L1
	nj_inst_lget	0
	nj_inst_ret

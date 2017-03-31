.test fibonacci 0 0
.test fibonacci 1 1
.test fibonacci 5 5
.test fibonacci 8 21

; static int fibonacci(int n)
; {
;     if (n == 0)
;         return 0;
;     if (n == 1)
;         return 1;
;     return fibonacci(n - 1) + fibonacci(n - 2);
; }

.func fibonacci
  nj_inst_const   0
  nj_inst_arg     0h
  nj_inst_eq
  nj_inst_jmp     .l1
  nj_inst_const   0
  nj_inst_ret
.l1
  nj_inst_const   0
  nj_inst_arg     0h
  nj_inst_eq
  nj_inst_jmp     .l2
  nj_inst_const   1
  nj_inst_ret
.l2
  nj_inst_arg     0h
  nj_inst_const   2
  nj_inst_sub
  nj_inst_call    0h
  nj_inst_arg     0h
  nj_inst_const   1
  nj_inst_sub
  nj_inst_call    0h
  nj_inst_add
  nj_inst_ret

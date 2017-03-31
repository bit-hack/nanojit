.test print()       0

.data msg "Hello World!\n"

.func print
  nj_inst_frame     0
  nj_inst_const     $msg
  nj_inst_syscall   "printf"
  nj_inst_const     0
  nj_inst_ret

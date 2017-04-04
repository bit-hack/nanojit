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

.proto fibonacci

.func fibonacci
  const   0
  arg     h0
  ne
  cjmp    .l1       ; ---.
  const   0         ;    |
  ret     0         ;    |
.l1                 ; <--'
  const   0
  arg     h0
  ne
  cjmp    .l2       ; ---.
  const   1         ;    |
  ret     0         ;    |
.l2                 ; <--'
  arg     h0
  const   2
  sub
  call    fibonacci
  arg     h0
  const   1
  sub
  call    fibonacci
  add
  ret     0

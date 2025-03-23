```
make
```

`std` (malloc/free):
```
USER Time used: 9.15362 s
REAL Time used: 2.9509 s
Memory used: 4877 MB
Memory required: 2441.41 MB
Overhead: 49.9%
```

`global_mutex_pool`:
```
[HW7] ./global_mutex_pool                                                                                        main  ✭ ✈
mmap ok 0x7f8cb788a000
USER Time used: 10.4475 s
REAL Time used: 7.7725 s
Memory used: 2444.77 MB
Memory required: 2441.41 MB
Overhead:  0.1%
```
`global_lockfree_pool`:
```
[HW7] ./global_lockfree_pool                                                                                       main  ✭
mmap ok 0x7f10de1ca000
USER Time used: 17.3334 s
REAL Time used: 2.38796 s
Memory used: 2444.84 MB
Memory required: 2441.41 MB
Overhead:  0.1%
```
`local_pools`:
```
[HW7] ./local_pools                                                                                                main  ✭
USER Time used: 1.65078 s
REAL Time used: 0.422417 s
Memory used: 2372.41 MB
Memory required: 2441.41 MB
Overhead: -2.9%
```

sigsegv example for `local_pools`:
```
[HW7] ./local_pools                                                                                                main  ✭
pool #4 exhausted
SIGSEGV
```

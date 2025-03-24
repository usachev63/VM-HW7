```
make
```

`std` (malloc/free):
```
USER Time used: 9.49845 s
REAL Time used: 3.03712 s
Memory used: 4877.1 MB
Memory required: 2441.41 MB
Overhead: 49.9%
```

`global_mutex_pool`:
```
[HW7] ./global_mutex_pool                                                                                          main  ✭
mmap ok 0x7f055b5a2000
USER Time used: 10.2931 s
REAL Time used: 7.79732 s
Memory used: 2444.92 MB
Memory required: 2441.41 MB
Overhead:  0.1%
```
`global_lockfree_pool`:
```
[HW7] ./global_lockfree_pool                                                                                       main  ✭
mmap ok 0x7f63cf2d9000
USER Time used: 16.6578 s
REAL Time used: 2.31965 s
Memory used: 2444.82 MB
Memory required: 2441.41 MB
Overhead:  0.1%
```
`local_pools`:
```
[HW7] ./local_pools                                                                                                main  ✭
USER Time used: 1.57926 s
REAL Time used: 0.419265 s
Memory used: 2390.45 MB
Memory required: 2441.41 MB
Overhead: -2.1%
```

sigsegv example for `local_pools`:
```
[HW7] ./local_pools                                                                                                main  ✭
pool #3 exhausted
[1]    1169848 segmentation fault (core dumped)  ./local_pools
```

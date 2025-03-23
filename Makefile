CPP=g++ -O3

all: std global_mutex_pool global_lockfree_pool local_pools

std: std.cpp
	$(CPP) std.cpp -o std
global_mutex_pool: global_mutex_pool.cpp
	$(CPP) global_mutex_pool.cpp -o global_mutex_pool
global_lockfree_pool: global_lockfree_pool.cpp
	$(CPP) global_lockfree_pool.cpp -o global_lockfree_pool
local_pools: local_pools.cpp
	$(CPP) local_pools.cpp -o local_pools

clean:
	rm -rf ./std ./global_mutex_pool ./global_lockfree_pool ./local_pools

.PHONY: all clean

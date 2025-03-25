#include <atomic>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <thread>
#include <vector>

#define THREADS_NUM 16

using namespace std;

struct sigaction old_sigsegv_act;

static void get_usage(struct rusage &usage) {
  if (getrusage(RUSAGE_SELF, &usage)) {
    perror("Cannot get usage");
    exit(EXIT_SUCCESS);
  }
}

static constexpr size_t PAGE_SIZE = 4096;
static constexpr size_t round_up_to_whole_pages(size_t size) {
  return (size + (PAGE_SIZE - 1)) & -PAGE_SIZE;
}

struct Node {
  Node *next;
  unsigned node_id;
};

class MyPool {
public:
  MyPool() {}

  void init(size_t size);

  // address is 8-aligned
  void *alloc(size_t size);

  void free();

  size_t pool_size;
  void *base_ptr;
  char *free_ptr;
};

struct PoolRegistryRecord {
  char *bottom;
  char *top;
  std::atomic<bool> active = false;
};

struct PoolRegistry {
  static constexpr size_t MAX_POOLS = 128;
  PoolRegistryRecord pools[MAX_POOLS];
  std::atomic<int> nextPoolId;
} poolRegistry;

static void signal_safe_itoa(char *out, unsigned x) {
  if (x == 0) {
    strcpy(out, "0");
    return;
  }
  char *current = out;
  while (x > 0) {
    *current++ = '0' + x % 10;
    x /= 10;
  }
  *current = '\0';
  char *left = out;
  char *right = current - 1;
  while (left < right) {
    char tmp = *right;
    *right = *left;
    *left = tmp;
    ++left;
    --right;
  }
}

static void dump_pool_exhausted_message(int id) {
  constexpr char msg1[] = "pool #";
  constexpr char msg2[] = " exhausted\n";
  write(STDOUT_FILENO, msg1, sizeof msg1 - 1);
  char printedId[32];
  signal_safe_itoa(printedId, id);
  write(STDOUT_FILENO, printedId, strlen(printedId));
  write(STDOUT_FILENO, msg2, sizeof msg2 - 1);
}

static void pool_sigsegv_handler(int signal, siginfo_t *info, void *ucontext) {
  void *fault_addr = info->si_addr;
  int poolsNum = poolRegistry.nextPoolId;
  for (int id = 0; id < poolsNum; ++id) {
    if (!poolRegistry.pools[id].active)
      continue;
    if (fault_addr >= poolRegistry.pools[id].bottom - PAGE_SIZE &&
        fault_addr < poolRegistry.pools[id].top) {
      dump_pool_exhausted_message(id);
      break;
    }
  }
  old_sigsegv_act.sa_sigaction(signal, info, ucontext);
}

void MyPool::init(size_t size) {
  pool_size = round_up_to_whole_pages(size);
  base_ptr = mmap(nullptr, pool_size, PROT_READ | PROT_WRITE | PROT_GROWSDOWN,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
  if (base_ptr == MAP_FAILED) {
    perror("pool allocate (mmap) failed");
    std::exit(1);
  }
  free_ptr = (char *)base_ptr + pool_size;
}

void *MyPool::alloc(size_t size) { return free_ptr -= size; }

void MyPool::free() {
  if (munmap(base_ptr, pool_size) != 0) {
    perror("pool free (munmap) failed");
    std::exit(1);
  }
  base_ptr = nullptr;
  free_ptr = nullptr;
}

static inline Node *create_list(unsigned n, MyPool &pool) {
  Node *list = nullptr;
  for (unsigned i = 0; i < n; i++) {
    Node *newList = (Node *)pool.alloc(sizeof(Node));
    newList->next = list;
    newList->node_id = i;
    list = newList;
  }
  return list;
}

static void testOneThread(unsigned n, int i) {
  MyPool pool;
  // if (i == 12)
  //   pool.init(n);
  // else
  pool.init(n * sizeof(Node));
  int poolId = poolRegistry.nextPoolId++;
  poolRegistry.pools[poolId].bottom = (char *)pool.base_ptr;
  poolRegistry.pools[poolId].top = (char *)pool.base_ptr + pool.pool_size;
  poolRegistry.pools[poolId].active = true;
  create_list(n, pool);
  pool.free();
}

static inline void test(unsigned n) {
  struct sigaction act = {0};
  act.sa_sigaction = pool_sigsegv_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &act, &old_sigsegv_act);

  struct rusage start, finish;
  get_usage(start);
  auto chronoStart = std::chrono::steady_clock::now();

  std::vector<std::thread> threads;
  for (int i = 0; i < THREADS_NUM; ++i) {
    threads.emplace_back(std::thread(testOneThread, n, i));
  }
  for (auto &thread : threads)
    thread.join();

  get_usage(finish);
  auto chronoEnd = std::chrono::steady_clock::now();

  struct timeval diff;
  timersub(&finish.ru_utime, &start.ru_utime, &diff);
  double time_used = diff.tv_sec + diff.tv_usec / 1000000.0;
  cout << "USER Time used: " << time_used << " s\n";

  auto chronoDuration = chronoEnd - chronoStart;
  auto chronoSeconds =
      std::chrono::duration_cast<std::chrono::microseconds>(chronoDuration);
  cout << "REAL Time used: " << chronoSeconds.count() / 1'000'000.0 << " s\n";

  double mem_used = finish.ru_maxrss / 1024.0;
  cout << "Memory used: " << mem_used << " MB\n";

  auto mem_required = THREADS_NUM * n * sizeof(Node) / 1024.0 / 1024.0;
  cout << "Memory required: " << mem_required << " MB\n";

  auto overhead = (mem_used - mem_required) * double(100) / mem_used;
  cout << "Overhead: " << std::fixed << std::setw(4) << std::setprecision(1)
       << overhead << "%\n";
}

int main(const int argc, const char *argv[]) {
  test(10'000'000);
  return EXIT_SUCCESS;
}

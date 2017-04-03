#include "monolog.h"
#include "gtest/gtest.h"

#include <thread>

class MonoLogTest : public testing::Test {
 public:
  const uint64_t kArraySize = (1024ULL * 1024ULL);  // 1 KBytes

  template<typename DS>
  void monolog_test(DS& ds) {
    for (uint64_t i = 0; i < kArraySize; i++) {
      ds.set(i, i);
    }

    for (uint64_t i = 0; i < kArraySize; i++) {
      ASSERT_EQ(ds.get(i), i);
    }
  }

  template<typename DS>
  void monolog_test_mt(DS& ds, uint32_t num_threads) {
    std::vector<std::thread> workers;
    for (uint32_t i = 1; i <= num_threads; i++) {
      workers.push_back(std::thread([i, &ds, this] {
        for (uint32_t j = 0; j < kArraySize; j++) {
          ds.push_back(i);
        }
      }));
    }

    for (std::thread& worker : workers) {
      worker.join();
    }

    ASSERT_EQ(kArraySize * num_threads, ds.size());
    std::vector<uint32_t> counts(num_threads, 0);
    for (uint32_t i = 0; i < ds.size(); i++) {
      uint64_t val = ds.at(i);
      ASSERT_TRUE(val >= 1 && val <= num_threads);
      counts[val - 1]++;
    }

    for (uint32_t count : counts) {
      ASSERT_EQ(kArraySize, count);
    }
  }
};

TEST_F(MonoLogTest, MonoLogBaseBaseTest) {
  monolog::monolog_base<uint64_t> array;
  monolog_test(array);
}

TEST_F(MonoLogTest, MonoLogConsistentTest) {
  monolog::monolog_write_stalled<uint64_t> array;
  monolog_test(array);

  for (uint32_t num_threads = 1; num_threads <= 4; num_threads++) {
    monolog::monolog_write_stalled<uint64_t> arr;
    monolog_test_mt(arr, num_threads);
  }
}

TEST_F(MonoLogTest, MonoLogRelaxedTest) {
  monolog::monolog_relaxed<uint64_t> array;
  monolog_test(array);
  for (uint32_t num_threads = 1; num_threads <= 4; num_threads++) {
    monolog::monolog_relaxed<uint64_t> arr;
    monolog_test_mt(arr, num_threads);
  }
}

TEST_F(MonoLogTest, MMapMonoLogTest) {
  monolog::mmap_monolog_relaxed<uint64_t, 8, 1048576> array("mlog", "/tmp");
  monolog_test(array);
  for (uint32_t num_threads = 1; num_threads <= 4; num_threads++) {
    monolog::mmap_monolog_relaxed<uint64_t, 8, 1048576> arr("mlog", "/tmp");
    monolog_test_mt(arr, num_threads);
  }
}

TEST_F(MonoLogTest, MonoLogBitVectorSetGetTest) {
  monolog::monolog_bitvector<> bits;

  for (uint64_t i = 0; i < kArraySize; i++) {
    bits.set_bit(i);
  }

  for (uint64_t i = 0; i < kArraySize; i++) {
    ASSERT_TRUE(bits.get_bit(i));
  }
}

TEST_F(MonoLogTest, MonoLogBitVectorMultiSetGetTest) {
  {
    // Aligned
    monolog::monolog_bitvector<> bits;
    for (uint64_t i = 0; i < kArraySize; i += 1024) {
      bits.set_bits(i, 1024);
    }

    for (uint64_t i = 0; i < kArraySize; i++) {
      ASSERT_TRUE(bits.get_bit(i));
    }
  }

  {
    // Unaligned
    monolog::monolog_bitvector<> bits;
    uint64_t max = 1000 * 1000;
    for (uint64_t i = 0; i < max; i += 1000) {
      bits.set_bits(i, 1000);
    }

    for (uint64_t i = 0; i < max; i++) {
      ASSERT_TRUE(bits.get_bit(i));
    }
  }
}

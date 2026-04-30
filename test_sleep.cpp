#include <chrono>
#include <iostream>
#include <thread>
#include <pthread.h>

struct ThreadResult {
    double sleep_for_s;
};

static void* thread_func(void* arg) {
    ThreadResult* result = (ThreadResult*)arg;
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    result->sleep_for_s = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - t1).count();
    return nullptr;
}

int main() {
    std::cout << "=== Main thread ===\n";
    {
        auto t1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - t1).count();
        std::cout << "sleep_for(1500ms): " << elapsed << " s\n";
    }

    std::cout << "\n=== Background thread ===\n";
    {
        ThreadResult result = {};
        pthread_t tid;
        pthread_create(&tid, nullptr, thread_func, &result);
        pthread_join(tid, nullptr);
        std::cout << "sleep_for(1500ms): " << result.sleep_for_s << " s\n";
    }

    std::cout << "\n=== Result ===\n";
    {
        auto t1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - t1).count();
        if (elapsed < 1.4) {
            std::cout << "FAIL: " << elapsed << "s\n";
            return 1;
        }
        std::cout << "PASS\n";
    }
    return 0;
}

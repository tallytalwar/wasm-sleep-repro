#include <chrono>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>

struct ThreadResult {
    double futex_wait_ms;
    double sleep_for_s;
};

static void* thread_func(void* arg) {
    ThreadResult* result = (ThreadResult*)arg;

    // emscripten_futex_wait on a background thread - no clamping should occur
    uint32_t dummy = 0;
    double before = emscripten_get_now();
    emscripten_futex_wait(&dummy, 0, 1500.0);
    double after = emscripten_get_now();
    result->futex_wait_ms = after - before;

    // sleep_for on a background thread
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    auto t2 = std::chrono::steady_clock::now();
    result->sleep_for_s = std::chrono::duration<double>(t2 - t1).count();

    return nullptr;
}

int main() {
    std::cout << "=== Main thread (runtime thread - clamping applies) ===\n";
    {
        uint32_t dummy = 0;
        double before = emscripten_get_now();
        emscripten_futex_wait(&dummy, 0, 1500.0);
        double after = emscripten_get_now();
        std::cout << "emscripten_futex_wait(1500ms): " << (after - before) << " ms\n";
    }
    {
        auto t1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        auto t2 = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(t2 - t1).count();
        std::cout << "sleep_for(1500ms):              " << elapsed << " s\n";
    }

    std::cout << "\n=== Background thread (no clamping should apply) ===\n";
    {
        ThreadResult result = {};
        pthread_t tid;
        pthread_create(&tid, nullptr, thread_func, &result);
        pthread_join(tid, nullptr);
        std::cout << "emscripten_futex_wait(1500ms): " << result.futex_wait_ms << " ms\n";
        std::cout << "sleep_for(1500ms):              " << result.sleep_for_s << " s\n";
    }

    // Fail if main thread sleep_for was too short
    std::cout << "\n=== Result ===\n";
    {
        auto t1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        auto t2 = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(t2 - t1).count();
        if (elapsed < 1.4) {
            std::cout << "FAIL: main thread sleep_for returned after " << elapsed << "s\n";
            return 1;
        }
        std::cout << "PASS\n";
    }
    return 0;
}

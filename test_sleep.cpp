#include <chrono>
#include <iostream>
#include <thread>
#include <emscripten/threading.h>
#include <emscripten/emscripten.h>

int main() {
    // Test 1: time emscripten_futex_wait directly on the main runtime thread.
    // On the main runtime thread, _do_futex_wait clamps max_wait_ms to
    // _emscripten_next_timer(). If a timer is pending (~1ms), this returns
    // far earlier than the requested 1500ms.
    {
        uint32_t dummy = 0;
        double before = emscripten_get_now();
        emscripten_futex_wait(&dummy, 0, 1500.0);
        double after = emscripten_get_now();
        std::cout << "emscripten_futex_wait(1500ms) on main thread took "
                  << (after - before) << " ms\n";
    }

    // Test 2: sleep_for - the full chain from std::this_thread down to futex_wait
    {
        auto t1 = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        auto t2 = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(t2 - t1).count();
        std::cout << "sleep_for(1500ms) took " << elapsed << " s\n";
        if (elapsed < 1.4) {
            std::cout << "FAIL\n";
            return 1;
        }
        std::cout << "PASS\n";
    }
    return 0;
}

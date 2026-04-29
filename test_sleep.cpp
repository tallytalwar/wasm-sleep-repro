#include <chrono>
#include <iostream>
#include <thread>

int main() {
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    auto t2 = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "Sleep for 1500ms took " << elapsed << "s\n";
    if (elapsed < 1.4) {
        std::cout << "FAIL: sleep returned too early\n";
        return 1;
    }
    std::cout << "PASS\n";
    return 0;
}

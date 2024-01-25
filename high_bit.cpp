#include <limits>
#include <type_traits>
#include <iostream>

using namespace std;

template <typename T> int high_bit(T x) {
    auto ux = make_unsigned_t<T>(x);
    int lb = -1, rb = numeric_limits<decltype(ux)>::digits;
    while (lb + 1 < rb) {
        int mid = (lb + rb) / 2;
        if (ux >> mid) {
            lb = mid;
        }
        else {
            rb = mid;
        }
    }
    return lb;
}

int main() {
    cout <<high_bit(0) <<endl;
    cout <<high_bit(1) <<endl;
    cout <<high_bit(7) <<endl;
    cout <<high_bit(8) <<endl;
    cout <<high_bit(34) <<endl;
}
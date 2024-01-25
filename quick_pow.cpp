#include <bits/stdc++.h>

using namespace std;

/*
    pow(m, n) % p
    suppose n = 13 (1101), that means pow(m, n) = pow(m, 0x1) * pow(m, 0x100) * pow(m, 0x1000)
*/
template <typename T> 
inline T pow_mod(const T m, T n, const T p) {
    T ans = 1;
    T cur = m % p;
    while (n > 0) {
        if (n & 1) {
            ans = (ans * cur) % p;
        }
        cur = cur * cur % p;
        n >>= 1;
    }
    return ans;
}

int main() {
    cout <<pow_mod(9, 0, 7) <<endl;
    cout <<pow_mod(9, 1, 7) <<endl;
    cout <<pow_mod(3, 2, 7) <<endl;
    cout <<pow_mod(2, 13, 11) <<endl;
    return 0;
}
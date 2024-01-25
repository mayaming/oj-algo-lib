#include <bits/stdc++.h>

using namespace std;

template <typename T>
class CombDigits {
public:
    const vector<T>& get(int c, int m) {
        int key = (c << BASE_SHIFT) + m;

        if (memo.count(key) == 0) {
            vector<T> v;
            if (m > 0) {
                if (m == 1) {
                    for (int i = 0; i < c; ++i) {
                        v.push_back((T)1 << i);
                    }
                }
                else if (c <= m) {
                    v.push_back(((T)1 << c)-1);
                }
                else {
                    const vector<T>& subZero = get(c - 1, m);
                    v = subZero;
                    const vector<T>& subOne = get(c - 1, m - 1);
                    for (const T t: subOne) {
                        v.push_back(((T)1 << (c-1)) | t);
                    }
                }
            }

            memo[key] = v;
        }

        return memo[key];
    }
private:
    unordered_map<int, vector<T>> memo;
    const int BASE_SHIFT = 8;
};

int main() {
    CombDigits<short> combDigits;
    for (int i = 0; i <= 5; ++i) {
        const vector<short>& v = combDigits.get(5, i);
        copy(v.begin(), v.end(), ostream_iterator<short>(cout, " "));
        cout <<endl;
    }
    /**
     * return: 7 11 13 14 19 21 22 25 26 28
     * 7: 00111
     * 11:01011
     * 13:01101
     * 14:01110
     * 19:10011
     * 21:10101
     * 22:10110
     * 25:11001
     * 26:11010
     * 28:11100
    */
    return 0;
}
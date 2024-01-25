#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <cmath>
#include <unordered_set>

using namespace std;

/**
 * https://oi-wiki.org/string/sa/
 * height[i] = LCP(sa[i], sa[i-1])
 * height[rk[i]] >= height[rk[i-1]] - 1
 * 
 * e.g. height[rk[2]] >= height[rk[1]] - 1
 * => height[7] >= height[5] - 1
 * => LCP(sa[7], sa[6]) >= LCP(sa[5], sa[4]) - 1
 * => LCP("baaaab", "b") >= LCP("abaaaab", "ab") - 1
 * => 1 >= 2 - 1
 * rk          0  1  2  3  4  5  6  7
 *             3  5  7  0  1  2  4  6
 *             a  a  b  a  a  a  a  b
 * sa   lcp
 * 0  3  0              a  a  a  a  b   
 * 1  4  3                 a  a  a  b
 * 2  5  2                    a  a  b
 * 3  0  3     a  a  b  a  a  a  a  b         
 * 4  6  1                       a  b
 * 5  1  2        a  b  a  a  a  a  b
 * 6  7  0                          b
 * 7  2  1           b  a  a  a  a  b
*/
template <typename C>
class SuffixArray {
public:
    using Entry = struct { int k1, k2, idx; };
    using EntrySorter = function<bool(const Entry&, const Entry&)>;
    EntrySorter sorter = [](const Entry &e1, const Entry &e2) { return e1.k1 < e2.k1 || e1.k1 == e2.k1 && e1.k2 < e2.k2; };

    SuffixArray(vector<C> &input): elem(input), size(input.size()), rk(input.size()), sa(input.size()), sa_lcp(input.size()) {
        vector<Entry> entries;
        int i = 0;
        transform(input.begin(), input.end(), back_inserter(entries), [&i](const C &c) { 
                return Entry{(int)c, 0, i++}; 
            }
        );
        sort(entries.begin(), entries.end(), sorter);
        refresh_rank(entries);
        init_by_sort();
    }

    /**
     * a    a    b    a    a    a    a    b
     * 0    0    1    0    0    0    0    1
     * 0,0  0,1  1,0  0,0  0,0  0,0  0,1  1,-1
     * 0    1    3    0    0    0    1    2
     * 0,3  1,0  3,0  0,0  0,1  0,2  1,-1 2,-1
     * 3    5    7    0    1    2    4    6
     * 3,1  5,2  7,4  0,6  1,-1 2,-1 4,-1 6,-1
     * 3    5    7    0    1    2    4    6 
    */
    void init_by_sort() {
        vector<Entry> entries(size);
        int step = 1;
        while (step < size) {
            for (int i = 0; i < size; ++i) {
                entries[i] = {rk[i], i + step < size ? rk[i+step] : -1, i};
            }
            sort(entries.begin(), entries.end(), sorter);
            refresh_rank(entries);
            step <<= 1;
        }

        refresh_sa();
        refresh_sa_lcp();
    }

    vector<int> get_sa() {
        return sa;
    }

    vector<int> get_sa_lcp() {
        return sa_lcp;
    }

    void print() {
        cout <<"elem   = ";
        copy(elem.begin(), elem.end(), ostream_iterator<C>(std::cout, " "));
        cout <<endl;

        cout <<"rk     = ";
        copy(rk.begin(), rk.end(), ostream_iterator<int>(std::cout, " "));
        cout <<endl;

        cout <<"sa     = ";
        copy(sa.begin(), sa.end(), ostream_iterator<int>(std::cout, " "));
        cout <<endl;

        cout <<"sa_lcp = ";
        copy(sa_lcp.begin(), sa_lcp.end(), ostream_iterator<int>(std::cout, " "));
        cout <<endl;
    }

private:
    void refresh_rank(vector<Entry> &entries) {
        for (int i = 0; i < size; ++i) {
            Entry &entry = entries[i];
            rk[entry.idx] = i == 0 ? 0 : 
                (entry.k1 == entries[i-1].k1 && entry.k2 == entries[i-1].k2 ? rk[entries[i-1].idx] : (rk[entries[i-1].idx] + 1));
        }
    }

    void refresh_sa() {
        for (int i = 0; i < size; ++i) {
            sa[rk[i]] = i;
        }
    }

    /*
                   i-1  i
                    a   b ...
                    |   |
        a b ...     |   |
        a b ...  ---|   |
        ...             |
        b ...           |
        b ...    -------|

        sa_lcp[rk[i]] >= sa_lcp[rk[i-1]] - 1
        sa_lcp[rk[i]] = lcp(sa[rk[i]], sa[rk[i]-1]) = lcp(i, sa[rk[i]-1]), sa[rk[i]-1] is the previous suffix of i
        sa_lcp[rk[i-1]] = lcp(sa[rk[i-1]], sa[rk[i-1]-1]) = lcp(i-1, sa[rk[i-1]-1]), sa[rk[i-1]-1] is the previous suffix of i - 1
    */
    void refresh_sa_lcp() {
        int common_len = 0;
        for (int idx = 0; idx < size; ++idx) {
            int sa_idx = rk[idx];

            if (sa_idx == 0) {
                common_len = 0;
            }
            else {
                int prev_idx = sa[sa_idx-1];
                while ((idx + common_len < size) && 
                    (prev_idx + common_len < size) && 
                    (elem[idx+common_len] == elem[prev_idx+common_len])) {
                    ++common_len;
                }
                sa_lcp[sa_idx] = common_len;
                common_len = max(common_len - 1, 0);
            }
        }
    }

    int size;
    vector<C> elem;
    vector<int> rk;
    vector<int> sa;
    vector<int> sa_lcp;
};

inline vector<char> convert(const string &s) {
    vector<char> v;
    for (int i = 0; i < s.size(); ++i) {
        v.push_back(s[i]);
    }
    return v;
}

int main() {
    vector<char> v = convert("aabaaaab");
    SuffixArray<char> sa(v);
    sa.print();
    return 0;
}
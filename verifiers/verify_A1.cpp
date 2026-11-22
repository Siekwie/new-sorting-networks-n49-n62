// Standalone sorting-network verifier.
// usage: verify <n> <network.txt> [cap]
// network.txt: one layer per line, e.g. [(0,1),(2,3)]  (comparators are read in file order)
// Checks 0<=i<j<n, counts size, computes ASAP depth, and proves sorting:
//   n <= 36 : exhaustive over all 2^n 0/1 inputs (bit-parallel, 64 inputs per word)
//   n  > 36 : exact product method: the longest prefix consisting of independent
//             sub-networks on disjoint channel groups (group size <= cap, default 16) is
//             evaluated exhaustively per group (2^|group| inputs -> exact output set);
//             the remaining comparators are run on the full Cartesian product of those sets.
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
typedef uint64_t u64;
struct CE { int a, b; };
int n;
std::vector<CE> nw;

static bool sorted01(u64 w) { // channel 0 must hold smallest: ones occupy the top channels
    // sorted iff bits set form a suffix block [k, n-1]
    u64 full = (n == 64) ? ~0ULL : ((1ULL << n) - 1);
    u64 inv = (~w) & full;           // zeros
    return (inv & (inv + 1)) == 0;   // zeros form a prefix block [0,k-1]
}

int main(int argc, char** argv) {
    if (argc < 3) { printf("usage: verify n file [cap]\n"); return 2; }
    n = atoi(argv[1]);
    int cap = argc > 3 ? atoi(argv[3]) : 16;
    std::ifstream f(argv[2]);
    if (!f) { printf("cannot open file\n"); return 2; }
    std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    std::vector<long> nums;
    for (size_t i = 0; i < s.size();) {
        if (isdigit((unsigned char)s[i])) { long v = 0; while (i < s.size() && isdigit((unsigned char)s[i])) v = v * 10 + (s[i++] - '0'); nums.push_back(v); }
        else if (s[i] == '-') { printf("FAIL: negative number\n"); return 1; }
        else i++;
    }
    if (nums.size() % 2) { printf("FAIL: odd count of numbers\n"); return 1; }
    for (size_t i = 0; i < nums.size(); i += 2) {
        long a = nums[i], b = nums[i + 1];
        if (!(0 <= a && a < b && b < n)) { printf("FAIL: bad comparator (%ld,%ld)\n", a, b); return 1; }
        nw.push_back({ (int)a, (int)b });
    }
    // depth (ASAP)
    std::vector<int> last(n, 0); int depth = 0;
    for (auto& c : nw) { int l = std::max(last[c.a], last[c.b]) + 1; last[c.a] = last[c.b] = l; depth = std::max(depth, l); }
    printf("n=%d size=%zu depth=%d\n", n, nw.size(), depth);

    if (n <= 36) {
        // exhaustive: low 6 bits vary inside a word, the remaining n-6 bits enumerate blocks
        static const u64 pat[6] = { 0xAAAAAAAAAAAAAAAAULL,0xCCCCCCCCCCCCCCCCULL,0xF0F0F0F0F0F0F0F0ULL,0xFF00FF00FF00FF00ULL,0xFFFF0000FFFF0000ULL,0xFFFFFFFF00000000ULL };
        int low = std::min(n, 6);
        u64 blocks = 1ULL << (n - low);
        u64 validmask = (n >= 6) ? ~0ULL : ((1ULL << (1 << n)) - 1);
        std::vector<u64> v(n);
        for (u64 blk = 0; blk < blocks; blk++) {
            for (int i = 0; i < low; i++) v[i] = pat[i];
            for (int i = low; i < n; i++) v[i] = ((blk >> (i - low)) & 1) ? ~0ULL : 0ULL;
            for (auto& c : nw) { u64 x = v[c.a], y = v[c.b]; v[c.a] = x & y; v[c.b] = x | y; }
            u64 bad = 0;
            for (int i = 0; i + 1 < n; i++) bad |= v[i] & ~v[i + 1]; // a 1 above a 0 => unsorted
            if (bad & validmask) { printf("FAIL: unsorted output found (block %llu)\n", (unsigned long long)blk); return 1; }
        }
        printf("PASS: all 2^%d inputs sorted (exhaustive)\n", n);
        return 0;
    }
    // product method
    std::vector<int> grp(n); for (int i = 0; i < n; i++) grp[i] = i;
    std::vector<int> gsize(n, 1);
    std::vector<char> dirty(n, 0);
    std::vector<CE> pre, tail;
    auto find = [&](int x) { while (grp[x] != x) x = grp[x] = grp[grp[x]]; return x; };
    for (auto& c : nw) {
        int ga = find(c.a), gb = find(c.b);
        bool ok = !dirty[c.a] && !dirty[c.b] && (ga == gb || gsize[ga] + gsize[gb] <= cap);
        if (ok) { if (ga != gb) { grp[ga] = gb; gsize[gb] += gsize[ga]; } pre.push_back(c); }
        else { dirty[c.a] = dirty[c.b] = 1; tail.push_back(c); }
    }
    // a channel becoming dirty makes later prefix comparators on it impossible (checked above);
    // but a prefix comparator must also not follow a tail comparator on the same channel: guaranteed by dirty flags.
    std::vector<std::vector<int>> groups;
    { std::vector<int> id(n, -1);
      for (int i = 0; i < n; i++) { int r = find(i); if (id[r] < 0) { id[r] = (int)groups.size(); groups.push_back({}); } groups[id[r]].push_back(i); } }
    std::vector<std::vector<u64>> outs(groups.size());
    double prod = 1;
    for (size_t g = 0; g < groups.size(); g++) {
        auto& ch = groups[g]; int k = (int)ch.size();
        std::vector<int> pos(n, -1); for (int i = 0; i < k; i++) pos[ch[i]] = i;
        std::vector<CE> loc; for (auto& c : pre) if (pos[c.a] >= 0) loc.push_back({ pos[c.a], pos[c.b] });
        std::vector<u64> res; res.reserve(1ULL << k);
        for (u64 x = 0; x < (1ULL << k); x++) {
            u64 w = x;
            for (auto& c : loc) { u64 ba = (w >> c.a) & 1, bb = (w >> c.b) & 1; if (ba & ~bb & 1) { w ^= (1ULL << c.a) | (1ULL << c.b); } }
            u64 full = 0; for (int i = 0; i < k; i++) if ((w >> i) & 1) full |= 1ULL << ch[i];
            res.push_back(full);
        }
        std::sort(res.begin(), res.end()); res.erase(std::unique(res.begin(), res.end()), res.end());
        outs[g] = res; prod *= (double)res.size();
        printf("group %zu: %d channels, %zu prefix-output patterns\n", g, k, res.size());
    }
    printf("prefix comparators=%zu tail comparators=%zu product size=%.0f\n", pre.size(), tail.size(), prod);
    if (prod > 4e10) { printf("FAIL: product too large for this verifier\n"); return 1; }
    // enumerate the product with an odometer, 64 vectors per batch
    std::vector<size_t> idx(groups.size(), 0);
    std::vector<u64> v(n); u64 total = 0; bool done = false;
    while (!done) {
        std::fill(v.begin(), v.end(), 0); int cnt = 0;
        while (cnt < 64 && !done) {
            u64 w = 0; for (size_t g = 0; g < groups.size(); g++) w |= outs[g][idx[g]];
            for (int i = 0; i < n; i++) if ((w >> i) & 1) v[i] |= 1ULL << cnt;
            cnt++; total++;
            size_t g = 0; while (g < groups.size()) { if (++idx[g] < outs[g].size()) break; idx[g] = 0; g++; }
            if (g == groups.size()) done = true;
        }
        for (auto& c : tail) { u64 x = v[c.a], y = v[c.b]; v[c.a] = x & y; v[c.b] = x | y; }
        u64 bad = 0; for (int i = 0; i + 1 < n; i++) bad |= v[i] & ~v[i + 1];
        u64 mask = cnt == 64 ? ~0ULL : ((1ULL << cnt) - 1);
        if (bad & mask) { printf("FAIL: unsorted output found\n"); return 1; }
    }
    printf("PASS: tail sorts all %llu vectors of the exact prefix-output product (covers all 2^%d inputs)\n", (unsigned long long)total, n);
    (void)sorted01;
    return 0;
}

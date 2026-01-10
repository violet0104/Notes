#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

using namespace std;

int main() {
    int ia[6] = {22, 26, 55, 47, 69};
    vector<int> vi(ia, ia + 5);

    cout << count_if(vi.begin(), vi.end(), not1(bind2nd(less<int>(), 40))) << endl;
    return 0;
}
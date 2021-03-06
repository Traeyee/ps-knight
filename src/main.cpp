#include <cmath>

#include "cpputil/common/conf.h"

#include "ps/ps.h"

using namespace ps;

void StartServer() {
    if (!IsServer()) {
        return;
    }
    std::cout << "start of server\n";
    auto server = new KVServer<float>(0);
    server->set_request_handle(KVServerDefaultHandle<float>());
    RegisterExitCallback([server]() { delete server; });
    std::cout << "end of server\n";
}

void RunWorker() {
    if (!IsWorker()) return;
    KVWorker<float> kv(0, 0);

    // init
    int num = 10000;
    std::vector<Key> keys(num);
    std::vector<float> vals(num);

    int rank = MyRank();
    srand(rank + 7);
    for (int i = 0; i < num; ++i) {
        keys[i] = kMaxKey / num * i + rank;
        vals[i] = (rand() % 1000);
    }

    // push
    int repeat = 5;
    std::vector<int> ts;
    for (int i = 0; i < repeat; ++i) {
        ts.push_back(kv.Push(keys, vals));

        // to avoid too frequency push, which leads huge memory usage
        if (i > 10) kv.Wait(ts[ts.size() - 10]);
    }
    for (int t : ts) kv.Wait(t);

    // pull
    std::vector<float> rets;
    kv.Wait(kv.Pull(keys, &rets));

    float res = 0;
    for (int i = 0; i < num; ++i) {
        res += fabs(rets[i] - vals[i] * repeat);
    }
    CHECK_LT(res / repeat, 1e-5);
    LL << "error: " << res / repeat;

    for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << MyRank() << " key [" << keys[i] << "] vals [";
        for (const auto ind_ret : rets) {
            std::cout << " " << ind_ret;
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {
    cpputil::program::Conf conf("debug.conf");
    auto role = conf.get("role");
    std::cout << role << std::endl;
    Start(0);

    if ("server" == role) {
        std::cout << "1s\n";
        std::cout << "setup server nodes\n";
        // setup server nodes
        StartServer();
    } else if ("worker" == role) {
        std::cout << "2w\n";
    } else {
        std::cout << "3else\n";
    }

    Finalize(0, true);
    return 0;
}

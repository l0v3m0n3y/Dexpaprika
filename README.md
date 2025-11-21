# Dexpaprika
api for dexpaprika.com crypto dex site
# main
```cpp
#include "Dexpaprika.h"
#include <iostream>

int main() {
   Dexpaprika api;

    auto networks = api.networks_list().then([](json::value result) {
        std::cout << result<< std::endl;
    });
    networks.wait();
    
    return 0;
}
```

# Launch (your script)
```
g++ -std=c++11 -o main main.cpp -lcpprest -lssl -lcrypto -lpthread -lboost_system -lboost_chrono -lboost_thread
./main
```

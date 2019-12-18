#ifndef __INSERTION_ORDERED_MAP__
#define __INSERTION_ORDERED_MAP__

#include <iostream>
#include <list>
#include <utility>
#include <unordered_map>
#include <memory>

using namespace std;

class lookup_error : public std::exception {
    [[nodiscard]] const char *what() const noexcept override {
        return "Key does not exist";
    }
};

template <class K, class V, class Hash = std::hash<K>>
class insertion_ordered_map {
private:
    shared_ptr<list<pair<K, V>>> values_with_keys;
    shared_ptr<unordered_map<K, typename list<pair<K, V>>::iterator>> map_ptr;

public:
    insertion_ordered_map() {
        values_with_keys = make_shared<list<pair<K, V>>>();
        map_ptr = make_shared<unordered_map<K, typename list<pair<K, V>>::iterator>>();
    }

};

#endif /* __INSERTION_ORDERED_MAP__ */

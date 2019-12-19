#ifndef __INSERTION_ORDERED_MAP__
#define __INSERTION_ORDERED_MAP__

#include <iostream>
#include <list>
#include <utility>
#include <unordered_map>
#include <unordered_set>
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
    shared_ptr<unordered_set<K>> referenced; // if there are elements, we cannot copy-on-write
    void copy(insertion_ordered_map &copied) {
        // here we have to add copying TODO
        referenced = make_shared<unordered_set<K>>();
    }

public:
    insertion_ordered_map() {
        values_with_keys = make_shared<list<pair<K, V>>>();
        map_ptr = make_shared<unordered_map<K, typename list<pair<K, V>>::iterator>>();
        referenced = make_shared<unordered_set<K>>();
    }

    insertion_ordered_map(insertion_ordered_map const &other) {
        if (!referenced->size()) {
            map_ptr = other.map_ptr;
            values_with_keys = other.values_with_keys;
            referenced = make_shared<unordered_set<K>>();
        }
        else {
            copy(other);
        }
    }

    insertion_ordered_map(insertion_ordered_map &&other) = default;

    insertion_ordered_map &operator=(insertion_ordered_map other) = default;

    bool insert(K const &k, V const &v) {

    }

    void erase(K const &k) {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->erase(k);

            if (map_ptr.use_count() > 2)
                copy(this);

            map_ptr->erase(k);

            // we have erase k from list some way TODO
        }
    }

    void merge(insertion_ordered_map const &other) {

    }

    V &at(K const &k) {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->insert(k);
            if (map_ptr.use_count() > 2)
                copy(this);
            // we have to return ref TODO
        }
    }

    V const &at(K const &k) const {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->insert(k);
            if (map_ptr.use_count() > 2)
                copy(this);
            // we have to return ref TODO
        }
    }

    V &operator[](K const &k) {

    }

    size_t size() const {
        return map_ptr->size();
    }

    bool empty() const {
        return size() == 0;
    }

    void clear() {
        values_with_keys = make_shared<list<pair<K, V>>>();
        map_ptr = make_shared<unordered_map<K, typename list<pair<K, V>>::iterator>>();
        referenced = make_shared<unordered_set<K>>();
    }

    bool contains(K const &k) const {
        return map_ptr->find(k) != (*map_ptr).end();
    }


};

#endif /* __INSERTION_ORDERED_MAP__ */

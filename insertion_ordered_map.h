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
        referenced = make_shared<unordered_set<K>>();
        shared_ptr<list<pair<K, V>>> old_list = values_with_keys;
        map_ptr = make_shared<unordered_map<K, typename list<pair<K, V>>::iterator>>();
        values_with_keys = make_shared<list<pair<K, V>>>();

        for (pair<K,V> i: old_list) {
            insert(i.first, i.second);
        }
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
        if (!contains(k) || (end() - 2)->first != k) {
            if (map_ptr.use_count() > 2)
                copy(this);

            if (!contains(k)) {
                typename list<pair<K, V>>::iterator i = values_with_keys->insert(make_pair(k, v));
                map_ptr->insert(k, i);
            }
            else {
                V second = map_ptr[k]->second;
                values_with_keys->erase(map_ptr[k]);
                typename list<pair<K, V>>::iterator i = values_with_keys->insert(make_pair(k, second));
                map_ptr->erase(k);
                map_ptr->insert(k, i);
            }
        }
    }

    void erase(K const &k) {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->erase(k);

            if (map_ptr.use_count() > 2)
                copy(this);

            map_ptr->erase(k);
            values_with_keys->erase(k);
        }
    }

    void merge(insertion_ordered_map const &other) {
        if (map_ptr.use_count() > 2)
            copy(this);

        for (K j: other.map_ptr) {
            if (!this->contains(j)) {
                insert(j, other[j]->second);
            }
        }
    }

    V &at(K const &k) {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->insert(k);
            if (map_ptr.use_count() > 2)
                copy(this);
            return &(map_ptr[k]->second);
        }
    }

    V const &at(K const &k) const {
        if (!contains(k))
            throw lookup_error();
        else {
            referenced->insert(k);
            if (map_ptr.use_count() > 2)
                copy(this);
            return &(map_ptr[k]->second);
        }
    }

    V &operator[](K const &k) {
        referenced->insert(k);

        if (map_ptr.use_count() > 2)
            copy(this);

        if (!contains(k)) {
            insert(k, V());
        }
        return &(map_ptr[k]->second);
    }

    [[nodiscard]] size_t size() const {
        return map_ptr->size();
    }

    [[nodiscard]] bool empty() const {
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

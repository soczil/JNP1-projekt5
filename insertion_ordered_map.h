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
    using my_list = list<pair<K, V>>;
    using my_unordered_map = unordered_map<K, typename my_list::iterator, Hash>;

    shared_ptr<my_list> keys_and_values;
    shared_ptr<my_unordered_map> map;
    bool need_for_copy;

    void copy_list() {
        auto new_list = make_shared<my_list>(*keys_and_values);
        keys_and_values = new_list;
    }

    void copy_map() {
        auto new_map = make_shared<my_unordered_map>();
        auto list_iterator = keys_and_values.get()->begin();
        pair<K, typename my_list::iterator> p;

        while (list_iterator != keys_and_values.get()->end()) {
            p = make_pair(list_iterator->first, list_iterator);
            new_map.get()->insert(p);
            ++list_iterator;
        }

        map = new_map;
    }

    void copy() {
        auto old_list = keys_and_values.get();
        auto old_map = map.get();

        try {
            copy_list();
            copy_map();
        } catch (exception &e) {
            keys_and_values.reset(old_list);
            map.reset(old_map);
            throw e;
        }
    }

    void insert_new(K const &k, V const &v) {
        auto pair = make_pair(k, v);

        try {
            keys_and_values.get()->push_back(pair);
            auto position = keys_and_values.get()->end();
            --position;
            auto new_element = make_pair(k, position);
            map.get()->insert(new_element);
        } catch (exception &e) {
            if (keys_and_values.get()->back() == pair) {
                keys_and_values.get()->pop_back();
            }
            throw e;
        }
    }

    void insert_old(K const &k) {
        auto element = map.get()->find(k);
        auto old_position = element->second;
        V v = old_position->second;
        auto new_pair = make_pair(k, v);

        try {
            keys_and_values.get()->push_back(new_pair);
            auto new_position = keys_and_values.get()->end();
            --new_position;
            element->second = new_position;
            keys_and_values.get()->erase(old_position);
        } catch (exception &e) {
            throw e;
        }
    }

public:
    ~insertion_ordered_map() = default;

    insertion_ordered_map() {
        keys_and_values = make_shared<list<pair<K, V>>>();
        map = make_shared<my_unordered_map>();
        need_for_copy = false;
    }

    insertion_ordered_map(insertion_ordered_map const &other) {
        keys_and_values = other.keys_and_values;
        map = other.map;
        need_for_copy = false;

        if (other.need_for_copy) {
            copy();
        }
    }

    insertion_ordered_map(insertion_ordered_map &&other) noexcept {
        keys_and_values = other.keys_and_values;
        map = other.map;
        need_for_copy = false;
    }

    insertion_ordered_map &operator=(insertion_ordered_map other) {
        keys_and_values = other.keys_and_values;
        map = other.map;
        need_for_copy = false;

        return *this;
    }

    bool insert(K const &k, V const &v) {
        if (!map.unique()) {
            copy();
        }

        auto old_element = map.get()->find(k);
        if (old_element == map.get()->end()) {
            insert_new(k, v);
            return true;
        } else {
            insert_old(k);
            return false;
        }

        need_for_copy = false;
    }

    void erase(K const &k) {
        if (!map.unique()) {
            copy();
        }

        auto element = map.get()->find(k);
        if (element == map.get()->end()) {
            throw lookup_error();
        } else {
            keys_and_values.get()->erase(element->second);
            map.get()->erase(element);
        }

        need_for_copy = false;
    }

    void merge(insertion_ordered_map const &other) {
        if (!map.unique()) {
            copy();
        }

        auto other_iterator = other.keys_and_values.get()->begin();
        while (other_iterator != other.keys_and_values.get()->end()) {
            insert(other_iterator->first, other_iterator->second);
            ++other_iterator;
        }
    }

    V &at(K const &k) {
        auto element = map.get()->find(k);

        if (element == map.get()->end()) {
            throw lookup_error();
        }

        if (!map.unique()) {
            copy();
        }
        element = map.get()->find(k);
        need_for_copy = true;

        return element->second->second;
    }

    V const &at(K const &k) const {
        return at(k);
    }

    V &operator[](K const &k) {
        if (!map.unique()) {
            copy();
        }

        auto element = map.get()->find(k);
        if (element == map.get()->end()) {
            insert(k, V());
        }

        return at(k);
    }

    void clear() {
        auto old_list = keys_and_values.get();
        auto old_map = map.get();

        try {
            if (!map.unique()) {
                // making new list and map
                keys_and_values = make_shared<my_list>();
                map = make_shared<my_unordered_map>();
            } else {
                // clearing existing list and map
                keys_and_values.get()->clear();
                map.get()->clear();
            }
        } catch (exception &e) {
            keys_and_values.reset(old_list);
            map.reset(old_map);
            throw e;
        }
        need_for_copy = false;
    }

    [[nodiscard]] size_t size() const {
        return map.get()->size();
    }

    [[nodiscard]] bool empty() const {
        return map.get()->empty();
    }

    bool contains(K const &k) const {
        return (map.get()->find(k) != map.get()->end());
    }

    class iterator {
    private:
        typename my_list::iterator it;

    public:
        ~iterator() = default;

        iterator() = default;

        iterator(const iterator &other) {
            it = other.it;
        }

        explicit iterator(const typename my_list::iterator &other) {
            it = other;
        }

        iterator &operator++() {
            ++it;
            return *this;
        }

        pair<K, V> &operator*() {
            return *it;
        }

        pair<K, V> *operator->() {
            return &(*it);
        }

        bool operator==(const iterator &other) {
            return (it == other.it);
        }

        bool operator!=(const iterator &other) {
            return (it != other.it);
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return (a.it == b.it);
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return (a.it != b.it);
        }
    };

    iterator begin() const {
        iterator it (keys_and_values.get()->begin());
        return it;
    }

    iterator end() const {
        iterator it (keys_and_values.get()->end());
        return it;
    }

};

#endif /* __INSERTION_ORDERED_MAP__ */

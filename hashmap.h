#pragma once

#include <vector>
#include <ostream>
#include <istream>
using namespace std;

class hashmap
{
public:
    hashmap();
    ~hashmap();

    int get(int key) const;
    void put(int key, int value);
    bool containsKey(int key);
    vector<int> keys() const;
    int size();

    void sanityCheck();
    hashmap(const hashmap &myMap); // copy constructor
    hashmap& operator= (const hashmap &myMap); // equals operator
    // overloads the << operator, which is VERY useful printing the hashmap
    // or writing it to a stream/file.
    friend ostream &operator<<(ostream &out, hashmap &myMap);
    // overloads the >> operator, which is VERY useful for extracting it from
    // streams/files.
    friend istream &operator>>(istream &in, hashmap &myMap);
private:
    struct key_val_pair {
        int key;
        int value;
        key_val_pair* next;
    };

    typedef key_val_pair** bucketArray; 

    bucketArray createBucketArray(int nBuckets);
    int hashFunction(int input) const;

    bucketArray buckets;

    int nBuckets;
    int nElems;
};

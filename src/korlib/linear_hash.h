#pragma once

#include "common.h"

template <class key_type, class value_type, typename Hash = std::hash<key_type>, typename Pred = std::equal_to<key_type> > class LinearHash
{
  Hash hash;
  Pred pred;


  typedef pair<key_type, uint> key_index_pair;
  typedef pair<key_type, value_type> kay_value_type;

  key_index_pair* hash_table;
  uint hash_table_size;
  //uint hash_table_size_log2;

  value_type* value_storage;
  uint value_storage_num_values;

  float fill_factor;
  uint capacity;

  key_type zero_key;

  inline bool key_is_empty(const key_type &key)
  {
    return memcmp(&zero_key, &key, sizeof(key_type)) == 0;
  }

  bool dispose_value_storage;

  LinearHash& operator=(const LinearHash& val) {}
  LinearHash(const LinearHash& val) {}

 public:

  void GetAllValues(value_type* &_value_storage, uint &num_values)
  {
    _value_storage = value_storage;
    num_values = value_storage_num_values;
    dispose_value_storage = false;
  }

  void Reset()
  {
    if (dispose_value_storage == false)
    {
      value_storage = new value_type[capacity];
    }

    memset(hash_table, 0, hash_table_size * sizeof(key_index_pair));

    value_storage_num_values = 0;

    dispose_value_storage = true;
  }

  LinearHash(uint initial_hash_table_size_log2 = 4, float _fill_factor = 0.5f): fill_factor(_fill_factor), dispose_value_storage(true)
  {
    hash_table_size = 1;
    for (uint i = 0; i < initial_hash_table_size_log2; i++)
      hash_table_size *= 2;

    //hash_table_size_log2 = initial_hash_table_size_log2;

    hash_table = new key_index_pair[hash_table_size];
    capacity = (uint)(hash_table_size * fill_factor);
    value_storage = new value_type[capacity];
    value_storage_num_values = 0;

    memset(&zero_key, 0, sizeof(key_type));
  }

  ~LinearHash()
  {
    delete[] hash_table;

    if (dispose_value_storage)
      delete[] value_storage;
  }

  void Rehash()
  {
    uint new_ht_size = hash_table_size * 2;
    //uint new_ht_size_log2 = hash_table_size_log2 + 1;
    key_index_pair* hash_new = new key_index_pair[new_ht_size];

    memset(hash_new, 0, new_ht_size * sizeof(key_index_pair));

    for (uint i = 0; i < hash_table_size; i++)
    {
      key_type &key = hash_table[i].first;

      if (key_is_empty(key))
        continue;

      uint bucket = hash(key) & (new_ht_size - 1);

      while (! key_is_empty(hash_new[bucket].first))
      {
        bucket++;

        if (bucket == new_ht_size)
          bucket = 0;
      }

      hash_new[bucket] = hash_table[i];
    }

    delete[] hash_table;
    hash_table = hash_new;
    hash_table_size = new_ht_size;
    //hash_table_size_log2 = new_ht_size_log2;

    uint new_capacity = (uint)(hash_table_size * fill_factor);

    value_type* value_storage_new = new value_type[new_capacity];
    memcpy(value_storage_new, value_storage, capacity * sizeof(value_type));
    delete[] value_storage;
    value_storage = value_storage_new;
    capacity = new_capacity;
  }

  void Insert(const key_type &key, const value_type &value, value_type* &value_pointer_returned_if_value_already_present)
  {
    value_pointer_returned_if_value_already_present = nullptr;
    assert(dispose_value_storage == true);

    uint value_index = value_storage_num_values ++;
    value_storage[value_index] = value;

    uint bucket = hash(key) & (hash_table_size - 1);

    while (! key_is_empty(hash_table[bucket].first))
    {
      if (pred(hash_table[bucket].first, key))
      {
        value_pointer_returned_if_value_already_present = &(value_storage[hash_table[bucket].second]);
        return;
      }

      bucket++;
      if (bucket == hash_table_size)
        bucket = 0;
    }

    hash_table[bucket] = make_pair(key, value_index);

    if (value_storage_num_values >= capacity)
      Rehash();
  }

  bool GetValue(const key_type &key, value_type &ret)
  {
    uint bucket = hash(key) & (hash_table_size - 1);

    while (true)
    {
      if (key_is_empty(hash_table[bucket].first))
        return false;

      if (pred(key, hash_table[bucket].first))
      {
        ret = value_storage[hash_table[bucket].second];
        return true;
      }

      bucket++;

      if (bucket == hash_table_size)
        bucket = 0;
    }
  }

  static void TestIt()
  {
    LinearHash<int, int> lhash;

    unordered_map<int, int> umap;

    int* dummy;

    for (uint i = 0; i < 20000; i++)
    {
      int rand_key = rand();
      int rand_val = rand();

      lhash.Insert(rand_key, rand_val, dummy);
      umap.insert(make_pair(rand_key, rand_val));
    }

    for (int i = 0; i < RAND_MAX; i++)
    {
      auto fit = umap.find(i);
      bool umap_contain = (fit != umap.end());
      int lhash_value;
      bool lhash_contain = lhash.GetValue(i, lhash_value);

      if (lhash_contain != umap_contain) exit(-124);

      if (lhash_contain && umap_contain)
      {
        int umap_value = fit->second;

        if (lhash_value != umap_value)
        {
          exit(-123);
        }
      }
    }

    printf("TestOK!!!\n");
  }
};


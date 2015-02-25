/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

#ifndef STAGEPOSIBILITY_CASH2_HPP_
#define STAGEPOSIBILITY_CASH2_HPP_

//#include "StdAfx.h"

#include <unordered_map>
#include <list>
#include <vector>
#include <stdint.h>
#include <iostream>
#include "utils.hpp"

using namespace std;

namespace ngramchecker {

template <class key_type, class value_type, typename Hash = std::hash<key_type>, typename Pred = std::equal_to<key_type> > class MyCash2
{
 private:

  Hash hash;
  Pred pred;

  struct Element;

  struct ListItem {
    ListItem* next;
    ListItem* prev;
    Element* elem_pointer;
  };

  struct Element
  {
    key_type key;
    value_type value;
    ListItem* li;
    Element* next;
  };

  vector<Element*> hash_table;
  float load_factor;
  vector<char> data;
  uint32_t max_vals;
  uint32_t hash_table_size;
  ListItem *list_top;
  ListItem *list_bottom;
  ListItem *list_stop;
  uint num_vals;
  bool filled;

 public:

  MyCash2(uint32_t _max_vals, float _load_factor): load_factor(_load_factor), max_vals(_max_vals), hash_table_size((uint32_t)(_max_vals / _load_factor))
  {
    num_vals = 0;
    hash_table.resize(hash_table_size);
    for (size_t i = 0; i < hash_table_size; i++)
      hash_table[i] = NULL;

    uint32_t data_byte_size = _max_vals * sizeof(Element);
    data.resize(data_byte_size);
    //memset?
    for (size_t i = 0; i < data_byte_size; i++)
      data[i] = 0;

    cerr << "data byte size: " << data_byte_size << endl;

    list_top= new ListItem();
    list_top->elem_pointer = (Element*)&(data[0]);

    ListItem *li = list_top;

    for (size_t i = 1; i < max_vals; i++)
    {
      ListItem *new_item = new ListItem();
      new_item->elem_pointer = li->elem_pointer + 1;//(int)sizeof(Element);
      li->prev = new_item;
      new_item->next = li;
      li = new_item;

      FATAL_CONDITION(new_item->elem_pointer->li == NULL, "");
    }

    list_stop = li;
    list_bottom = list_top;
    filled = false;
    //list_bottom = li->next;
  }

  bool GetCashedValue(const key_type &key, value_type &value)
  {
    size_t hash_code = hash(key);
    Element* el_pointer = hash_table[hash_code % hash_table_size];

    if (el_pointer == NULL)
      return false;
    else
    {
      while (!pred(el_pointer->key, key))
      {
        el_pointer = el_pointer->next;
        if (el_pointer == NULL)
          return false;
      }

      value = el_pointer->value;

      if (filled == true)
      {
        ListItem* li = el_pointer->li;
        if (li != list_top)
        {
          if (li != list_bottom)
          {
            li->prev->next = li->next;
            li->next->prev = li->prev;
          }
          else
          {
            list_bottom = li->next;
          }

          list_top->next = li;
          li->prev = list_top;
          list_top = li;
        }
      }
      return true;
    }
  }

  void StoreValueForKey(const key_type &key, const value_type &value)
  {
    Element* storage_place = list_bottom->elem_pointer;

    //num_vals++;
    //cerr << num_vals;

    if (storage_place->li != NULL)
    {
      //FATAL_CONDITION(list_bottom == storage_place->li, "");
      //under this condition list_bottom == storage_place->li

      //i.e. the storage_place aready contains data (these data will be thrown away)
      size_t bucket = hash(storage_place->key) % hash_table_size;

      Element* el_pointer = hash_table[bucket];

      if (el_pointer == storage_place)
        hash_table[bucket] = storage_place->next;
      else
      {
        //WARNING(hash_pointer->key != storage_place->key, hash_pointer << ", " << storage_place << "; key = " << storage_place->key << endl);
        while (el_pointer->next != storage_place)
          el_pointer = el_pointer->next;

        el_pointer->next = storage_place->next;
      }

      ListItem* li = list_bottom->next;
      list_top->next = list_bottom;
      list_bottom->prev = list_top;
      list_top = list_bottom;
      list_bottom = li;
    }
    else
    {
      storage_place->li = list_bottom;

      if (list_bottom != list_stop)
        list_bottom = list_bottom->prev;
      else
        filled = true;

      /*
      //yep, this is valid until the full capacity is reached!
      if (list_bottom != list_top)
      {
      ListItem* li;

      if (list_bottom != list_stop)
      li = list_bottom->prev;
      else
      {
      filled = true;
      li = list_bottom->next;
      }

      list_bottom->next->prev = list_bottom->prev;

      if (list_bottom != list_stop)
      list_bottom->prev->next = list_bottom->next;

      list_top->next = list_bottom;
      list_bottom->prev = list_top;

      list_top = list_bottom;
      list_bottom = li;
      }
      else
      {
      FATAL_CONDITION(list_bottom->prev->next == list_bottom, "");
      list_bottom = list_bottom->prev;

      }

      //cerr << "list_bottom: " << list_bottom << ", list_top: " << list_top << endl;
      //cerr << "list_bottom->el: " << list_bottom->elem_pointer << ", list_top->elem_pointer: " << list_top->elem_pointer << endl;
      */

    }

    //now the storage place should be disconnected!

    size_t hash_code = hash(key) % hash_table_size;
    storage_place->next = hash_table[hash_code];
    hash_table[hash_code] = storage_place;
    storage_place->value = value;
    storage_place->key = key;

  }

};

}

#endif /* STAGEPOSIBILITY_CASH2_HPP_ */

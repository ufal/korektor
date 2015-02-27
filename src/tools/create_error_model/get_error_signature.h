// This file is part of korektor <http://github.com/ufal/korektor/>.
//
// Copyright 2015 by Institute of Formal and Applied Linguistics, Faculty
// of Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under 3-clause BSD licence.

#pragma once

#include "common.h"
#include "utils/utils.h"

namespace ufal {
namespace korektor {

bool GetErrorSignature(u16string misspelled, u16string correct, u16string& signature)
{
  for (unsigned i = 0; i < misspelled.length(); i++)
    misspelled[i] = Utils::ToLower(misspelled[i]);

  for (unsigned i = 0; i < correct.length(); i++)
    correct[i] = Utils::ToLower(correct[i]);

  if (misspelled.length() == correct.length())
  {
    for (unsigned i = 0; i < misspelled.length(); i++)
    {
      if (misspelled[i] != correct[i] && i + 1 < misspelled.length() && misspelled[i + 1] == correct[i] && misspelled[i] == correct[i + 1])
      {
        for (unsigned j = i + 2; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j])
          {
            cerr << "!!!!!!!!!!!!!! swap failed: " << endl;
            return false;
          }
        }
        //-------------------Letter swap-------------------------------
        signature = Utils::utf8_to_utf16("swap_");
        signature += correct[i];
        signature += correct[i + 1];
        return true;
      }
      else if (misspelled[i] != correct[i])
      {
        //--------------------------------Substitution-----------------------------------
        for (unsigned j = i + 1; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j])
          {
            cerr << "!! " << misspelled[j] << " - " << correct[j] << "!! j = " << j << ", length = " << misspelled.length() << endl;
            cerr << "sub failed!: " << Utils::utf16_to_utf8(misspelled) << " -- " << Utils::utf16_to_utf8(correct) << endl;
            return false;
          }
        }
        signature = Utils::utf8_to_utf16("s_");
        signature += misspelled[i];
        signature += correct[i];
        return true;
      }
    }
  }
  else if (misspelled.length() == correct.length() + 1)
  {
    //-----------------------------------Insertion---------------------------------------
    for (unsigned i = 0; i < correct.length(); i++)
    {
      if (misspelled[i] != correct[i])
      {
        for (unsigned j = i; j < correct.length(); j++)
        {
          if (misspelled[j + 1] != correct[j])
          {
            cerr << "insert failed: i = " << i << ", j = " << j << ", correct_length = " << correct.length() << endl;
            return false;
          }
        }

        signature = Utils::utf8_to_utf16("i_");
        signature += misspelled[i];

        if (i == 0)
          signature += char16_t(' ');
        else
          signature += correct[i - 1];

        signature += correct[i];
        return true;
      }

    }

    signature = Utils::utf8_to_utf16("i_");
    signature += misspelled[misspelled.length() - 1];
    signature += correct[correct.length() - 1];
    signature += char16_t(' ');
    return true;
  }
  else if (misspelled.length() == correct.length() - 1)
  {
    //------------------------------------Deletion-------------------------------------------------
    for (unsigned i = 0; i < correct.length(); i++)
    {
      if (i == correct.length() - 1 && i > 0)
      {
        signature = Utils::utf8_to_utf16("d_");
        signature += correct[i];
        signature += correct[i - 1];
        return true;
      }

      if (misspelled[i] != correct[i])
      {
        for (unsigned j = i; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j + 1])
            return false;
        }

        signature = Utils::utf8_to_utf16("d_");
        signature += correct[i];
        if (i > 0)
          signature += correct[i - 1];
        else
          signature += char16_t(' ');
        return true;

      }
    }
  }

  return false;
}

} // namespace korektor
} // namespace ufal

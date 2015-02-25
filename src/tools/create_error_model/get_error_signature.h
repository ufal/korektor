#ifndef _GET_ERROR_SIGNATURE_HPP
#define _GET_ERROR_SIGNATURE_HPP

bool GetErrorSignature(u16string misspelled, u16string correct, u16string& signature)
{
  for (uint i = 0; i < misspelled.length(); i++)
    misspelled[i] = MyUtils::ToLower(misspelled[i]);

  for (uint i = 0; i < correct.length(); i++)
    correct[i] = MyUtils::ToLower(correct[i]);

  if (misspelled.length() == correct.length())
  {
    for (uint i = 0; i < misspelled.length(); i++)
    {
      if (misspelled[i] != correct[i] && i + 1 < misspelled.length() && misspelled[i + 1] == correct[i] && misspelled[i] == correct[i + 1])
      {
        for (uint j = i + 2; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j])
          {
            cerr << "!!!!!!!!!!!!!! swap failed: " << endl;
            return false;
          }
        }
        //-------------------Letter swap-------------------------------
        signature = MyUtils::utf8_to_utf16("swap_");
        signature += correct[i];
        signature += correct[i + 1];
        return true;
      }
      else if (misspelled[i] != correct[i])
      {
        //--------------------------------Substitution-----------------------------------
        for (uint j = i + 1; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j])
          {
            cerr << "!! " << misspelled[j] << " - " << correct[j] << "!! j = " << j << ", length = " << misspelled.length() << endl;
            cerr << "sub failed!: " << MyUtils::utf16_to_utf8(misspelled) << " -- " << MyUtils::utf16_to_utf8(correct) << endl;
            return false;
          }
        }
        signature = MyUtils::utf8_to_utf16("s_");
        signature += misspelled[i];
        signature += correct[i];
        return true;
      }
    }
  }
  else if (misspelled.length() == correct.length() + 1)
  {
    //-----------------------------------Insertion---------------------------------------
    for (uint i = 0; i < correct.length(); i++)
    {
      if (misspelled[i] != correct[i])
      {
        for (uint j = i; j < correct.length(); j++)
        {
          if (misspelled[j + 1] != correct[j])
          {
            cerr << "insert failed: i = " << i << ", j = " << j << ", correct_length = " << correct.length() << endl;
            return false;
          }
        }

        signature = MyUtils::utf8_to_utf16("i_");
        signature += misspelled[i];

        if (i == 0)
          signature += char16_t(' ');
        else
          signature += correct[i - 1];

        signature += correct[i];
        return true;
      }

    }

    signature = MyUtils::utf8_to_utf16("i_");
    signature += misspelled[misspelled.length() - 1];
    signature += correct[correct.length() - 1];
    signature += char16_t(' ');
    return true;
  }
  else if (misspelled.length() == correct.length() - 1)
  {
    //------------------------------------Deletion-------------------------------------------------
    for (uint i = 0; i < correct.length(); i++)
    {
      if (i == correct.length() - 1 && i > 0)
      {
        signature = MyUtils::utf8_to_utf16("d_");
        signature += correct[i];
        signature += correct[i - 1];
        return true;
      }

      if (misspelled[i] != correct[i])
      {
        for (uint j = i; j < misspelled.length(); j++)
        {
          if (misspelled[j] != correct[j + 1])
            return false;
        }

        signature = MyUtils::utf8_to_utf16("d_");
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

#endif

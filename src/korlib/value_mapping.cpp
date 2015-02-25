/*
Copyright (c) 2012, Charles University in Prague
All rights reserved.
*/

/// @file value_mapping.cpp
/// @class ValueMapping value_mapping.h "value_mapping.h"
/// @brief Data structure for mapping the array of values to larger
/// set of values that includes the original set as well as more values within the range of the original set.
///
/// @todo Logic is still not clear
#include "value_mapping.h"
#include "utils.h"

namespace ngramchecker {

/// @brief Get the closest value in the larger mapped set
///
/// @param value Value from the original set
/// @return Value close to the supplied parameter in the mapped set
uint32_t ValueMapping::GetCenterID(double value) const
{
  uint32_t left = 0;
  uint32_t right = sorted_centers.size() - 1;
  uint32_t index;

  //if (value < -0.9) return 0;

  while (right - left > 1)
  {
    index = (left + right) / 2;
    if (sorted_centers[index] < value)
    {
      left = index;
    }
    else
    {
      right = index;
    }
  }

  if (fabs(sorted_centers[left] - value) < fabs(sorted_centers[right] - value))
  {
    return left;
  }
  else
  {
    return right;
  }
}

/// @brief Write the mapped set data structure to the output stream
///
/// @param ofs Output stream
void ValueMapping::writeToStream(ostream &ofs) const
{
  uint32_t num_centers = sorted_centers.size();
  ofs.write((char*)&num_centers, sizeof(uint32_t));
  ofs.write((char*)&num_bits_per_value, sizeof(uint32_t));

  for (uint32_t i = 0; i < sorted_centers.size(); i++)
  {
    ofs.write((char*)&(sorted_centers[i]), sizeof(double));
  }
}

/// @brief Constructor initialization from already mapped set. Only sorting is done in the constructor.
///
/// @param centers Vector of values
ValueMapping::ValueMapping(vector<double> centers)
{
  sort(centers.begin(), centers.end());
  sorted_centers = centers;
}

/// @brief Constructor initialization from input stream
///
/// @param ifs Input stream
ValueMapping::ValueMapping(istream &ifs)
{
  uint32_t num_centers;
  ifs.read((char*)&num_centers, sizeof(uint32_t));
  ifs.read((char*)&num_bits_per_value, sizeof(uint32_t));

  double val;
  for (uint32_t i = 0; i < num_centers; i++)
  {
    ifs.read((char*)&val, sizeof(double));
    sorted_centers.push_back(val);
  }

}

/// @brief Constructor. Initializes the data structure from vector of values. Larger set is constructed from this vector.
///
/// @param values Vector of values
/// @param bits_per_value Bits per value
ValueMapping::ValueMapping(vector<double> values, const uint32_t bits_per_value): num_bits_per_value(bits_per_value)
{
  uint32_t num_classes = 1;
  for (uint32_t i = 0; i < bits_per_value; i++)
    num_classes = num_classes << 1;


  cerr << "value mapping: " << values.size() << " into " << num_classes << " classes\n";

  vector<double> centers;

  double min_pos = +100000000;
  double max = -100000000;

  for (uint32_t i = 0; i < values.size(); i++)
  {
    //if (values[i] > 30) values[i] = 30;
    if (values[i] < min_pos) min_pos = values[i];
    if (values[i] > max) max = values[i];
  }

  double pom_incr = (max - min_pos ) / (num_classes + 2);
  double pom = min_pos;

  for (uint32_t i = 0; i < num_classes; i++)
  {
    centers.push_back(pom);
    pom += pom_incr;
  }

  vector<uint32_t> valuesAssignment;

  for (uint32_t i = 0; i < values.size(); i++)
  {
    valuesAssignment.push_back(1000000);
  }

  cerr << "initialized..." << endl;

  bool change;
  double old_error = 10000000;
  double cum_error;
  uint32_t iter_counter = 0;
  uint32_t iter_max = 1;
  do {

    iter_counter++;
    change = false;
    for (uint32_t i = 0; i < values.size(); i++)
    {

      double minDist = 100;
      uint32_t candidateCenter;

      for (uint32_t j = 0; j < centers.size(); j++)
      {
        if (fabs(centers[j] - values[i]) < minDist)
        {
          candidateCenter = j;
          minDist = fabs(centers[j] - values[i]);
        }
      }

      if (candidateCenter != valuesAssignment[i])
      {
        change = true;
        valuesAssignment[i] = candidateCenter;
      }


    }

    vector<double> pomV_sum;
    vector<uint32_t> pomV_num;

    for (uint32_t i = 0; i < centers.size(); i++)
    {
      pomV_sum.push_back(0);
      pomV_num.push_back(0);
    }

    for (uint32_t i = 0; i < values.size(); i++)
    {

      pomV_sum[ valuesAssignment[i] ] += values[i];
      pomV_num[ valuesAssignment[i] ]++;
    }

    for (uint32_t i = 0; i < centers.size(); i++)
    {
      if (pomV_num[i] > 0)
      {
        centers[i] = pomV_sum[i] / pomV_num[i];
      }
      else
      {
        centers[i] = MyUtils::RandomNumber(min_pos, max);
      }
    }

    cum_error = 0;

    for (uint32_t i = 0; i < values.size(); i++)
    {
      cum_error += fabs(values[i] - centers[ valuesAssignment[i] ]);
    }
    //cout << "cummulative error: " << cum_error << endl;

    if (cum_error < old_error)
      change = true;

    if (iter_counter % 5 == 0)
      cerr << cum_error << endl;
    old_error = cum_error;

    if (iter_counter >= iter_max)
    {
      change = false;
    }

  } while (change == true);

  cerr << endl;


  sort(centers.begin(), centers.end());

  /*for (uint32_t i = 0; i < centers.size(); i++)
    {
    cout << "center " << i << ": " << centers[i] << endl;
    }*/

  cerr << "Value mapping finished!" << endl;

  for (uint32_t i = 0; i < centers.size(); i++)
    cerr << "   " << centers[i];
  cerr << endl << endl;

  sorted_centers = centers;

}
}

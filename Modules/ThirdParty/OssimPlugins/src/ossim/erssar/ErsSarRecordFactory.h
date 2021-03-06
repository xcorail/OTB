/*
 * Copyright (C) 2005-2017 by Centre National d'Etudes Spatiales (CNES)
 *
 * This file is licensed under MIT license:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef ErsSarRecordFactory_h
#define ErsSarRecordFactory_h


#include "erssar/ErsSarRecordHeader.h"
#include "erssar/ErsSarRecord.h"

#include <map>


namespace ossimplugins
{


/**
 * @ingroup ErsSar
 * @brief This class is a facory able to construct Record base classes given the id of the record wanted
 *
 */
class ErsSarRecordFactory
{
public:
  /**
   * @brief Constructor
   */
  ErsSarRecordFactory();
  /**
   * @brief Destructor
   */
  virtual ~ErsSarRecordFactory();

  /**
   * @brief Add a new Record type available in this factory
   * @param record Record to add in the factory
   * @param id Id of the new avalaible Record
   */
  void RegisterRecord(int id, ErsSarRecord * record);

  /**
   * @brief Instantiate a new Record
   * @param id Id of the Record we want to instantiate
   */
  ErsSarRecord* Instantiate(int id) ;
protected:

  /**
   * @brief Contain all the available Records for the factory
   */
  std::map<int, ErsSarRecord*> _availableRecords;

private:
};
}

#endif

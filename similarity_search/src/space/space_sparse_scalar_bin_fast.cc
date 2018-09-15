/**
 * Non-metric Space Library
 *
 * Main developers: Bilegsaikhan Naidan, Leonid Boytsov, Yury Malkov, Ben Frederickson, David Novak
 *
 * For the complete list of contributors and further details see:
 * https://github.com/searchivarius/NonMetricSpaceLib
 *
 * Copyright (c) 2013-2018
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */
#include <memory>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <limits>

#include "utils.h"
#include "space/space_sparse_scalar_bin_fast.h"
#include "experimentconf.h"

namespace similarity {

using namespace std;

unique_ptr<DataFileInputState> SpaceSparseCosineSimilarityBinFast::OpenReadFileHeader(const string& inpFileName) const {
  return unique_ptr<DataFileInputState>(new DataFileInputStateBinSparseVec(inpFileName));
}

unique_ptr<Object> SpaceSparseCosineSimilarityBinFast::CreateObjFromStr(IdType id, LabelType label, const string& s,
                                    DataFileInputState* pInpState) const {
  vector<ElemType>  vec;
  parseSparseBinVector(s, vec);
  return unique_ptr<Object>(CreateObjFromVect(id, label, vec));
}

bool SpaceSparseCosineSimilarityBinFast::ReadNextObjStr(DataFileInputState &state, string& strObj,
                                                        LabelType&, string&) const {

  return readNextBinSparseVect(dynamic_cast<DataFileInputStateBinSparseVec&>(state), strObj);
}

unique_ptr<DataFileInputState> SpaceSparseNegativeScalarProductBinFast::OpenReadFileHeader(const string& inpFileName) const {

  return unique_ptr<DataFileInputState>(new DataFileInputStateBinSparseVec(inpFileName));
}

bool SpaceSparseNegativeScalarProductBinFast::ReadNextObjStr(DataFileInputState &state, string& strObj,
                                                             LabelType& , string& ) const {

  return readNextBinSparseVect(dynamic_cast<DataFileInputStateBinSparseVec&>(state), strObj);
}

unique_ptr<Object> SpaceSparseNegativeScalarProductBinFast::CreateObjFromStr(IdType id, LabelType label, const string& s,
                                                                             DataFileInputState* pInpState) const {
  vector<ElemType>  vec;
  parseSparseBinVector(s, vec);
  return unique_ptr<Object>(CreateObjFromVect(id, label, vec));
}


}  // namespace similarity

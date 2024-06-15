#ifndef _EXTRACT_V1495_IN_TIME__H_
#define _EXTRACT_V1495_IN_TIME__H_
#include <fstream>
#include <unordered_map>
#include "ExtractHodoInTime.h"

class ExtractV1495InTime: public ExtractHodoInTime {
 public:
  ExtractV1495InTime(const std::string& name="ExtractV1495InTime");
  virtual ~ExtractV1495InTime() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif // _EXTRACT_V1495_IN_TIME__H_

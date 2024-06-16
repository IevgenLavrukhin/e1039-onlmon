#ifndef _ONL_MON_TRIG_ROAD__H_
#define _ONL_MON_TRIG_ROAD__H_
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "TriggerRoadset.h"
#include "OnlMonClient.h"

class OnlMonTrigRoad: public OnlMonClient {
 public:
  typedef enum { TOP = 0, BOTTOM = 1 } TopBottom_t;
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  TriggerRoadset roadset;

  TH1* h1_cnt;
  TH1* h1_rs_cnt[4];

  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];

 public:
  OnlMonTrigRoad();
  virtual ~OnlMonTrigRoad() {}
  OnlMonClient* Clone() { return new OnlMonTrigRoad(*this); }

  TriggerRoadset* Roadset() { return &roadset; }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void CountFiredRoads(const int top0bot1, std::vector<SQHit*>* H1X, std::vector<SQHit*>* H2X, std::vector<SQHit*>* H3X, std::vector<SQHit*>* H4X, TriggerRoads* roads, TH1* h1_rs_cnt);
  void SetDet();
};

#endif /* _ONL_MON_TRIG_ROAD__H_ */

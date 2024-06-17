#ifndef _ONL_MON_TRIG_EP__H_
#define _ONL_MON_TRIG_EP__H_
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "TriggerRoadset.h"
#include "OnlMonClient.h"

/// OnlMonClient to estimate the efficiency and the purity of FPGA trigger.
class OnlMonTrigEP: public OnlMonClient {
 public:
  typedef enum { TOP = 0, BOTTOM = 1 } TopBottom_t;
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  TriggerRoadset roadset;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];

  TH1* h1_cnt;
  TH1* h1_purity;  
  TH1* h1_eff_NIM3;
 
 public:
  OnlMonTrigEP();
  virtual ~OnlMonTrigEP() {}
  OnlMonClient* Clone() { return new OnlMonTrigEP(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void FindFiredRoads(const int top0bot1, std::vector<SQHit*>* H1X, std::vector<SQHit*>* H2X, std::vector<SQHit*>* H3X, std::vector<SQHit*>* H4X, TriggerRoads* roads, std::vector<TriggerRoad1*>& list_fired_roads);
  std::string PrintHitVec(const std::string title, const std::vector<SQHit*>* vec);
  void SetDet();
};

#endif /* _ONL_MON_TRIG_EP__H_ */

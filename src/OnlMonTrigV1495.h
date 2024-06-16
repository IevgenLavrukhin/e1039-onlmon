#ifndef _ONL_MON_TRIG_V1495__H_
#define _ONL_MON_TRIG_V1495__H_
#define DEBUG_LVL 0
//#include <rs_Reader/rs_Reader.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "TriggerRoadset.h"
#include "OnlMonClient.h"

class OnlMonTrigV1495: public OnlMonClient {
 public:
  typedef enum { TOP = 0, BOTTOM = 1 } TopBottom_t;
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  TriggerRoadset roadset;

  TH1* h1_cnt;
  TH2* h2_trig_time;
  TH2* h2_fpga_nim_time_af;  
  TH2* h2_RF;

  TH1* h1_trig_diff_TS;

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];

  TH1* h1_rs_cnt[4];

  std::vector<SQHit*>* vecH1T;
  std::vector<SQHit*>* vecH2T;
  std::vector<SQHit*>* vecH3T;
  std::vector<SQHit*>* vecH4T;

  std::vector<SQHit*>* vecH1B;
  std::vector<SQHit*>* vecH2B;
  std::vector<SQHit*>* vecH3B;
  std::vector<SQHit*>* vecH4B;
 
 public:
  OnlMonTrigV1495();
  virtual ~OnlMonTrigV1495() {}
  OnlMonClient* Clone() { return new OnlMonTrigV1495(*this); }

  TriggerRoadset* Roadset() { return &roadset; }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void CountFiredRoads(const int top0bot1, std::vector<SQHit*>* H1X, std::vector<SQHit*>* H2X, std::vector<SQHit*>* H3X, std::vector<SQHit*>* H4X, TriggerRoads* roads, TH1* h1_rs_cnt);

  void debug_print(int debug_lvl);
  double Abs(double var0, double var1);
  void SetDet();
  void FPGA_NIM_Time(std::vector<SQHit*>* FPGA, std::vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* h2, TH1* h1);
  void DrawTH2WithPeakPos(TH2* h2, const double cont_min=100);
};

#endif /* _ONL_MON_TRIG_V1495__H_ */

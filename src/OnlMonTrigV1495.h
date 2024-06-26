#ifndef _ONL_MON_TRIG_V1495__H_
#define _ONL_MON_TRIG_V1495__H_
#define DEBUG_LVL 0
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

class OnlMonTrigV1495: public OnlMonClient {
 public:
  typedef enum { TOP = 0, BOTTOM = 1 } TopBottom_t;
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  TH2* h2_trig_time;
  TH2* h2_fpga_nim_time_af;
  TH2* h2_RF;
  TH1* h1_trig_diff_TS;

  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];

 public:
  OnlMonTrigV1495();
  virtual ~OnlMonTrigV1495() {}
  OnlMonClient* Clone() { return new OnlMonTrigV1495(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  //void debug_print(int debug_lvl);
  double Abs(double var0, double var1);
  void SetDet();
  void FPGA_NIM_Time(std::vector<SQHit*>* FPGA, std::vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* h2, TH1* h1);
  void DrawTH2WithPeakPos(TH2* h2, const double cont_min=100);
};

#endif /* _ONL_MON_TRIG_V1495__H_ */

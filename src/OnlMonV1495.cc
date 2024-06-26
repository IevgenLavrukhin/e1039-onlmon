/// OnlMonV1495.C
#include <sstream>
#include <iomanip>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <UtilAna/UtilSQHit.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonParam.h"
#include "OnlMonV1495.h"
using namespace std;

OnlMonV1495::OnlMonV1495(const HodoType_t type, const int lvl)
  : m_type(type)
  , m_lvl(lvl)
  , m_trig_mask(1023) // 1023 = 0b1111111111 = all triggers
{
  NumCanvases(2);
  is_H1 = 0;
  switch (m_type) {
  case H1X:  Name("OnlMonV1495H1X" );  Title("V1495: H1X" ); is_H1 = 1;  break;
  case H2X:  Name("OnlMonV1495H2X" );  Title("V1495: H2X" );  break;
  case H3X:  Name("OnlMonV1495H3X" );  Title("V1495: H3X" );  break;
  case H4X:  Name("OnlMonV1495H4X" );  Title("V1495: H4X" );  break;
  case H1Y:  Name("OnlMonV1495H1Y" );  Title("V1495: H1Y" );  break;
  case H2Y:  Name("OnlMonV1495H2Y" );  Title("V1495: H2Y" );  break;
  case H4Y1: Name("OnlMonV1495H4Y1");  Title("V1495: H4Y1");  break;
  case H4Y2: Name("OnlMonV1495H4Y2");  Title("V1495: H4Y2");  break;
  }
}

int OnlMonV1495::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::InitRunOnlMon(PHCompositeNode* topNode)
{
  h1_cnt = new TH1D("h1_cnt", ";Type;Count", 15, 0.5, 15.5);
  RegisterHist(h1_cnt);

  OnlMonParam param("OnlMonV1495");
  m_trig_mask = param.GetIntParam("TRIGGER_MASK");

  const double DT = 2.0; // 1 ns per single count of v1495 TDC
  int NT    = 1000;
  double T0 =    0.5*DT;
  double T1 = 1000.5*DT;
  switch (m_type) {
  case H1X:  SetDet("H1T"  ,"H1B"  ); break;
  case H2X:  SetDet("H2T"  ,"H2B"  ); break;
  case H3X:  SetDet("H3T"  ,"H3B"  ); break;
  case H4X:  SetDet("H4T"  ,"H4B"  ); break;
  case H1Y:  SetDet("H1L"  ,"H1R"  ); break;
  case H2Y:  SetDet("H2L"  ,"H2R"  ); break;
  case H4Y1: SetDet("H4Y1L","H4Y1R"); break;
  case H4Y2: SetDet("H4Y2L","H4Y2R"); break;
  }

  GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  for (int i_det = 0; i_det < N_DET; i_det++) {
    string name = list_det_name[i_det];
    int  det_id = list_det_id  [i_det];
    int n_ele  = geom->getPlaneNElements(det_id);
    if (det_id <= 0 || n_ele <= 0) {
      cout << "OnlMonV1495::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

    oss.str("");
    oss << "h1_ele_" << i_det;
    h1_ele[i_det] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;Hit count";
    h1_ele[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_ele_in_" << i_det;
    h1_ele_in[i_det] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;In-time hit count";
    h1_ele_in[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_" << i_det;
    h1_time[i_det] = new TH1D(oss.str().c_str(), "", NT, T0, T1);

    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_in_" << i_det;
    h1_time_in[i_det] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;In-time hit count";
    h1_time_in[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h2_time_ele_" << i_det;
    h2_time_ele[i_det] = new TH2D(oss.str().c_str(), "",n_ele, 0.5, n_ele+0.5, 80, 680.5, 840.5);
    oss.str("");
    oss << name << ";Element ID;tdcTime;Hit count";
    h2_time_ele[i_det]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele    [i_det]);
    RegisterHist(h1_ele_in [i_det]);
    RegisterHist(h1_time   [i_det]);
    RegisterHist(h1_time_in[i_det]);
    RegisterHist(h2_time_ele[i_det]);
  }

  for(int i = 0; i < 8; i++){
    oss.str("");
    oss << "RF_proj_" << i;
    RF_proj[i] = new TH1D(oss.str().c_str(), "",160 , 680.5, 840.5);
    oss.str("");
    oss << "RF TDC Projection" << ";TDC RF;Hit count";
    RF_proj[i]->SetTitle(oss.str().c_str());
  
    RegisterHist(RF_proj[i]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector* hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  h1_cnt->AddBinContent(1);
  
  unsigned short trig_bits = evt->get_trigger();
  if (! (trig_bits & m_trig_mask)) return Fun4AllReturnCodes::EVENT_OK;

  h1_cnt->AddBinContent(2);

//RF ***************************************************************************************
//Looping through RF data to determine RF buckets
  auto vec1 = UtilSQHit::FindTriggerHitsFast(evt, hit_vec, "RF");
  int count = 0;
  for(auto it = vec1->begin(); it != vec1->end(); it++){
    double tdc_time = (*it)->get_tdc_time();
    float element = (*it)->get_element_id();
    //Fill 1D histograms with RF data to determine RF buckets
    if(element == 3 || element == 4){
      int cnt_mod = count%8;
      RF_proj[cnt_mod]->Fill(tdc_time);
    }
    count ++;
  }


//Hodoscope Hits ***************************************************************************
  for (int i_det = 0; i_det < N_DET; i_det++) {
    int det_id = list_det_id[i_det];
    auto vec = UtilSQHit::FindTriggerHitsFast(evt, hit_vec, det_id);
    for (auto it = vec->begin(); it != vec->end(); it++) {
      if ((*it)->get_level() != m_lvl) continue;
      int    ele  = (*it)->get_element_id();
      double time = (*it)->get_tdc_time  ();
      h1_ele     [i_det]->Fill(ele );
      h1_time    [i_det]->Fill(time);
      h2_time_ele[i_det]->Fill(ele, time);
      if ((*it)->is_in_time()) {
        h1_ele_in [i_det]->Fill(ele );
        h1_time_in[i_det]->Fill(time);
      }
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::FindAllMonHist()
{
  h1_cnt = FindMonHist("h1_cnt");
  if (! h1_cnt) return 1;

  ostringstream oss;
  for (int i_det = 0; i_det < N_DET; i_det++) {
    oss.str("");
    oss << "h1_ele_" << i_det;
    h1_ele[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_ele[i_det]) return 1;
    oss.str("");
    oss << "h1_ele_in_" << i_det;
    h1_ele_in[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_ele_in[i_det]) return 1;
    oss.str("");
    oss << "h1_time_" << i_det;
    h1_time[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_time[i_det]) return 1;
    oss.str("");
    oss << "h1_time_in_" << i_det;
    h1_time_in[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_time_in[i_det]) return 1;
    oss.str("");
    oss << "h2_time_ele_" << i_det;
    h2_time_ele[i_det] = (TH2*)FindMonHist(oss.str().c_str());
    if (! h2_time_ele[i_det]) return 1;
  }

  for (int i = 0; i < 8; i++) {
    oss.str("");
    oss << "RF_proj_" << i;
    RF_proj[i] = (TH1*)FindMonHist(oss.str().c_str());
    if (! RF_proj[i]) return 1;
  }

  return 0;
}

int OnlMonV1495::DrawMonitor()
{
  //Determine maximum value from projection histos to determine RF rising edges
  TLine* proj_line[8];
  for (int i = 0; i < 8; i++) {
    int binmax = RF_proj[i]->GetMaximumBin();
    double x = RF_proj[i]->GetXaxis()->GetBinCenter(binmax);
    //Lines to show RF buckets on 2d histo
    if (is_H1 == 1) proj_line[i] = new TLine(0.5, x, 23.5, x);
    else            proj_line[i] = new TLine(0.5, x, 16.5, x);
  }

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(2, N_DET);
  for (int i_det = 0; i_det < N_DET; i_det++) {
    TVirtualPad* pad01 = pad0->cd(2*i_det+1);
    pad01->SetGrid();
    h1_ele[i_det]->SetLineColor(kBlack);
    h1_ele[i_det]->Draw();
    h1_ele_in[i_det]->SetLineColor(kBlue);
    h1_ele_in[i_det]->SetFillColor(kBlue-7);
    h1_ele_in[i_det]->Draw("same");

    TVirtualPad* pad02 = pad0->cd(2*i_det+2);
    pad02->SetGrid();
    h2_time_ele[i_det]->Draw("colz");
    ostringstream oss;
    oss << "pr_" << h2_time_ele[i_det]->GetName();
    TProfile* pr = h2_time_ele[i_det]->ProfileX(oss.str().c_str());
    pr->SetLineColor(kBlack);
    pr->Draw("E1same");
    for (int i = 0; i < 8; i++) {
      proj_line[i]->SetLineStyle(2);
      proj_line[i]->SetLineWidth(3);
      proj_line[i]->SetLineColor(kRed);
      proj_line[i]->Draw();
    }
  }
  //can0->SetStatus(OnlMonCanvas::OK);
  //int n_evt_all = h1_cnt->GetBinContent(1);
  int n_evt_ana = h1_cnt->GetBinContent(2);
  can0->AddMessage(TString::Format("N of analyzed events = %d.", n_evt_ana));

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1, N_DET);
  for (int i_det = 0; i_det < N_DET; i_det++) {
    TVirtualPad* pad11 = pad1->cd(i_det+1);
    pad11->SetGrid();
    UtilHist::AutoSetRange(h1_time[i_det]);
    h1_time[i_det]->SetLineColor(kBlack);
    h1_time[i_det]->Draw();
    h1_time_in[i_det]->SetLineColor(kBlue);
    h1_time_in[i_det]->SetFillColor(kBlue-7);
    h1_time_in[i_det]->Draw("same");
  }
  //can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}

void OnlMonV1495::SetDet(const char* det0, const char* det1)
{
  list_det_name[0] = det0;
  list_det_name[1] = det1;
  GeomSvc* geom = GeomSvc::instance();
  for (int ii = 0; ii < N_DET; ii++) {
    list_det_id[ii] = geom->getDetectorID(list_det_name[ii]);
  }
}

/// OnlMonTrigV1495.C
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unordered_set>
#include <TSystem.h>
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
#include "OnlMonTrigV1495.h"
using namespace std;

OnlMonTrigV1495::OnlMonTrigV1495()
{
  NumCanvases(4);
  Name("OnlMonTrigV1495" ); 
  Title("V1495 Trigger Analysis" );
}

int OnlMonTrigV1495::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::InitRunOnlMon(PHCompositeNode* topNode)
{
  SetDet();

  if (roadset.PosTop()->GetNumRoads() == 0) {
    SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
    if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
    int LBtop = sq_run->get_v1495_id(2);
    int LBbot = sq_run->get_v1495_id(3);
    int ret = roadset.LoadConfig(LBtop, LBbot);
    if (ret != 0) {
      cout << "!!WARNING!!  OnlMonTrigEP::InitRunOnlMon():  roadset.LoadConfig returned " << ret << ".\n";
    }
  }

  GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  int num_tot_ele = 0;
  //Loop through hodoscopes 
  for (int i_det = 0; i_det < N_DET; i_det++) {
    string name = list_det_name[i_det];
    int  det_id = list_det_id  [i_det];
    int n_ele  = geom->getPlaneNElements(det_id);
    num_tot_ele += n_ele;
    if (det_id <= 0 || n_ele <= 0) {
      cout << "OnlMonTrigV1495::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  unsigned int n_pos_top = roadset.PosTop()->GetNumRoads();
  unsigned int n_pos_bot = roadset.PosBot()->GetNumRoads();
  unsigned int n_neg_top = roadset.NegTop()->GetNumRoads();
  unsigned int n_neg_bot = roadset.NegBot()->GetNumRoads();
  h1_rs_cnt[0] = new TH1D("h1_rs_cnt_pos_top", "", n_pos_top, -0.5, n_pos_top-0.5);
  h1_rs_cnt[1] = new TH1D("h1_rs_cnt_pos_bot", "", n_pos_bot, -0.5, n_pos_bot-0.5);
  h1_rs_cnt[2] = new TH1D("h1_rs_cnt_neg_top", "", n_neg_top, -0.5, n_neg_top-0.5);
  h1_rs_cnt[3] = new TH1D("h1_rs_cnt_neg_bot", "", n_neg_bot, -0.5, n_neg_bot-0.5);
  oss.str("");
  oss << "Positive Top #minus RS " << roadset.RoadsetID() << ", FW " << hex << roadset.LBTop() << dec << ";Road Index (N=" << n_pos_top << ");Counts";
  h1_rs_cnt[0]->SetTitle(oss.str().c_str());
  oss.str("");
  oss << "Positive Bottom #minus RS " << roadset.RoadsetID() << ", FW " << hex << roadset.LBBot() << dec << ";Road Index (N=" << n_pos_bot << ");Counts";
  h1_rs_cnt[1]->SetTitle(oss.str().c_str());
  oss.str("");
  oss << "Negative Top #minus RS " << roadset.RoadsetID() << ", FW " << hex << roadset.LBTop() << dec << ";Road Index (N=" << n_neg_top << ");Counts";
  h1_rs_cnt[2]->SetTitle(oss.str().c_str());
  oss.str("");
  oss << "Negative Bottom #minus RS " << roadset.RoadsetID() << ", FW " << hex << roadset.LBBot() << dec << ";Road Index (N=" << n_neg_bot << ");Counts";
  h1_rs_cnt[3]->SetTitle(oss.str().c_str());
  for (int i = 0; i < 4; i++) RegisterHist(h1_rs_cnt[i]);

  h1_cnt = new TH1D("h1_cnt", "", 20, 0.5, 20.5);
  RegisterHist(h1_cnt); 

  h1_cnt->SetBinContent(1, roadset.RoadsetID());
  h1_cnt->SetBinContent(2, roadset.LBTop());
  h1_cnt->SetBinContent(3, roadset.LBBot());
  h1_cnt->SetBinContent(4, roadset.PosTop()->GetNumRoads());
  h1_cnt->SetBinContent(5, roadset.PosBot()->GetNumRoads());
  h1_cnt->SetBinContent(6, roadset.NegTop()->GetNumRoads());
  h1_cnt->SetBinContent(7, roadset.NegBot()->GetNumRoads());

  const double DT = 20/9.0; // 4/9 ns per single count of Taiwan TDC
  const int NT    = 400;
  const double T0 = 200.5*DT;
  const double T1 = 600.5*DT; 

  oss.str("");
  oss << "h2_trig_time_" << 1;
  h2_trig_time = new TH2D(oss.str().c_str(), "",NT, T0, T1,10, 0.5, 10.5);
  oss.str("");
  oss << "Trigger Timing After Inh" << ";Trigger;tdcTime;Hit count";
  h2_trig_time->SetTitle(oss.str().c_str());

  h2_trig_time->GetYaxis()->SetBinLabel( 1, "FPGA1");
  h2_trig_time->GetYaxis()->SetBinLabel( 2, "FPGA2");
  h2_trig_time->GetYaxis()->SetBinLabel( 3, "FPGA3");
  h2_trig_time->GetYaxis()->SetBinLabel( 4, "FPGA4");
  h2_trig_time->GetYaxis()->SetBinLabel( 5, "FPGA5");
  h2_trig_time->GetYaxis()->SetBinLabel( 6, "NIM1");
  h2_trig_time->GetYaxis()->SetBinLabel( 7, "NIM2");
  h2_trig_time->GetYaxis()->SetBinLabel( 8, "NIM3");
  h2_trig_time->GetYaxis()->SetBinLabel( 9, "NIM4");
  h2_trig_time->GetYaxis()->SetBinLabel(10, "NIM5");

  const double DT2 = 1.0; // 1 ns per single count of v1495 TDC
  const int NT2 = 300;
  const double T02 = 300.5 * DT2;
  const double T12 = 900.5;

  oss.str("");
  oss << "h2_RF_" << 1;
  h2_RF = new TH2D(oss.str().c_str(), "",NT2, T02, T12,  9, 0.5, 9.5);
  oss.str("");
  oss << "RF TDC" << ";tdcTime;RF Board;Hit count";
  h2_RF->SetTitle(oss.str().c_str());

  oss.str("");
  oss << "h2_fpga_nim_time_af_" << 1;
  h2_fpga_nim_time_af = new TH2D(oss.str().c_str(), "", 100, 1000.5, 1100.5, 100, 1000.5, 1100.5);
  oss.str("");
  oss << "FPGA 1 & NIM 4 After Inh Timing" << ";NIM tdcTime;FPGA tdcTime;Hit count";
  h2_fpga_nim_time_af->SetTitle(oss.str().c_str());

  oss.str("");
  oss << "h1_trig_diff_TS_" << 0;
  h1_trig_diff_TS = new TH1D(oss.str().c_str(), "", NT+1, -0.5, 30.5);
  oss.str("");
  oss << "FPGA1 NIM4 Timing Difference TS constrained" << ";TDC time diff;Hit count";
  h1_trig_diff_TS->SetTitle(oss.str().c_str());

  RegisterHist(h1_trig_diff_TS);
  RegisterHist(h2_trig_time);
  RegisterHist(h2_fpga_nim_time_af);   
  RegisterHist(h2_RF); 
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 
  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1 
  int is_FPGA1 = (evt->get_trigger(SQEvent::MATRIX1)) ? 1 : 0; 
  
//RF *************************************************************************************** 
  auto vec1 = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, "RF");
  int count = 0;
  for(auto it = vec1->begin(); it != vec1->end(); it++){
    double tdc_time = (*it)->get_tdc_time();
    int element = (*it)->get_element_id();       
    //Determining RF buckets for road set timing constraints 
    if(is_FPGA1){
      h2_RF->Fill(tdc_time,element);
      if      (count ==  3) RF_edge_low[TOP   ] = tdc_time;
      else if (count ==  4) RF_edge_up [TOP   ] = tdc_time;
      else if (count == 11) RF_edge_low[BOTTOM] = tdc_time;
      else if (count == 12) RF_edge_up [BOTTOM] = tdc_time;
    }
    count ++;
  }

//TW TDC ************************************************************************************
  auto vec_FPGA_af = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhMatrix");
  for (auto it = vec_FPGA_af->begin(); it != vec_FPGA_af->end(); it++) {
    h2_trig_time->Fill((*it)->get_tdc_time(),(*it)->get_element_id());
  }

  auto vec_NIM_af = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhNIM");
  for (auto it = vec_NIM_af->begin(); it != vec_NIM_af->end(); it++) {
    h2_trig_time->Fill((*it)->get_tdc_time(),(*it)->get_element_id()+5); // element +5 so nim index start at 6 in histo
  }

  if(evt->get_trigger(SQEvent::MATRIX1) && evt->get_trigger(SQEvent::NIM4)){

    FPGA_NIM_Time(vec_FPGA_af, vec_NIM_af, 4, 1,h2_fpga_nim_time_af,h1_trig_diff_TS);

  }else{

  }

//ROAD ID Logic  *************************************************************************** 
  if(is_FPGA1){
    vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
    vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
    vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
    vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);
    
    vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
    vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
    vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
    vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);
    
    CountFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.PosTop(), h1_rs_cnt[0]);
    CountFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.PosBot(), h1_rs_cnt[1]);
    CountFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.NegTop(), h1_rs_cnt[2]);
    CountFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.NegBot(), h1_rs_cnt[3]);
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigV1495::FindAllMonHist()
{

 // cout << "FIND ALL MON HIST PART" << endl;
  ostringstream oss; 

  oss.str("");
  oss << "h1_trig_diff_TS_" << 0;
  h1_trig_diff_TS = FindMonHist(oss.str().c_str());
  if (! h1_trig_diff_TS) return 1; 

  h1_rs_cnt[0] = FindMonHist("h1_rs_cnt_pos_top");
  h1_rs_cnt[1] = FindMonHist("h1_rs_cnt_pos_bot");
  h1_rs_cnt[2] = FindMonHist("h1_rs_cnt_neg_top");
  h1_rs_cnt[3] = FindMonHist("h1_rs_cnt_neg_bot");
  if (! h1_rs_cnt[0] || ! h1_rs_cnt[1] || ! h1_rs_cnt[1] || ! h1_rs_cnt[3]) return 1;

  h1_cnt = FindMonHist("h1_cnt");
  if (! h1_cnt) return 1; 

  oss.str("");
  oss << "h2_RF_" << 1;
  h2_RF = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_RF) return 1; 
 
  oss.str("");
  oss << "h2_trig_time_" << 1;
  h2_trig_time = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_trig_time) return 1;

  oss.str("");
  oss << "h2_fpga_nim_time_af_" << 1;
  h2_fpga_nim_time_af = (TH2*)FindMonHist(oss.str().c_str());
  if (! h2_fpga_nim_time_af) return 1; 


  return 0;
}

int OnlMonTrigV1495::DrawMonitor()
{
  //DRAWING HISTOGRAMS ON .PNG FILES ******************************************
  UtilHist::AutoSetRangeX(h2_trig_time);
  UtilHist::AutoSetRangeX(h2_fpga_nim_time_af); 
  UtilHist::AutoSetRangeY(h2_fpga_nim_time_af); 
  UtilHist::AutoSetRangeX(h2_RF);

  //int roadset_id = (int)h1_cnt->GetBinContent(1);
  //int LBTop      = (int)h1_cnt->GetBinContent(2);
  //int LBBot      = (int)h1_cnt->GetBinContent(3);
  //int n_pos_top  = (int)h1_cnt->GetBinContent(4);
  //int n_pos_bot  = (int)h1_cnt->GetBinContent(5);
  //int n_neg_top  = (int)h1_cnt->GetBinContent(6);
  //int n_neg_bot  = (int)h1_cnt->GetBinContent(7);

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1,2);
  TVirtualPad* pad00 = pad0->cd(1);
  pad00->SetGrid();
  DrawTH2WithPeakPos(h2_trig_time);
  /*ostringstream oss;
  oss << "pr_" << h2_trig_time->GetName();
  TProfile* pr = h2_trig_time->ProfileX(oss.str().c_str());
  pr->SetLineColor(kBlack);
  pr->Draw("E1same");
*/

  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h2_RF->Draw("colz");  

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1,2);
  TVirtualPad* pad10 = pad1->cd(1);
  pad10->SetGrid();
  h1_trig_diff_TS->Draw();

  TVirtualPad* pad11 = pad1->cd(2);
  pad11->SetGrid();
  h2_fpga_nim_time_af->Draw("colz");
  ostringstream oss11;
  oss11 << "pr_" << h2_fpga_nim_time_af->GetName();
  TProfile* pr11 = h2_fpga_nim_time_af->ProfileX(oss11.str().c_str());
  pr11->SetLineColor(kBlack);
  pr11->Draw("E1same");

  OnlMonCanvas* can2 = GetCanvas(2);
  TPad* pad2 = can2->GetMainPad();
  pad2->Divide(1,2);
  TVirtualPad* pad20 = pad2->cd(1);
  pad20->SetGrid();
  h1_rs_cnt[0]->SetLineColor(kBlue);
  h1_rs_cnt[0]->Draw();

  //can2->AddMessage(TString::Format("Roadset %d, LBTop 0x%x, LBBot 0x%x", roadset_id, LBTop, LBBot).Data());
  //can2->AddMessage(TString::Format("N_{PosTop} %d, N_{PosBot} %d, N_{NegTop} %d, N_{NegBot} %d", n_pos_top, n_pos_bot, n_neg_top, n_neg_bot).Data());
  
  TVirtualPad* pad21 = pad2->cd(2);
  pad21->SetGrid();
  h1_rs_cnt[1]->SetLineColor(kBlue);
  h1_rs_cnt[1]->Draw();

  OnlMonCanvas* can3 = GetCanvas(3);
  TPad* pad3 = can3->GetMainPad();
  pad3->Divide(1,2);
  TVirtualPad* pad30 = pad3->cd(1);
  pad30->SetGrid();
  h1_rs_cnt[2]->SetLineColor(kBlue);
  h1_rs_cnt[2]->Draw();

  TVirtualPad* pad31 = pad3->cd(2);
  pad31->SetGrid();
  h1_rs_cnt[3]->SetLineColor(kBlue);
  h1_rs_cnt[3]->Draw();

  return 0;
}

void OnlMonTrigV1495::SetDet()
{
  list_det_name[0] = "H1T";
  list_det_name[1] = "H1B";
  list_det_name[2] = "H2T";
  list_det_name[3] = "H2B";
  list_det_name[4] = "H3T";
  list_det_name[5] = "H3B";
  list_det_name[6] = "H4T";
  list_det_name[7] = "H4B";
   
  GeomSvc* geom = GeomSvc::instance();
  for (int ii = 0; ii < N_DET; ii++) {
    list_det_id[ii] = geom->getDetectorID(list_det_name[ii]);
  }
}

void OnlMonTrigV1495::CountFiredRoads(const int top0bot1, vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X, TriggerRoads* roads, TH1* h1_rs_cnt)
{
  unordered_set<int> set_ele1;
  unordered_set<int> set_ele2;
  unordered_set<int> set_ele3;
  unordered_set<int> set_ele4;
  for (auto it = H1X->begin(); it != H1X->end(); it++) {
    int    ele  = (*it)->get_element_id();
    //double time = (*it)->get_tdc_time();
    //if (RF_edge_low[top0bot1] < time && time < RF_edge_up[top0bot1]) set_ele1.insert(ele);
    if ((*it)->is_in_time()) set_ele1.insert(ele);
  }
  for (auto it = H2X->begin(); it != H2X->end(); it++) {
    int    ele  = (*it)->get_element_id();
    //double time = (*it)->get_tdc_time();
    //if (RF_edge_low[top0bot1] < time && time < RF_edge_up[top0bot1]) set_ele2.insert(ele);
    if ((*it)->is_in_time()) set_ele2.insert(ele);
  }
  for (auto it = H3X->begin(); it != H3X->end(); it++) {
    int    ele  = (*it)->get_element_id();
    //double time = (*it)->get_tdc_time();
    //if (RF_edge_low[top0bot1] < time && time < RF_edge_up[top0bot1]) set_ele3.insert(ele);
    if ((*it)->is_in_time()) set_ele3.insert(ele);
  }
  for (auto it = H4X->begin(); it != H4X->end(); it++) {
    int    ele  = (*it)->get_element_id();
    //double time = (*it)->get_tdc_time();
    //if (RF_edge_low[top0bot1] < time && time < RF_edge_up[top0bot1]) set_ele4.insert(ele);
    if ((*it)->is_in_time()) set_ele4.insert(ele);
  }

  for (unsigned int ir = 0 ; ir < roads->GetNumRoads(); ir++) {
    TriggerRoad1* road = roads->GetRoad(ir);
    if (set_ele1.find(road->H1X) != set_ele1.end() &&
        set_ele2.find(road->H2X) != set_ele2.end() &&
        set_ele3.find(road->H3X) != set_ele3.end() &&
        set_ele4.find(road->H4X) != set_ele4.end()   ) h1_rs_cnt->Fill(ir);
  }
}

void OnlMonTrigV1495::FPGA_NIM_Time(vector<SQHit*>* FPGA,vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* h2, TH1* h1){
  //Fill 2D histo with TDC timing for NIM & FPGA trigger
  //Fill 1D histo with TDC timing difference for NIM & FPGA trigger
  for(auto it0 = FPGA->begin(); it0 != FPGA->end(); it0++){ //FPGA
      double ele_FPGA = (*it0)->get_element_id();
      double time_FPGA = (*it0)->get_tdc_time();
      for(auto it1 = NIM->begin(); it1 != NIM->end(); it1++){//NIM
        double ele_NIM = (*it1)->get_element_id();
        double time_NIM = (*it1)->get_tdc_time();
        if(ele_FPGA == FPGA_trig_num && ele_NIM == NIM_trig_num){
          if(h2 != NULL){
            h2->Fill(time_NIM,time_FPGA);
          }
          double time_diff = Abs(time_FPGA,time_NIM);
          h1->Fill(time_diff);
        }
      }
  }

}

void OnlMonTrigV1495::DrawTH2WithPeakPos(TH2* h2, const double cont_min)
{
  h2->Draw("colz");
  int ny = h2->GetNbinsY();
  for (int iy = 1; iy <= ny; iy++) {
    TH1* h1 = h2->ProjectionX("h1_draw_th2", iy, iy);
    ostringstream oss;
    if (h1->GetMaximum() >= cont_min) {
      oss << "Peak @ " << h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
    } else {
      oss << "No sizable peak";
    }
    TText* text = new TText();
    text->SetNDC(true);
    text->SetTextAlign(22);
    text->DrawText(0.3, 0.1+(iy-0.5)*0.8/ny, oss.str().c_str());
    // The y-position above assumes that the top & bottom margins are 0.1 each.
  }
}

void OnlMonTrigV1495:: debug_print(int debug_lvl){
  //debug function
  if(debug_lvl == 0){
    cout << endl;
    cout << "New Event" << endl;
    cout << "H1T: ";
    for (auto it = vecH1T->begin(); it != vecH1T->end(); it++) {
        double ele1 = (*it)->get_element_id();
        cout  << ele1 << ", ";
    }
    cout << endl;

    cout << "H2T: ";
    for (auto it = vecH2T->begin(); it != vecH2T->end(); it++) {
        double ele2 = (*it)->get_element_id();
        cout  << ele2 << ", ";
    }
    cout << endl;

    cout << "H3T: ";
    for (auto it = vecH3T->begin(); it != vecH3T->end(); it++) {
        double ele3 = (*it)->get_element_id();
        cout  << ele3 << ", ";
    }
    cout << endl;

    cout << "H4T: ";
    for (auto it = vecH4T->begin(); it != vecH4T->end(); it++) {
        double ele4 = (*it)->get_element_id();
        cout  << ele4 << ", ";
    }
    cout << endl;
    cout << endl;

    cout << "H1B: ";
    for (auto it = vecH1B->begin(); it != vecH1B->end(); it++) {
        double ele1 = (*it)->get_element_id();
        cout  << ele1 << ", ";
    }
    cout << endl;

    cout << "H2B: ";
    for (auto it = vecH2B->begin(); it != vecH2B->end(); it++) {
        double ele2 = (*it)->get_element_id();
        cout  << ele2 << ", ";
    }
    cout << endl;

    cout << "H3B: ";
    for (auto it = vecH3B->begin(); it != vecH3B->end(); it++) {
        double ele3 = (*it)->get_element_id();
        cout  << ele3 << ", ";
    }
    cout << endl;

    cout << "H4B: ";
    for (auto it = vecH4B->begin(); it != vecH4B->end(); it++) {
        double ele4 = (*it)->get_element_id();
        cout  << ele4 << ", ";
    }
    cout << endl;
  }
}

double OnlMonTrigV1495:: Abs(double var0, double var1){
  //get absolute value of difference between var0 and var1
  if(var0 > var1){ 
    return (var0 - var1);
  }else{
    return (var1 - var0);
  }
}

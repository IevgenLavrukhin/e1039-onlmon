/// OnlMonTrigRoad.C
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
#include "OnlMonTrigRoad.h"
using namespace std;

OnlMonTrigRoad::OnlMonTrigRoad()
{
  NumCanvases(2);
  Name("OnlMonTrigRoad"); 
  Title("Trigger Road");
}

int OnlMonTrigRoad::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRoad::InitRunOnlMon(PHCompositeNode* topNode)
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

  //GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  
  //int num_tot_ele = 0;
  //Loop through hodoscopes 
  //for (int i_det = 0; i_det < N_DET; i_det++) {
  //  string name = list_det_name[i_det];
  //  int  det_id = list_det_id  [i_det];
  //  int n_ele  = geom->getPlaneNElements(det_id);
  //  num_tot_ele += n_ele;
  //  if (det_id <= 0 || n_ele <= 0) {
  //    cout << "OnlMonTrigRoad::InitRunOnlMon():  Invalid det_id or n_ele: " 
  //         << det_id << " " << n_ele << " at name = " << name << "." << endl;
  //    return Fun4AllReturnCodes::ABORTEVENT;
  //  }
  //}

  h1_cnt = new TH1D("h1_cnt", "", 20, 0.5, 20.5);
  RegisterHist(h1_cnt); 

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

  h1_cnt->SetBinContent(1, roadset.RoadsetID());
  h1_cnt->SetBinContent(2, roadset.LBTop());
  h1_cnt->SetBinContent(3, roadset.LBBot());
  h1_cnt->SetBinContent(4, roadset.PosTop()->GetNumRoads());
  h1_cnt->SetBinContent(5, roadset.PosBot()->GetNumRoads());
  h1_cnt->SetBinContent(6, roadset.NegTop()->GetNumRoads());
  h1_cnt->SetBinContent(7, roadset.NegBot()->GetNumRoads());
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRoad::ProcessEventOnlMon(PHCompositeNode* topNode)
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
    //int element = (*it)->get_element_id();       
    //Determining RF buckets for road set timing constraints 
    if(is_FPGA1){
      //h2_RF->Fill(tdc_time,element);
      if      (count ==  3) RF_edge_low[TOP   ] = tdc_time;
      else if (count ==  4) RF_edge_up [TOP   ] = tdc_time;
      else if (count == 11) RF_edge_low[BOTTOM] = tdc_time;
      else if (count == 12) RF_edge_up [BOTTOM] = tdc_time;
    }
    count ++;
  }

//ROAD ID Logic  *************************************************************************** 
  if(is_FPGA1){
    auto vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
    auto vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
    auto vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
    auto vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);

    auto vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
    auto vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
    auto vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
    auto vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);
    
    CountFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.PosTop(), h1_rs_cnt[0]);
    CountFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.PosBot(), h1_rs_cnt[1]);
    CountFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.NegTop(), h1_rs_cnt[2]);
    CountFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.NegBot(), h1_rs_cnt[3]);
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRoad::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigRoad::FindAllMonHist()
{
  h1_cnt = FindMonHist("h1_cnt");
  if (! h1_cnt) return 1; 

  h1_rs_cnt[0] = FindMonHist("h1_rs_cnt_pos_top");
  h1_rs_cnt[1] = FindMonHist("h1_rs_cnt_pos_bot");
  h1_rs_cnt[2] = FindMonHist("h1_rs_cnt_neg_top");
  h1_rs_cnt[3] = FindMonHist("h1_rs_cnt_neg_bot");
  if (! h1_rs_cnt[0] || ! h1_rs_cnt[1] || ! h1_rs_cnt[1] || ! h1_rs_cnt[3]) return 1;

  return 0;
}

int OnlMonTrigRoad::DrawMonitor()
{
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
  h1_rs_cnt[0]->SetLineColor(kBlue);
  h1_rs_cnt[0]->Draw();

  //can0->AddMessage(TString::Format("Roadset %d, LBTop 0x%x, LBBot 0x%x", roadset_id, LBTop, LBBot).Data());
  //can0->AddMessage(TString::Format("N_{PosTop} %d, N_{PosBot} %d, N_{NegTop} %d, N_{NegBot} %d", n_pos_top, n_pos_bot, n_neg_top, n_neg_bot).Data());
  
  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h1_rs_cnt[1]->SetLineColor(kBlue);
  h1_rs_cnt[1]->Draw();


  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1,2);
  TVirtualPad* pad10 = pad1->cd(1);
  pad10->SetGrid();
  h1_rs_cnt[2]->SetLineColor(kBlue);
  h1_rs_cnt[2]->Draw();

  TVirtualPad* pad11 = pad1->cd(2);
  pad11->SetGrid();
  h1_rs_cnt[3]->SetLineColor(kBlue);
  h1_rs_cnt[3]->Draw();

  return 0;
}

void OnlMonTrigRoad::SetDet()
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

void OnlMonTrigRoad::CountFiredRoads(const int top0bot1, vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X, TriggerRoads* roads, TH1* h1_rs_cnt)
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

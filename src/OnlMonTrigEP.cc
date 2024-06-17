/// OnlMonTrigEP.C
#include <sstream>
#include <iomanip>
#include <cstring>
#include <unordered_set>
#include <TSystem.h>
#include <TH1D.h>
#include <TH2D.h>
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
#include "OnlMonTrigEP.h"
using namespace std;

OnlMonTrigEP::OnlMonTrigEP()
{
  NumCanvases(1);
  Name("OnlMonTrigEP" ); 
  Title("FPGA1 Purity/Efficiency" );
}

int OnlMonTrigEP::InitOnlMon(PHCompositeNode* topNode)
{
	event_counter = 0;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::InitRunOnlMon(PHCompositeNode* topNode)
{
  SetDet();

  if (roadset.PosTop()->GetNumRoads() == 0) {
    SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
    if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
    int LBtop = sq_run->get_v1495_id(2);
    int LBbot = sq_run->get_v1495_id(3);
    int ret = roadset.LoadConfig(LBtop, LBbot);
    if (Verbosity() >= 0) {
      cout << "OnlMonTrigEP: LoadConfig(" << LBtop << ", " << LBbot << ")\n"
           << roadset << endl;
    }
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
      cout << "OnlMonTrigEP::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

  }

  h1_cnt = new TH1D("h1_cnt", "", 100, 0.5, 100.5);
  RegisterHist(h1_cnt);

  oss.str("");
  oss << "h1_purity_" << 0;
  h1_purity = new TH1D(oss.str().c_str(), "", 2, -0.5, 1.5);
  oss.str("");
  oss << "FGPA1 Purity" << ";;Hit count";
  h1_purity->SetTitle(oss.str().c_str());
  h1_purity->GetXaxis()->SetBinLabel( 2, "FPGA1 && Emu"   );//"FPGA1 && rd hit" );
  h1_purity->GetXaxis()->SetBinLabel( 1, "FPGA1 && ! Emu" );//"FPGA1 && no rd hit");

  RegisterHist(h1_purity);

  oss.str("");
  oss << "h1_eff_NIM3_" << 0;
  h1_eff_NIM3 = new TH1D(oss.str().c_str(), "", 2, -0.5, 1.5);
  oss.str("");
  //oss << "FPGA1 Efficiency (NIM3 + Event type)" << ";;Hit count";
  oss << "FPGA1 Efficiency" << ";;Hit count";
  h1_eff_NIM3->SetTitle(oss.str().c_str());
  h1_eff_NIM3->GetXaxis()->SetBinLabel( 2, "Non-FPGA1 && Emu && FGPA1"   );//"NIM3 && rd hit && FPGA1");
  h1_eff_NIM3->GetXaxis()->SetBinLabel( 1, "Non-FPGA1 && Emu && ! FPGA1" );//"NIM3 && rd hit && NO FPGA1");
  
  RegisterHist(h1_eff_NIM3);
 

	//Ievgen
	event_counter = 0;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::ProcessEventOnlMon(PHCompositeNode* topNode)
{ 
  SQEvent*      evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*  hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector*  trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");

  if (!evt || !hit_vec  || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //Determine whether event is FPGA1 
  int is_FPGA1 = (evt->get_trigger(SQEvent::MATRIX1)) ? 1 : 0;
	int is_NIM3 = (evt->get_trigger(SQEvent::NIM3)) ? 1 : 0;
 // bool is_not_FPGA1 = evt->get_trigger() & 991; // 0b1111011111 = 991 = non-FPGA1 triggers

	
	auto vec_FPGA = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhMatrix");
	auto vec_NIM = UtilSQHit::FindHitsFast(evt, hit_vec, "AfterInhNIM");

	
	int is_FPGA1hit =0;
	int is_NIM3hit = 0;
	
	//check if in-time hit in TW TDC for NIM3
	for(auto it = vec_NIM->begin(); it != vec_NIM->end(); it++){
			int element = (*it)->get_element_id();
			double tdc_time = (*it)->get_tdc_time();
			if(element ==3 && tdc_time>1035 && tdc_time<1045){
				is_NIM3hit = 1;			
			}

	}
	
	//check if in-time hit in TW TDC for FPGA1
	for(auto it = vec_FPGA->begin(); it != vec_FPGA->end(); it++){
      int element = (*it)->get_element_id();
      double tdc_time = (*it)->get_tdc_time();
      if(element ==1 && tdc_time>1035 && tdc_time<1045){
        is_FPGA1hit = 1;
      }
  }

  //getting hodoscome hits hits from TW TDC;
	std::vector<SQHit*>* vecH[8]; //mapping 1T, 1B, 2T, 2B, 3T, 3B, 4T, 4B;
  for(int i=0; i<8; i++){
     vecH[i] = UtilSQHit::FindHitsFast(evt, hit_vec, list_det_id[i]);
  }


  vector<TriggerRoad1*> roads_pos_top_f;
  vector<TriggerRoad1*> roads_pos_bot_f;
  vector<TriggerRoad1*> roads_neg_top_f;
  vector<TriggerRoad1*> roads_neg_bot_f;
  FindFiredRoads(TOP   , vecH[0], vecH[2], vecH[4], vecH[6], roadset.PosTop(), roads_pos_top_f);
  FindFiredRoads(BOTTOM, vecH[1], vecH[3], vecH[5], vecH[7], roadset.PosBot(), roads_pos_bot_f);
  FindFiredRoads(TOP   , vecH[0], vecH[2], vecH[4], vecH[6], roadset.NegTop(), roads_neg_top_f);
  FindFiredRoads(BOTTOM, vecH[1], vecH[3], vecH[5], vecH[7], roadset.NegBot(), roads_neg_bot_f);


  int is_FPGA1recon = (roads_pos_top_f.size() > 0 && roads_neg_bot_f.size() > 0) ||
                      (roads_pos_bot_f.size() > 0 && roads_neg_top_f.size() > 0);


	//Getting FPGA1 Efficiency:
	//if there is in-time TDC hit in NIM3 trigger, check if there is a RS confirmed.
	if(is_NIM3hit){

		if(is_FPGA1recon){
			printf("-------- Event = %i  => NIM3 Trigger: found in TDC =%i, found in TS = %i\n", event_counter, is_NIM3hit, is_NIM3);
   		printf("\t\t => FPGA1 trigger Reconstructed:  found in TDC = %i, found in TS =%i \n",  is_FPGA1hit, is_FPGA1);			

     	if (is_FPGA1hit || is_FPGA1){
				 h1_eff_NIM3->Fill(1);
   	  }else{
         h1_eff_NIM3->Fill(0);
			}
    }
	}			
	
	//Getting FPGA1 Purity: 
	//If there is FPGA1 trigger hit check if we can reconstruct it:
	if(is_FPGA1hit || is_FPGA1){
			if(is_FPGA1recon){
				h1_purity->Fill(1);				
			}else{
				h1_purity->Fill(0);
			}

	}


/*
//RF *************************************************************************************** 
  if(is_FPGA1){
    auto vec1 = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, "RF");
    int count = 0;
    for(auto it = vec1->begin(); it != vec1->end(); it++){
      double tdc_time = (*it)->get_tdc_time();
      //int element = (*it)->get_element_id();
      //Determining RF buckets for road set timing constraints 
      if      (count ==  3) RF_edge_low[TOP   ] = tdc_time;
      else if (count ==  4) RF_edge_up [TOP   ] = tdc_time;
      else if (count == 11) RF_edge_low[BOTTOM] = tdc_time;
      else if (count == 12) RF_edge_up [BOTTOM] = tdc_time;
      count ++;
    }
    //cout << "RF_edge: " << RF_edge_low[0] << " " << RF_edge_up[0] << "  " << RF_edge_low[1] << " " << RF_edge_up[1] << endl;
  }
 
//ROAD SET Logic  *************************************************************************** 
  std::vector<SQHit*>* vecH1T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[0]);
  std::vector<SQHit*>* vecH2T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[2]);
  std::vector<SQHit*>* vecH3T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[4]);
  std::vector<SQHit*>* vecH4T = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[6]);

  std::vector<SQHit*>* vecH1B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[1]);
  std::vector<SQHit*>* vecH2B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[3]);
  std::vector<SQHit*>* vecH3B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[5]);
  std::vector<SQHit*>* vecH4B = UtilSQHit::FindTriggerHitsFast(evt, trig_hit_vec, list_det_id[7]);

  vector<TriggerRoad1*> roads_pos_top_f;
  vector<TriggerRoad1*> roads_pos_bot_f;
  vector<TriggerRoad1*> roads_neg_top_f;
  vector<TriggerRoad1*> roads_neg_bot_f;
  FindFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.PosTop(), roads_pos_top_f);
  FindFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.PosBot(), roads_pos_bot_f);
  FindFiredRoads(TOP   , vecH1T, vecH2T, vecH3T, vecH4T, roadset.NegTop(), roads_neg_top_f);
  FindFiredRoads(BOTTOM, vecH1B, vecH2B, vecH3B, vecH4B, roadset.NegBot(), roads_neg_bot_f);
  bool emu1 = (roads_pos_top_f.size() > 0 && roads_neg_bot_f.size() > 0) ||
              (roads_pos_bot_f.size() > 0 && roads_neg_top_f.size() > 0);
  bool emu2 = (roads_pos_top_f.size() > 0 && roads_neg_top_f.size() > 0) ||
              (roads_pos_bot_f.size() > 0 && roads_neg_bot_f.size() > 0);
  bool emu3 = (roads_pos_top_f.size() > 0 && roads_pos_bot_f.size() > 0) ||
              (roads_neg_top_f.size() > 0 && roads_neg_bot_f.size() > 0);
  bool emu4 =  roads_pos_top_f.size() > 0 || roads_pos_bot_f.size() > 0 ||
               roads_neg_top_f.size() > 0 || roads_neg_bot_f.size() > 0;
  if (emu1) h1_cnt->AddBinContent(11);
  if (emu2) h1_cnt->AddBinContent(12);
  if (emu3) h1_cnt->AddBinContent(13);
  if (emu4) h1_cnt->AddBinContent(14);

  if (is_not_FPGA1) {
    h1_cnt->AddBinContent(21);
    if (emu1) {
      if (is_FPGA1) h1_eff_NIM3->Fill(1);
      else          h1_eff_NIM3->Fill(0);
    }
    //if (emu1 && ! is_FPGA1) {
    //  cout << "Inefficient event:\n  PosTop: ";
    //  for (auto it = roads_pos_top_f.begin(); it != roads_pos_top_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  PosBot: ";
    //  for (auto it = roads_pos_bot_f.begin(); it != roads_pos_bot_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  NegTop: ";
    //  for (auto it = roads_neg_top_f.begin(); it != roads_neg_top_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  NegBot: ";
    //  for (auto it = roads_neg_bot_f.begin(); it != roads_neg_bot_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  ";
    //  cout << PrintHitVec("H1T", vecH1T) << "\n  "
    //       << PrintHitVec("H2T", vecH2T) << "\n  "
    //       << PrintHitVec("H3T", vecH3T) << "\n  "
    //       << PrintHitVec("H4T", vecH4T) << "\n  "
    //       << PrintHitVec("H1B", vecH1B) << "\n  "
    //       << PrintHitVec("H2B", vecH2B) << "\n  "
    //       << PrintHitVec("H3B", vecH3B) << "\n  "
    //       << PrintHitVec("H4B", vecH4B) << "\n\n";
    //}
  }

  //if (emu1 && evt->get_trigger(SQEvent::NIM3)){
  //if ( emu1 && (evt->get_trigger() & 991) ){ // 0b1111011111 = 991 = non-FPGA1 triggers
  //  if (is_FPGA1) h1_eff_NIM3->Fill(1);
  //  else          h1_eff_NIM3->Fill(0);
  //}
  if (is_FPGA1) {
    if (emu1) h1_purity->Fill(1);
    else      h1_purity->Fill(0);
    //if (! emu1) {
    //  cout << "Impure event:\n  PosTop: ";
    //  for (auto it = roads_pos_top_f.begin(); it != roads_pos_top_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  PosBot: ";
    //  for (auto it = roads_pos_bot_f.begin(); it != roads_pos_bot_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  NegTop: ";
    //  for (auto it = roads_neg_top_f.begin(); it != roads_neg_top_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  NegBot: ";
    //  for (auto it = roads_neg_bot_f.begin(); it != roads_neg_bot_f.end(); it++) cout << " " << (*it)->str(1);
    //  cout << "\n  ";
    //  cout << PrintHitVec("H1T", vecH1T) << "\n  "
    //       << PrintHitVec("H2T", vecH2T) << "\n  "
    //       << PrintHitVec("H3T", vecH3T) << "\n  "
    //       << PrintHitVec("H4T", vecH4T) << "\n  "
    //       << PrintHitVec("H1B", vecH1B) << "\n  "
    //       << PrintHitVec("H2B", vecH2B) << "\n  "
    //       << PrintHitVec("H3B", vecH3B) << "\n  "
    //       << PrintHitVec("H4B", vecH4B) << "\n\n";
    //}
  }
 
*/
	//Ievgen
	event_counter++;

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::EndOnlMon(PHCompositeNode* topNode)
{ 
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigEP::FindAllMonHist()
{
  ostringstream oss; 

  h1_cnt = FindMonHist("h1_cnt");
  if (! h1_cnt) return 1; 

  oss.str("");
  oss << "h1_purity_" << 0;
  h1_purity = FindMonHist(oss.str().c_str());
  if (! h1_purity) return 1;

  oss.str("");
  oss << "h1_eff_NIM3_" << 0;
  h1_eff_NIM3 = FindMonHist(oss.str().c_str());
  if (! h1_eff_NIM3) return 1;

  return 0;
}

int OnlMonTrigEP::DrawMonitor()
{
  //DRAWING HISTOGRAMS ON .PNG FILES ******************************************

  int n_emu1 = (int)h1_cnt->GetBinContent(11);
  int n_emu2 = (int)h1_cnt->GetBinContent(12);
  int n_emu3 = (int)h1_cnt->GetBinContent(13);
  int n_emu4 = (int)h1_cnt->GetBinContent(14);
  //int n_emu5 = (int)h1_cnt->GetBinContent(15);
  int n_not_fpga1 = (int)h1_cnt->GetBinContent(21);

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1,2);
  TVirtualPad* pad00 = pad0->cd(1);
  pad00->SetGrid();
  h1_purity->Draw();
  ostringstream oss;
  
  double pur = h1_purity->GetMean();
  oss << "Purity = " << pur;
  TText* text = new TText();
  text->SetNDC(true);
  text->SetTextAlign(22);
  text->DrawText(0.3, 0.5, oss.str().c_str());
  
  double eff_NIM3 = h1_eff_NIM3->GetMean();
  ostringstream oss0; 
  TVirtualPad* pad01 = pad0->cd(2);
  pad01->SetGrid();
  h1_eff_NIM3->Draw();
  oss0 << "Efficiency = " << eff_NIM3;
  TText* text0 = new TText();
  text0->SetNDC(true);
  text0->SetTextAlign(22);
  text0->DrawText(0.3, 0.5, oss0.str().c_str());

  can0->AddMessage(TString::Format("Emulated FPGA1 = %d", n_emu1).Data());
  can0->AddMessage(TString::Format("Emulated FPGA2 = %d", n_emu2).Data());
  can0->AddMessage(TString::Format("Emulated FPGA3 = %d", n_emu3).Data());
  can0->AddMessage(TString::Format("Emulated FPGA4 = %d", n_emu4).Data());
  can0->AddMessage(TString::Format("Not FPGA1 = %d", n_not_fpga1).Data());

  return 0;
}

void OnlMonTrigEP::SetDet()
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


void OnlMonTrigEP::FindFiredRoads(const int top0bot1, vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X, TriggerRoads* roads, std::vector<TriggerRoad1*>& list_fired_roads)
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
        set_ele4.find(road->H4X) != set_ele4.end()   ) list_fired_roads.push_back(road);
  }
}

std::string OnlMonTrigEP::PrintHitVec(const std::string title, const std::vector<SQHit*>* vec)
{
  ostringstream oss;
  oss << title << ":";
  for (auto it = vec->begin(); it != vec->end(); it++) {
    if ((*it)->is_in_time()) oss << " " << (*it)->get_element_id();
  }
  return oss.str();
}

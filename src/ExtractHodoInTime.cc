#include <sstream>
#include <iomanip>
#include <TStyle.h>
#include <TSystem.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TLine.h>
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
#include "ExtractHodoInTime.h"
using namespace std;

ExtractHodoInTime::ExtractHodoInTime(const std::string& name)
  : SubsysReco(name)
  , m_label("hodo")
  , m_list_det_name{
    "H1T", "H1B", "H1L", "H1R", 
    "H2T", "H2B", "H2L", "H2R", 
    "H3T", "H3B", 
    "H4Tu", "H4Td",   "H4Bu", "H4Bd", 
    "H4Y1Ll", "H4Y1Lr", "H4Y1Rl", "H4Y1Rr", 
    "H4Y2Ll", "H4Y2Lr", "H4Y2Rl", "H4Y2Rr", 
    "BeforeInhNIM", "BeforeInhMatrix", "AfterInhNIM", "AfterInhMatrix"}
  , m_dir_out("result")
  , m_file(0)
{
  ;
}

int ExtractHodoInTime::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int ExtractHodoInTime::InitRun(PHCompositeNode* topNode)
{
  ///
  /// Input
  ///
  m_evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  m_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!m_evt || !m_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  ///
  /// Output
  ///
  gSystem->mkdir(m_dir_out.c_str(), true);
  m_ofs_log.open((m_dir_out+"/log.txt").c_str());

  m_file = new TFile((m_dir_out+"/output.root").c_str(), "RECREATE");

  const double DT = 12/9.0; // 4/9 ns per single count of Taiwan TDC
  const double T0 =    0.5*DT;
  const double T1 = 1500.5*DT;
  const int    NT = 1500;

  ostringstream oss;
  oss << setfill('0');
  GeomSvc* geom = GeomSvc::instance();
  for (unsigned int i_det = 0; i_det < m_list_det_name.size(); i_det++) {
    string name = m_list_det_name[i_det];
    int id = geom->getDetectorID(name);
    if (id <= 0) {
      cerr << "!ERROR!  ExtractHodoInTime::InitRun():  Invalid ID (" << id << ") for name (" << name << ").  Abort." << endl;
      exit(1);
    }
    m_list_det_id.push_back(id);
    int n_ele = geom->getPlaneNElements(id);
    if      (name.substr(0, 2) == "H4") n_ele = 16;
    else if (name.length() >= 6 && name.substr(0, 6) == "Before") n_ele =  5;
    else if (name.length() >= 5 && name.substr(0, 5) == "After" ) n_ele =  5;
    //cout << "  " << setw(6) << name << " = " << id << " " << n_ele << endl;
    if (n_ele <= 0) n_ele = 5;

    oss.str("");
    oss << "h2_time_ele_" << setw(2) << id << "_" << name;
    TH2* h2 = new TH2D(oss.str().c_str(), "", NT, T0, T1,  n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << " (" << id << ");TDC time (ns);Element ID;Hit count";
    h2->SetTitle(oss.str().c_str());
    m_map_h2_time_ele[id] = h2;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int ExtractHodoInTime::process_event(PHCompositeNode* topNode)
{
  //int spill_id = m_evt->get_spill_id();
  //int event_id = m_evt->get_event_id();

  ///
  /// Event selection
  ///
  if (! (m_evt->get_trigger() & 999)) { // 0b1111100111 = 999 = "FPGA1-5,NIM1-3"
    return Fun4AllReturnCodes::EVENT_OK;
  }

  ///
  /// Get & fill the hit info
  ///
  for (auto it = m_hit_vec->begin(); it != m_hit_vec->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    int ele = hit->get_element_id();
    double time = hit->get_tdc_time();
    if (m_map_h2_time_ele.find(det) != m_map_h2_time_ele.end()) {
      m_map_h2_time_ele[det]->Fill(time, ele);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int ExtractHodoInTime::End(PHCompositeNode* topNode)
{
  int err_level_org = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  //cout << "ExtractHodoInTime::End()" << endl;
  ostringstream oss;
  oss << setfill('0') << m_dir_out << "/"<< m_label << "_in_time.txt";
  string fn_text = oss.str();
  ofstream ofs(fn_text.c_str());

  int n_ok   = 0;
  int n_fair = 0;
  int n_ng   = 0;

  TCanvas* c1 = new TCanvas("c1", "");
  c1->SetGrid();
  for (unsigned int i_det = 0; i_det < m_list_det_id.size(); i_det++) {
    int    det_id   = m_list_det_id  [i_det];
    string det_name = m_list_det_name[i_det];
    TH2* h2 = m_map_h2_time_ele[det_id];
    int n_ele = h2->GetNbinsY();
    for (int i_ele = 1; i_ele <= n_ele; i_ele++) {
      TH1* h1 = h2->ProjectionX("h1", i_ele, i_ele);
      
      int status_peak, bin_peak;
      FindInTimePeak(h1, status_peak, bin_peak);
      double time_peak = h1->GetBinCenter (bin_peak);
      double cont_peak = h1->GetBinContent(bin_peak);
      ofs << det_name << "\t" << det_id << "\t" << i_ele << "\t" << time_peak << "\t" << status_peak << "\n";

      if      (status_peak == PEAK_OK  ) n_ok++;
      else if (status_peak == PEAK_FAIR) n_fair++;
      else                               n_ng++;

      h1->Draw();
      oss.str("");
      oss << h1->GetTitle() << " | Paddle " << i_ele;
      h1->SetTitle(oss.str().c_str());

      UtilHist::AutoSetRange(h1);
      TLine line;
      line.SetLineColor(kRed);
      line.DrawLine(time_peak, 0.2*cont_peak, time_peak, 0.8*cont_peak);
      oss.str("");
      oss << m_dir_out << "/" << h2->GetName() << "_" << setw(2) << i_ele << ".png";
      c1->SaveAs(oss.str().c_str());
      delete h1;
    }
  }
  delete c1;

  ofs.close();
  DrawPeakTimeVsElement(fn_text, m_dir_out);

  m_ofs_log << "n_ok   = " << n_ok << "\n"
            << "n_fair = " << n_fair << "\n"
            << "n_ng   = " << n_ng << "\n";
  m_ofs_log.close();

  m_file->cd();
  m_file->Write();
  m_file->Close();
  gErrorIgnoreLevel = err_level_org;
  return Fun4AllReturnCodes::EVENT_OK;
}

void ExtractHodoInTime::SetOutputDir(const std::string dir_base, const int run_id)
{
  ostringstream oss;
  oss << setfill('0') << dir_base << "/run_" << setw(6) << run_id;
  m_dir_out = oss.str();
}

void ExtractHodoInTime::DrawPeakTimeVsElement(const std::string txt_in, const std::string dir_out)
{
  map<string, int > map_n_ent;
  map<string, TH1*> map_h1;

  ostringstream oss;
  oss << setfill('0');
  //TH1* h1 = new TH1D("h1", ";;Peak TDC time (ns)", 1000, 0.5, 1000.5);

  ifstream ifs(txt_in);
  string det_name;
  int det_id, ele_id, status;
  double time_peak;
  //int n_ent = 0;
  while (ifs >> det_name >> det_id >> ele_id >> time_peak >> status) {
    string key;
    if      (det_name == "H1T" || det_name == "H1B") key = "H1X";
    else if (det_name == "H1L" || det_name == "H1R") key = "H1Y";
    else if (det_name == "H2T" || det_name == "H2B") key = "H2X";
    else if (det_name == "H2L" || det_name == "H2R") key = "H2Y";
    else if (det_name == "H3T" || det_name == "H3B") key = "H3X";
    else if (det_name.substr(0, 4) == "H4Y1") key = "H4Y1";
    else if (det_name.substr(0, 4) == "H4Y2") key = "H4Y2";
    else if (det_name.substr(0, 2) == "H4"  ) key = "H4X";
    else if (det_name.substr(0, 6) == "Before") key = "BeforeInh";
    else if (det_name.substr(0, 5) == "After" ) key = "AfterInh";
    else continue;
    int n_ent = ++map_n_ent[key];
    TH1* h1;
    if (n_ent == 1) {
      oss.str("");
      oss << "h1_" << key;
      h1 = map_h1[key] = new TH1D(oss.str().c_str(), "", 100, 0.5, 100.5);
      oss.str("");
      oss << key << ";;Peak TDC time (ns)";
      h1->SetTitle(oss.str().c_str());
    } else {
      h1 = map_h1[key];
    }

    if (status < 2) h1->SetBinContent(n_ent, time_peak);
    oss.str("");
    oss << det_name << " : " << setw(2) << ele_id;
    h1->GetXaxis()->SetBinLabel(n_ent, oss.str().c_str());

    //if (det_name.substr(0, 1) != 'H') continue;
    //if (status == 2) time_peak = 0;
    //h1->SetBinContent(++n_ent, time_peak);
    //oss.str("");
    //oss << det_name << " : " << setw(2) << ele_id;
    //h1->GetXaxis()->SetBinLabel(n_ent, oss.str().c_str());
  }

  gStyle->SetOptStat(0);
  TCanvas* c1 = new TCanvas("c1", "", 1600, 600);
  c1->SetMargin(0.07, 0.03, 0.10, 0.10); // (l, r, b, t)
  c1->SetGrid();
  for (auto it = map_h1.begin(); it != map_h1.end(); it++) {
    string key = it->first;
    TH1*   h1  = it->second;
    int  n_ent = map_n_ent[key];
    h1->GetXaxis()->SetRange(1, n_ent);
    h1->GetYaxis()->SetRangeUser(h1->GetMinimum(1)-10, h1->GetMaximum()+10);
    h1->GetYaxis()->SetTitleOffset(0.6);
    h1->SetMarkerStyle(21);
    h1->SetMarkerColor(kRed);
    h1->Draw("P");
    oss.str("");
    oss << dir_out << "/h1_time_vs_ele_" << key << ".png";
    c1->SaveAs(oss.str().c_str()); // (dir_out.c_str());
    delete h1;
  }

  //TCanvas* c1 = new TCanvas("c1", "", 2400, 600);
  //gStyle->SetOptStat(0);
  //c1->SetGrid();
  //h1->GetXaxis()->SetRange(1, n_ent);
  //h1->GetYaxis()->SetRangeUser(h1->GetMinimum(1)-10, h1->GetMaximum()+10);
  //h1->SetMarkerStyle(7);
  //h1->Draw("P");
  //c1->SaveAs(dir_out.c_str());
  delete c1;
  //delete h1;
}

void ExtractHodoInTime::FindInTimePeak(TH1* h1, int& status_peak, int& bin_peak)
{
  bin_peak        = h1->GetMaximumBin();
  int    n_bin    = h1->GetNbinsX();
  double cont_max = h1->GetMaximum();
  if (cont_max < 100) {
    status_peak = PEAK_NG;
    return;
  }

  double cont_max2 = 0;
  int    bin_max2  = -1;

  int bin_now = bin_peak;
  // Search the low side for the 2nd max.
  while (--bin_now > 0) { // Move away from the 1st peak.
    if (h1->GetBinContent(bin_now) > h1->GetBinContent(bin_now+1)) break;
  }
  while (--bin_now > 0) {
    double cont = h1->GetBinContent(bin_now);
    if (cont >= cont_max2) {
      cont_max2 = cont;
      bin_max2  = bin_now;
    }
  }
  // Search the low side for the 2nd max.
  bin_now = bin_peak;
  while (++bin_now <= n_bin) { // Move away from the 1st peak.
    if (h1->GetBinContent(bin_now) > h1->GetBinContent(bin_now-1)) break;
  }
  while (++bin_now <= n_bin) {
    double cont = h1->GetBinContent(bin_now);
    if (cont >= cont_max2) {
      cont_max2 = cont;
      bin_max2  = bin_now;
    }
  }
  if (bin_max2 < 0) {
    status_peak = PEAK_NG;
    return;
  }

  double diff = cont_max - cont_max2;
  double diff_err = sqrt(cont_max + cont_max2);
  if (diff / diff_err > 4) status_peak = PEAK_OK;
  else                     status_peak = PEAK_FAIR;
}

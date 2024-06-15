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
#include "ExtractV1495InTime.h"
using namespace std;


ExtractV1495InTime::ExtractV1495InTime(const std::string& name)
  : ExtractHodoInTime(name)
{
  m_label = "v1495";
  m_list_det_name = {
    "H1T", "H1B", "H2T", "H2B", "H3T", "H3B", 
//    "H4T", "H4B" };
    "H4Tu", "H4Td", "H4Bu", "H4Bd" };
}

int ExtractV1495InTime::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int ExtractV1495InTime::InitRun(PHCompositeNode* topNode)
{
  ///
  /// Input
  ///
  m_evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  m_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!m_evt || !m_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  ///
  /// Output
  ///
  gSystem->mkdir(m_dir_out.c_str(), true);
  m_ofs_log.open((m_dir_out+"/log.txt").c_str());

  m_file = new TFile((m_dir_out+"/output.root").c_str(), "RECREATE");

  const double T0 =    0.5;
  const double T1 = 2000.5;
  const int    NT = 2000;

  ostringstream oss;
  oss << setfill('0');
  GeomSvc* geom = GeomSvc::instance();
  for (unsigned int i_det = 0; i_det < m_list_det_name.size(); i_det++) {
    string name = m_list_det_name[i_det];
    int id = geom->getDetectorID(name);
    if (id <= 0) {
      cerr << "!ERROR!  ExtractV1495InTime::InitRun():  Invalid ID (" << id << ") for name (" << name << ").  Abort." << endl;
      exit(1);
    }
    m_list_det_id.push_back(id);
    int n_ele;
    if (name.substr(0, 2) == "H4") n_ele = 16;
    else n_ele = geom->getPlaneNElements(id);

    oss.str("");
    oss << "h2_time_ele_" << setw(2) << id << "_" << name;
    TH2* h2 = m_map_h2_time_ele[id] = new TH2D(oss.str().c_str(), "", NT, T0, T1,  n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << " (" << id << ");TDC time (ns);Element ID;Hit count";
    h2->SetTitle(oss.str().c_str());
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int ExtractV1495InTime::process_event(PHCompositeNode* topNode)
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

int ExtractV1495InTime::End(PHCompositeNode* topNode)
{
  int err_level_org = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kWarning;
  //cout << "ExtractV1495InTime::End()" << endl;
  ostringstream oss;
  oss << setfill('0') << m_dir_out << "/" << m_label << "_in_time.txt";
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

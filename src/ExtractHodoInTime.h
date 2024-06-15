#ifndef _EXTRACT_HODO_IN_TIME__H_
#define _EXTRACT_HODO_IN_TIME__H_
#include <fstream>
#include <unordered_map>
#include <fun4all/SubsysReco.h>
class TFile;
class TH1;
class TH2;
class SQEvent;
class SQHitVector;

class ExtractHodoInTime: public SubsysReco {
 protected:
  typedef enum { PEAK_OK = 0, PEAK_FAIR = 1, PEAK_NG = 2 } PeakStatus_t;
  std::string m_label;
  std::vector<std::string> m_list_det_name; //< List of detectors that you want to analyze
  std::vector<int> m_list_det_id;
  std::string m_dir_out;
  std::ofstream m_ofs_log;

  SQEvent* m_evt;
  SQHitVector* m_hit_vec;

  TFile* m_file;
  std::unordered_map<int, TH2*> m_map_h2_time_ele;

 public:
  ExtractHodoInTime(const std::string& name="ExtractHodoInTime");
  virtual ~ExtractHodoInTime() {;}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void        SetOutputDir(const std::string dir_base, const int run_id);
  void        SetOutputDir(const std::string dir_out) { m_dir_out = dir_out; }
  std::string GetOutputDir() const             { return m_dir_out; }

  static void DrawPeakTimeVsElement(const std::string txt_in, const std::string dir_out);

 protected:
  void FindInTimePeak(TH1* h1, int& status_peak, int& bin_peak);
};

#endif // _EXTRACT_HODO_IN_TIME__H_

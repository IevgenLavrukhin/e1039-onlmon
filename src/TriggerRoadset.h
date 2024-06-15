#ifndef _TRIGGER_ROADSET__H_
#define _TRIGGER_ROADSET__H_
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
 
class TriggerRoad1 {
public:
 int road_id;
 int charge;
 int H1X;
 int H2X;
 int H3X;
 int H4X;
 double signal;
 double background;
 
 TriggerRoad1();
 virtual ~TriggerRoad1() {;}

 static void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb);
 static int  Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb);

 friend std::ostream& operator<<(std::ostream& os, const TriggerRoad1& rd);
};


class TriggerRoads {
  std::string m_file_name;
  //int N_ROADS;
  int m_pol; // mu+ = +1, mu- = -1
  int m_top_bot; // Top = +1, Bottom = -1
  std::vector<TriggerRoad1> m_roads;
  std::unordered_map<int, int> m_idx_map;
 
 public:  
  TriggerRoads(const int pol, const int top_bot); // const std::string file_name
  virtual ~TriggerRoads() {;}

  unsigned int GetNumRoads() const { return m_roads.size(); }
  TriggerRoad1* GetRoad(const int idx);
  TriggerRoad1* FindRoad(const int road_id);

  int Charge() const { return m_pol; }
  int TopBot() const { return m_top_bot; }
  int LoadConfig(const std::string file_name);

  friend std::ostream& operator<<(std::ostream& os, const TriggerRoads& tr);
};

class TriggerRoadset {
  std::string m_dir_conf;
  int m_roadset;
  int m_LBTop;
  int m_LBBot;
  TriggerRoads m_pos_top;
  TriggerRoads m_pos_bot;
  TriggerRoads m_neg_top;
  TriggerRoads m_neg_bot;
 
 public:  
  TriggerRoadset();
  virtual ~TriggerRoadset() {;}

  int RoadsetID() const { return m_roadset; }
  int LBTop    () const { return m_LBTop; }
  int LBBot    () const { return m_LBBot; }

  TriggerRoads* PosTop() { return &m_pos_top; }
  TriggerRoads* PosBot() { return &m_pos_bot; }
  TriggerRoads* NegTop() { return &m_neg_top; }
  TriggerRoads* NegBot() { return &m_neg_bot; }
  const TriggerRoads* PosTop() const { return &m_pos_top; }
  const TriggerRoads* PosBot() const { return &m_pos_bot; }
  const TriggerRoads* NegTop() const { return &m_neg_top; }
  const TriggerRoads* NegBot() const { return &m_neg_bot; }

  int LoadConfig(const std::string dir);
  int LoadConfig(const int roadset_id);
  int LoadConfig(const int firmware_LBTop, const int firmware_LBBot);

  friend std::ostream& operator<<(std::ostream& os, const TriggerRoadset& rs);
};

#endif /* _TRIGGER_ROADSET__H_ */

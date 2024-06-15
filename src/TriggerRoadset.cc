//TriggerRoadset.cc
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cstring>
#include <vector>
#include <TSystem.h>
#include "TriggerRoadset.h"
using namespace std;

TriggerRoad1::TriggerRoad1()
  : road_id(0)
  , charge (0)
  , H1X    (0)
  , H2X    (0)
  , H3X    (0)
  , H4X    (0)
  , signal (0)
  , background(0)
{
  ;
}

void TriggerRoad1::Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb)
{
  int rr = abs(road) - 1;
  h1 = 1 + (rr / 4096);
  h2 = 1 + (rr /  256) % 16;
  h3 = 1 + (rr /   16) % 16;
  h4 = 1 +  rr         % 16;
  tb = road / abs(road);
}

int TriggerRoad1::Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb)
{
  int rr = 4096*(h1-1) + 256*(h2-1) + 16*(h3-1) + h4;
  if (tb < 0) rr *= -1;
  return rr;
}

std::ostream& operator<<(std::ostream& os, const TriggerRoad1& rd)
{
  os << setw(6) << rd.road_id << ": " << rd.charge << " (" << rd.H1X << ", " << rd.H2X << ", " << rd.H3X << ", " << rd.H4X << ")";
  return os;
}

//// TriggerRoads ////////////////////////////////////////////////////////////

TriggerRoads::TriggerRoads(const int pol, const int top_bot)
  : m_file_name("")
  , m_pol(pol)
  , m_top_bot(top_bot)
{
  ;
}

TriggerRoad1* TriggerRoads::GetRoad(const int idx)
{
  if (idx < 0 || idx > (int)m_roads.size()) return 0;
  return &m_roads[idx];
}

TriggerRoad1* TriggerRoads::FindRoad(const int road_id)
{
  auto it = m_idx_map.find(road_id);
  if (it != m_idx_map.end()) return &m_roads[it->second];
  return 0;
}

int TriggerRoads::LoadConfig(const std::string file_name)
{
  //cout << "TriggerRoads::LoadConfig(" << file_name << ")\n";
  m_file_name = file_name;
  ifstream ifs(file_name);
  if (! ifs) return 1;

  int idx = 0;
  string buffer;
  istringstream iss;
  while (getline(ifs, buffer)) {
    if (buffer[0] == '#' || buffer[0] == 'r') continue; // 'r' of 'roadID'
    iss.clear(); // clear any error flags
    iss.str(buffer);

    TriggerRoad1 road;
    if (! (iss >> road.road_id >> road.charge
               >> road.H1X >> road.H2X >> road.H3X >> road.H4X
               >> road.signal >> road.background)) continue;
    if (road.road_id * m_top_bot <= 0) continue; // top-bottom mismatch
    if (road.charge  * m_pol     <= 0) continue; // charge mismatch
    m_roads.push_back(road);
    m_idx_map[road.road_id] = idx++;
  }
  ifs.close();
  return 0; // no validation so far
}

std::ostream& operator<<(std::ostream& os, const TriggerRoads& tr)
{
  ostringstream oss;
  oss << showpos << "Charge = " << tr.Charge() << ", TopBot = " << tr.TopBot() << ", N of roads = "
      << noshowpos << tr.GetNumRoads();
  os << oss.str();
  return os;
}

//// TriggerRoadset ////////////////////////////////////////////////////////////

TriggerRoadset::TriggerRoadset() 
  : m_dir_conf("")
  , m_roadset(0)
  , m_LBTop(0)
  , m_LBBot(0)
  , m_pos_top(+1, +1)
  , m_pos_bot(+1, -1)
  , m_neg_top(-1, +1)
  , m_neg_bot(-1, -1)
{ 
  ;
}

int TriggerRoadset::LoadConfig(const std::string dir)
{
  int ret = 0;
  ret += m_pos_top.LoadConfig(dir+"/rs_LB_pos_top.txt");
  ret += m_pos_bot.LoadConfig(dir+"/rs_LB_pos_bot.txt");
  ret += m_neg_top.LoadConfig(dir+"/rs_LB_neg_top.txt");
  ret += m_neg_bot.LoadConfig(dir+"/rs_LB_neg_bot.txt");
  return ret;
}

int TriggerRoadset::LoadConfig(const int roadset_id)
{
  m_roadset = roadset_id;
  ostringstream oss;
  if (m_dir_conf != "") oss << m_dir_conf;
  else oss << gSystem->Getenv("E1039_RESOURCE") << "/trigger/rs";
  oss << "/rs" << roadset_id;
  return LoadConfig(oss.str());
}

int TriggerRoadset::LoadConfig(const int firmware_LBTop, const int firmware_LBBot)
{
  ostringstream oss;
  oss << gSystem->Getenv("E1039_RESOURCE") << "/trigger/rs/firmware_ctrl.txt";
  string fn_ctrl = oss.str();
  //cout << "TriggerRoadset::LoadConfig(" << firmware_LBTop << ", " << firmware_LBBot << "): " << fn_ctrl << endl;
  ifstream ifs(fn_ctrl);
  if (! ifs) return 1;

  int roadset_id = -1;
  string buffer;
  istringstream iss;
  while (getline(ifs, buffer)) {
    if (buffer[0] == '#') continue;
    iss.clear(); // clear any error flags
    iss.str(buffer);

    int id;
    string str_top, str_bot;
    if (! (iss >> id >> str_top >> str_bot)) continue;
    if (str_top.substr(0, 4) != "0xB0") continue;
    if (str_bot.substr(0, 4) != "0xB1") continue;
    int top = stoi(str_top.substr(2), 0, 16);
    int bot = stoi(str_bot.substr(2), 0, 16);
    if (top == firmware_LBTop && bot == firmware_LBBot) {
      roadset_id = id;
      m_LBTop = top;
      m_LBBot = bot;
      break;
    }
  }
  ifs.close();
  if (roadset_id < 0) return 2;
  return LoadConfig(roadset_id);
}

std::ostream& operator<<(std::ostream& os, const TriggerRoadset& rs)
{
  os << "Roadset = " << rs.RoadsetID() << ", LBTop = " << hex << rs.LBTop() << ", LBBot = " << rs.LBBot() << dec << "\n"
     << "  " << *rs.PosTop() << "\n"
     << "  " << *rs.PosBot() << "\n"
     << "  " << *rs.NegTop() << "\n"
     << "  " << *rs.NegBot() << "\n";
  return os;
}

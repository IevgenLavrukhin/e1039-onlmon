R__LOAD_LIBRARY(OnlMon)

int TestOnlMonClient(const int run_id=5848, const int n_dst_ana=1, const int spill_id=-1)
{
  gSystem->Umask(0002);
  UtilOnline::SetOnlMonDir("/dev/shm/$USER/onlmon/plots");
  OnlMonServer* se = OnlMonServer::instance();
  se->setRun(run_id); // This sets the `RUNNUMBER` flag.
  se->SetOnline(false);

  ///
  /// Enable only what you want to test
  ///
  //se->registerSubsystem(new OnlMonMainDaq());
  //se->registerSubsystem(new OnlMonTrigSig());
  //se->registerSubsystem(new OnlMonTrigNim());
  //se->registerSubsystem(new OnlMonTrigV1495());
  se->registerSubsystem(new OnlMonTrigRoad ());
  se->registerSubsystem(new OnlMonTrigEP   ());
  //se->registerSubsystem(new OnlMonQie());
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H1X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H2X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H3X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H4X, 1));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H3X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1Y));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2Y));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y1));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y2));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1T));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1B));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2T));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2B));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4T));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4B));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1L));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1R));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2L));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2R));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D0));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D2));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D3p));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D3m));
  //se->registerSubsystem(new OnlMonProp (OnlMonProp::P1));
  //se->registerSubsystem(new OnlMonProp (OnlMonProp::P2));

  ///
  /// Fun4All Input & Event Processing.
  ///
  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(in);

  vector<string> list_dst;
  if (spill_id >= 0) {
    list_dst.push_back(UtilOnline::GetSpillDstDir(run_id)+"/"+UtilOnline::GetSpillDstFile(run_id, spill_id));
  } else {
    list_dst = UtilOnline::GetListOfSpillDSTs(run_id);
  }
  unsigned int n_dst = list_dst.size();
  cout << "N of DST files = " << n_dst << ", Max N to analyze = " << n_dst_ana << endl;
  for (unsigned int i_dst = 0; i_dst < n_dst; i_dst++) {
    string fn_in = list_dst[i_dst];
    cout << "DST file = " << fn_in << endl;
    in->fileopen(fn_in);
    se->run();
    if (n_dst_ana > 0 && i_dst+1 == n_dst_ana) break;
  }

  se->End();
  delete se;
  cout << "\nAll finished.\n"
       << "OnlMon output dir = " << UtilOnline::GetOnlMonDir() << endl;
  exit(0);
}

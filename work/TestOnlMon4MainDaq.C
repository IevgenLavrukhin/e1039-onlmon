/// TestOnlMon4MainDaq.C
//R__LOAD_LIBRARY(interface_main)
R__LOAD_LIBRARY(OnlMon)

int TestOnlMon4MainDaq()
{
  return 0;
}

void ConfigServer()
{
  OnlMonServer::SetPort (9086);
  OnlMonServer::SetPort0(9086);
  OnlMonServer::SetNumPorts(1);
}

int OnlMon4MainDaq()
{
  if (gROOT->IsBatch()) {
    cout << "ERROR: This macro cannot run in the batch mode (-b).  Abort.\n";
    exit(1);
  }
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  ConfigServer();

  OnlMonClientList_t list_omc;
  list_omc.push_back(new OnlMonMainDaq());
  list_omc.push_back(new OnlMonTrigNim());
  OnlMonUI* ui = new OnlMonUI(&list_omc);
  //ui->SetCycleInterval(5); // default = 10 sec
  //ui->SetAutoCycleFlag(true); // default = false
  ui->Run();
  
  return 0;
}

int Fun4MainDaq(const int run=1818, const int nevent=0)
{
  gSystem->Umask(0002);
  gSystem->Load("libinterface_main.so");
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  ConfigServer();
  UtilOnline::UseOutputLocationForDevel();

  string fn_in = "/data2/e1039/dst/" + UtilOnline::RunNum2DstFile(run);

  OnlMonServer* se = OnlMonServer::instance();
  //se->Verbosity(1);
  se->SetOnline(true);
  OnlMonComm::instance()->SetMaxNumSelSpills(20);

  se->StartServer();
  se->registerSubsystem(new OnlMonMainDaq());
  se->registerSubsystem(new OnlMonTrigNim());
  se->registerSubsystem(new AnaWait(1, 0)); // (spill, event)

  Fun4AllInputManager *in = new Fun4AllDstInputManager("MainDaq");
  in->fileopen(fn_in);
  se->registerInputManager(in);

  se->run(nevent);
  se->End();
  
  delete se;
  cout << "Fun4MainDaq Done!" << endl;
  return 0;
}

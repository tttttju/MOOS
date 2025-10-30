/*****************************************************************/
/*    NAME: MOOS-IvP Community                                   */
/*    FILE: uReplayMain.cpp                                      */
/*    DATE: March 10th, 2025                                     */
/*                                                               */
/* This file is part of the moos-ivp-extend repository.          */
/*                                                               */
/* moos-ivp-extend is free software: you can redistribute it     */
/* and/or modify it under the terms of the GNU General Public    */
/* License as published by the Free Software Foundation, either  */
/* version 3 of the License, or (at your option) any later        */
/* version.                                                      */
/*                                                               */
/* moos-ivp-extend is distributed in the hope that it will be    */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE.  See the GNU General Public License for more         */
/* details.                                                      */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with moos-ivp-extend.  If not, see              */
/* <http://www.gnu.org/licenses/>.                               */
/*****************************************************************/

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include "REPLAY_GUI.h"
#include "MBUtils.h"
#include "OpenURL.h"
#include "ReleaseInfo.h"
#include "LogViewLauncher.h"
#include <FL/Fl_File_Chooser.H>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

void help_message();
void idleProc(void*);
std::vector<std::string> collectSqliteFiles(const std::string& dirpath);
std::string promptForSqliteDirectory();

REPLAY_GUI* gui = 0;

//--------------------------------------------------------
// Procedure: idleProc

void idleProc(void *)
{
  if(gui)
    gui->conditionalStep();
  Fl::flush();
  millipause(10);
}

//--------------------------------------------------------
// Procedure: main

int main(int argc, char *argv[])
{
  LogViewLauncher launcher;

  if(scanArgs(argc, argv, "-w", "-web", "--web"))
    openURLX("https://oceanai.mit.edu/ivpman/apps/alogview");

  bool log_provided = false;
  std::vector<std::string> manual_logs;

  for(int i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-v") || (argi=="--version") || (argi=="-version")) {
      showReleaseInfo("uReplay", "gpl");
      return(0);
    }
    else if((argi == "-h") || (argi == "--help") || (argi=="-help")) {
      help_message();
      return(0);
    }
    else if(strEnds(argi, ".alog") || strEnds(argi, ".sqlite"))
      log_provided = true;
    else if((argi == "-vb") || (argi == "--verbose"))
      launcher.setVerbose();
  }

  if(!log_provided) {
    string chosen_dir = promptForSqliteDirectory();
    if(chosen_dir == "") {
      cout << "No log file given - exiting" << endl;
      return(1);
    }
    manual_logs = collectSqliteFiles(chosen_dir);
    if(manual_logs.empty()) {
      cout << "No .sqlite files found in directory - exiting" << endl;
      return(1);
    }
    for(unsigned int i=0; i<manual_logs.size(); ++i)
      cout << "Adding log file: " << manual_logs[i] << endl;
    launcher.setExtraLogs(manual_logs);
  }

  gui = launcher.launch(argc, argv);

  if(gui) {
    gui->updateXY();
    gui->resizeWidgetsShape();
    Fl::add_idle(idleProc);
    return(Fl::run());
  }
  else
    return(0);
}

//--------------------------------------------------------
// Procedure: help_message()

void help_message()
{
  cout << "Usage:                                                        " << endl;
  cout << "  uReplay file.alog [another_file.alog] [OPTIONS]              " << endl;
  cout << "                                                              " << endl;
  cout << "Synopsis:                                                     " << endl;
  cout << "  Replays vehicle data from MOOS log archives (.alog/.sqlite)." << endl;
  cout << "  Provides synchronized navigation and log-plot viewers that   " << endl;
  cout << "  mirror the alogview workflow while living inside             " << endl;
  cout << "  moos-ivp-extend.                                            " << endl;
  cout << "                                                              " << endl;
  cout << "Standard Arguments:                                           " << endl;
  cout << "  file.alog - The input logfile. Multiple files allowed.      " << endl;
  cout << "                                                              " << endl;
  cout << "Options:                                                      " << endl;
  cout << "  -h,--help       Displays this help message                  " << endl;
  cout << "  -v,--version    Displays the current release version        " << endl;
  cout << "  -vb,--verbose   Verbose output during configuration         " << endl;
  cout << "  --bg=file.tiff  Specify an alternate background image.      " << endl;
  cout << "  --lp=VEH:VAR    Specify starting left log plot.             " << endl;
  cout << "  --rp=VEH:VAR    Specify starting right log plot.            " << endl;
  cout << "  --quick,-q      Quick start (no geo shapes, logplots)       " << endl;
  cout << "  --web,-w        Open documentation in a web browser.        " << endl;
  cout << "                                                              " << endl;
  cout << "Further Notes:                                                " << endl;
  cout << "  (1) Multiple log files are synchronized on replay.          " << endl;
  cout << "  (2) Configuration mirrors alogview. See docs/uReplay.md.    " << endl;
  cout << "  (3) Without an input file, select a directory of .sqlite    " << endl;
  cout << "      logs when prompted to auto-load them.                   " << endl;
  cout << endl;
}

//--------------------------------------------------------
// Procedure: promptForSqliteDirectory()

string promptForSqliteDirectory()
{
  const char* result = fl_dir_chooser("Select directory with sqlite logs", ".", 0);
  if(!result)
    return("");
  return(std::string(result));
}

//--------------------------------------------------------
// Procedure: collectSqliteFiles()

vector<string> collectSqliteFiles(const string& dirpath)
{
  vector<string> results;
  if(dirpath == "")
    return(results);

  DIR* dir = opendir(dirpath.c_str());
  if(!dir)
    return(results);

  struct dirent* entry = nullptr;
  string base = dirpath;
  if((base.size() > 0) && (base[base.size()-1] != '/'))
    base += "/";

  while((entry = readdir(dir)) != nullptr) {
    string name = entry->d_name;
    if((name == ".") || (name == ".."))
      continue;
    string fullpath = base + name;
    struct stat info;
    if(stat(fullpath.c_str(), &info) != 0)
      continue;
    if(S_ISREG(info.st_mode) && strEnds(name, ".sqlite"))
      results.push_back(fullpath);
  }

  closedir(dir);
  std::sort(results.begin(), results.end());
  return(results);
}

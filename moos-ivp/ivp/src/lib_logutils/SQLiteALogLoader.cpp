/*****************************************************************/
/*    NAME: OpenAI Assistant                                      */
/*    ORGN: OpenAI                                                */
/*    FILE: SQLiteALogLoader.cpp                                 */
/*    DATE: Feb 14th, 2025                                        */
/*****************************************************************/

#include <iostream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "MBUtils.h"
#include "SQLiteALogLoader.h"

using namespace std;

namespace {

bool pathExists(const string& path)
{
  struct stat info;
  return(stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}

bool makeDirs(const string& path)
{
  if(path.empty())
    return(true);

  string sofar;
  vector<string> parts = parseString(path, '/');
  if(parts.empty()) {
    if(pathExists(path))
      return(true);
    return(mkdir(path.c_str(), 0777) == 0 || errno == EEXIST);
  }

  bool ok = true;
  for(unsigned int i=0; i<parts.size(); ++i) {
    if(i>0)
      sofar += "/";
    sofar += parts[i];
    if(sofar.empty())
      continue;
    if(pathExists(sofar))
      continue;
    if(mkdir(sofar.c_str(), 0777) != 0 && errno != EEXIST) {
      ok = false;
      break;
    }
  }
  return(ok);
}

} // end anonymous namespace

//-------------------------------------------------------------
// Constructor

SQLiteALogLoader::SQLiteALogLoader()
{
  m_verbose = false;
}

//-------------------------------------------------------------
// Procedure: ensureDirectory

bool SQLiteALogLoader::ensureDirectory(const string& directory) const
{
  if(directory == "" || directory == ".")
    return(true);
  if(pathExists(directory))
    return(true);
  if(!makeDirs(directory)) {
    if(m_verbose)
      cout << "SQLite cache: Failed to create directory " << directory << endl;
    return(false);
  }
  return(true);
}

//-------------------------------------------------------------
// Procedure: writeFile

bool SQLiteALogLoader::writeFile(const string& root,
                                 const string& rel_path,
                                 const string& data) const
{
  string full_path = root;
  if((!full_path.empty()) && (full_path.back() != '/'))
    full_path += "/";
  full_path += rel_path;

  string directory = full_path;
  rbiteString(directory, '/');
  if(!ensureDirectory(directory))
    return(false);

  FILE *fp = fopen(full_path.c_str(), "w");
  if(!fp) {
    if(m_verbose)
      cout << "SQLite cache: Failed to open " << full_path << " for writing" << endl;
    return(false);
  }
  if(!data.empty())
    fwrite(data.c_str(), 1, data.size(), fp);
  fclose(fp);
  return(true);
}

//-------------------------------------------------------------
// Procedure: exportCache

bool SQLiteALogLoader::exportCache(const string& db_file,
                                   const string& cache_root)
{
  if(m_verbose) {
    cout << "Caching SQLite log " << db_file << "..." << endl;
    cout << "  --> output directory " << cache_root << endl;
  }

  sqlite3* db = 0;
  int status = sqlite3_open(db_file.c_str(), &db);
  if(status != SQLITE_OK) {
    if(m_verbose)
      cout << "SQLite cache: failed to open database: "
           << sqlite3_errmsg(db) << endl;
    if(db)
      sqlite3_close(db);
    return(false);
  }

  // Always ensure the root cache directory exists.
  if(!ensureDirectory(cache_root)) {
    sqlite3_close(db);
    return(false);
  }

  const char* sql = "SELECT path, contents FROM files";
  sqlite3_stmt* stmt = 0;
  status = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
  if(status != SQLITE_OK) {
    if(m_verbose)
      cout << "SQLite cache: failed to prepare query: "
           << sqlite3_errmsg(db) << endl;
    sqlite3_close(db);
    return(false);
  }

  bool ok = true;
  while((status = sqlite3_step(stmt)) == SQLITE_ROW) {
    const unsigned char* path_txt = sqlite3_column_text(stmt, 0);
    if(!path_txt)
      continue;
    string rel_path = (const char*)path_txt;

    const void* blob_ptr = sqlite3_column_blob(stmt, 1);
    int blob_size = sqlite3_column_bytes(stmt, 1);
    string contents;
    if(blob_ptr && (blob_size > 0))
      contents.assign(static_cast<const char*>(blob_ptr), blob_size);

    if(!writeFile(cache_root, rel_path, contents)) {
      ok = false;
      break;
    }
  }

  if(status != SQLITE_DONE && status != SQLITE_ROW && m_verbose)
    cout << "SQLite cache: iteration ended with status " << status << endl;

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return(ok);
}


/*****************************************************************/
/*    NAME: OpenAI Assistant                                      */
/*    ORGN: OpenAI                                                */
/*    FILE: SQLiteALogLoader.h                                   */
/*    DATE: Feb 14th, 2025                                        */
/*****************************************************************/

#ifndef SQLITE_ALOG_LOADER_HEADER
#define SQLITE_ALOG_LOADER_HEADER

#include <string>

class SQLiteALogLoader
{
 public:
  SQLiteALogLoader();
  ~SQLiteALogLoader() {}

  void setVerbose(bool v=true) {m_verbose=v;}

  bool exportCache(const std::string& db_file,
                   const std::string& cache_root);

 private:
  bool ensureDirectory(const std::string& directory) const;
  bool writeFile(const std::string& root,
                 const std::string& rel_path,
                 const std::string& data) const;

 private:
  bool m_verbose;
};

#endif

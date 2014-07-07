//https://code.google.com/p/nya-engine/

#pragma once

#include "log.h"
#include <vector>

namespace nya_log
{

class warnings_counter
{
public:
    // returns index of inserted warning; preserves indices of previously added warnings
    int add_warning(const char *msg);
    void clear() { m_warnings.clear(); }
    int get_unique_warnings_count() { return (int)m_warnings.size(); }
    int get_count_for_warning_with_idx(int idx);
    const char *get_warning_message_with_idx(int idx);

private:
    typedef std::vector<std::pair<std::string,int> > warnings_counts_map;
    warnings_counts_map m_warnings;
};

class warning_ostream: public memory_ostream
{
public:
    warning_ostream(warnings_counter &counter): m_counter(counter) {}
    virtual ~warning_ostream() { flush(); }

    void flush();

private:
    warnings_counter &m_counter;
};

#if defined DEBUG || defined _DEBUG
  std::string get_line_descriptor(const char *filename, int line_number);
  //returns string with line number and file name of macro invocation
  #define nya_line_descriptor (nya_log::get_line_descriptor(__FILE__, __LINE__))
  #define nya_base_warning() nya_log::warning()<<"unexpected condition at "<<nya_line_descriptor<<": "
#else
  inline std::string get_line_descriptor(const char *filename, int line_number) {return "";}
  #define nya_debug_line_descriptor ""
  #define nya_base_warning() nya_log::warning()
#endif

warnings_counter &get_warnings_counter();
warning_ostream warning();

}

//https://code.google.com/p/nya-engine/

#include "warning.h"

namespace nya_log
{

int warnings_counter::add_warning(const char *msg)
{
    if(!msg || m_ignore_warnings)
        return 0;

    warnings_counts_map::iterator iter=m_warnings.begin();
    while(iter!=m_warnings.end() && iter->first!=msg)
        ++iter;

    if (iter==m_warnings.end())
        iter=m_warnings.insert(iter,std::make_pair(std::string(msg),0));

    ++(iter->second);
    return (int)(iter-m_warnings.begin());
}

int warnings_counter::get_count_for_warning_with_idx(int idx)
{
    if(idx<0 || idx>=(int)m_warnings.size())
        return 0;

    return m_warnings[idx].second;
}

const char *warnings_counter::get_warning_message_with_idx(int idx)
{
    if(idx<0 || idx>=(int)m_warnings.size())
        return 0;

    return m_warnings[idx].first.c_str();
}

void warning_ostream::flush()
{
    if(get_text()[0])
        m_counter.add_warning(get_text());
}

#if defined DEBUG || defined _DEBUG
std::string get_line_descriptor(const char *filename, int line_number)
{
    std::ostringstream ss;
    ss<<"line "<<line_number<<" in '"<<filename<<"'";
    return ss.str();
}
#endif

warnings_counter &get_warnings_counter() { static warnings_counter wc; return wc; }
warning_ostream warning() { return warning_ostream( get_warnings_counter() ); }

}

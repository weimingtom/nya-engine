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

    if(iter==m_warnings.end())
        iter=m_warnings.insert(iter,std::make_pair(std::string(msg),0));

    ++(iter->second);
    return (int)(iter-m_warnings.begin());
}

unsigned int warnings_counter::get_warnings_count(int idx)
{
    if(idx<0 || idx>=(int)m_warnings.size())
        return 0;

    return m_warnings[idx].second;
}

const char *warnings_counter::get_warning_message(int idx)
{
    if(idx<0 || idx>=(int)m_warnings.size())
        return 0;

    return m_warnings[idx].first.c_str();
}

unsigned int warnings_counter::get_total_warnings_count()
{
    unsigned int count=0;
    for(size_t i=0;i<m_warnings.size();++i)
        count+=m_warnings[i].second;
    return count;
}

void warning_ostream::flush()
{
    if(get_text()[0])
        m_counter.add_warning(get_text());
}

warnings_counter &get_warnings_counter() { static warnings_counter wc; return wc; }
warning_ostream warning() { return warning_ostream( get_warnings_counter() ); }

}

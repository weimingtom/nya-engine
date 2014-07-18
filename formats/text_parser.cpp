//https://code.google.com/p/nya-engine/

#include "text_parser.h"
#include "log/warning.h"
#include "memory/invalid_object.h"
#include "resources/resources.h"
#include "memory/tmp_buffer.h"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace
{
    const char global_marker='@';
    const char *whitespaces=" \t\r\n";
    const char *special_chars="=:";
}

namespace nya_formats
{

const char *text_parser::get_section_type(int idx) const
{
    if(idx<0 || idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[idx].type.c_str();
}

int text_parser::get_section_names_count(int idx) const
{
    if(idx < 0 || idx >= (int)m_sections.size())
    {
        nya_base_warning();
        return 0;
    }

    return (int)m_sections[idx].names.size();
}

const char *text_parser::get_section_name(int idx,int name_idx) const
{
    if(idx<0 || idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return 0;
    }

    if(name_idx<0 || name_idx>=(int)m_sections[idx].names.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[idx].names[name_idx].c_str();
}

const char *text_parser::get_section_option(int idx) const
{
    if(idx<0 || idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[idx].option.c_str();
}

const char *text_parser::get_section_value(int idx) const
{
    if(idx<0 || idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[idx].value.c_str();
}

nya_math::vec4 text_parser::get_section_value_vector(int idx) const
{
    if(idx<0 || idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return nya_memory::get_invalid_object<nya_math::vec4>();
    }

    nya_math::vec4 v;
    std::string s=m_sections[idx].value;
    std::replace(s.begin(),s.end(),',',' ');
    std::istringstream iss(s);
    if(iss>>v.x)
        if(iss>>v.y)
            if(iss>>v.z)
                iss>>v.w;
    return v;
}

int text_parser::get_subsections_count(int section_idx) const
{
    if(section_idx<0 || section_idx>=(int)m_sections.size())
    {
        nya_base_warning();
        return -1;
    }

    const section &s=m_sections[section_idx];
    if(!s.subsection_parsed)
    {
        // parse subsection
        line l=line::first(s.value.c_str(),s.value.size());
        while(l.next())
        {
            std::list<std::string> tokens=tokenize_line(l);
            if(tokens.size()>0)
            {
                s.subsections.push_back(subsection());
                subsection &ss=s.subsections.back();
                std::list<std::string>::iterator iter=tokens.begin();
                ss.type = *(iter++);
                if(iter!=tokens.end() && *(iter++) == "=")
                {
                    if(iter!=tokens.end())
                        ss.value=*iter;
                }
            }
        }

        s.subsection_parsed=true;
    }

    return (int)s.subsections.size();
}

const char *text_parser::get_subsection_type(int section_idx,int idx) const
{
    if(section_idx<0 || section_idx>=(int)m_sections.size() ||
       idx<0 || idx>=(int)m_sections[section_idx].subsections.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[section_idx].subsections[idx].type.c_str();
}

const char *text_parser::get_subsection_value(int section_idx,int idx) const
{
    if(section_idx<0 || section_idx>=(int)m_sections.size() ||
       idx<0 || idx>=(int)m_sections[section_idx].subsections.size())
    {
        nya_base_warning();
        return 0;
    }

    return m_sections[section_idx].subsections[idx].value.c_str();
}

bool text_parser::get_subsection_value_bool(int section_idx,int idx) const
{
    if(section_idx<0 || section_idx>=(int)m_sections.size() ||
       idx<0 || idx>=(int)m_sections[section_idx].subsections.size())
    {
        nya_base_warning();
        return 0;
    }

    std::string s=m_sections[section_idx].subsections[idx].value;
    std::transform(s.begin(),s.end(),s.begin(),::tolower);
    return s=="yes" || s=="1" || s=="true";
}

text_parser::line text_parser::line::first(const char *text,size_t text_size)
{
    line l;
    l.text=text;
    l.text_size=text_size;
    l.offset=l.size=0;
    l.global=l.empty=false;
    l.line_number=l.next_line_number=0;
    return l;
}

// line knows about quotes, '\n' characters inside quotes are not treated as new line
bool text_parser::line::next()
{
    if(offset+size>=text_size)
        return false;

    offset+=size;
    line_number=next_line_number;

    // calculate line size, keep in mind multiline quoted tokens
    size_t char_idx=offset;
    bool in_quotes=false;
    while(char_idx!=text_size)
    {
        char c=text[char_idx++];
        if(c=='\n')
        {
            ++next_line_number;
            if(!in_quotes)
                break;
        }
        else if(c=='"')
            in_quotes=!in_quotes;
    }

    size=char_idx-offset;

    const size_t first_non_whitespace_idx=skip_whitespaces(text,text_size,offset);
    global=(first_non_whitespace_idx<offset+size && text[first_non_whitespace_idx]==global_marker);
    empty=(first_non_whitespace_idx>=offset+size);

    return true;
}

bool text_parser::load_from_data(const char *text,size_t text_size)
{
    if(!text)
        return false;

    text_size=get_real_text_size(text,text_size);
    if(!text_size)
        return false;

    size_t global_count=0;
    line l=line::first(text,text_size);
    while(l.next()) if(l.global) ++global_count;
    m_sections.resize(global_count);

    size_t subsection_start_idx=0,subsection_end_idx=0,sections_count=0;
    bool subsection_empty=true;
    l=line::first(text,text_size);

    while(l.next())
    {
        if(l.global)
        {
            if(subsection_end_idx>subsection_start_idx && !subsection_empty)
            {
                m_sections[sections_count-1].value=std::string(text+subsection_start_idx,subsection_end_idx-subsection_start_idx);
                subsection_empty=true;
            }
            fill_section(m_sections[sections_count],l);
            subsection_start_idx=l.offset+l.size;
            ++sections_count;
        }
        else
        {
            if(sections_count>0)
            {
                subsection_end_idx=l.offset+l.size;
                if(!l.empty)
                    subsection_empty=false;
            }
            else if(!l.empty)
            {
                nya_log::log()<<typeid(text_parser).name()<<": subsection found before any section declaration at line "<< l.line_number;
            }
        }
    }

    if(subsection_end_idx>subsection_start_idx && !subsection_empty)
        m_sections[sections_count-1].value=std::string(text+subsection_start_idx,subsection_end_idx-subsection_start_idx);

    return true;
}

bool text_parser::load_from_file(const char *filename)
{
    nya_resources::resource_data* res=nya_resources::get_resources_provider().access(filename);
    if(!res)
    {
        nya_log::log()<<"file load error: unable to access resource '"<<filename<<"'";
        return false;
    }

    const size_t data_size=res->get_size();
    nya_memory::tmp_buffer_scoped buf(data_size);
    res->read_all(buf.get_data());
    res->release();
    return load_from_data((const char*)buf.get_data(),data_size);
}

void text_parser::fill_section(section &s,const line &l)
{
   std::list<std::string> tokens=tokenize_line(l);
    // assert(tokens.size() > 0);
    // assert(tokens.front().size > 0);
    // assert(tokens.front().at(0) == global_marker);
    std::list<std::string>::iterator iter=tokens.begin();
    s.type.swap(*(iter++));
    bool need_option=false;
    bool need_value=false;
    bool need_name=true;
    while(iter!=tokens.end())
    {
        if(need_option)
        {
            s.option.swap(*iter);
            need_option=false;
        }
        else if(need_value)
        {
            s.value.swap(*iter);
            need_value=false;
        }
        else if(*iter==":")
        {
            need_option=true;
            need_name=false;
        }
        else if(*iter=="=")
        {
            need_value=true;
            need_name=false;
        }
        else if(need_name)
        {
            if(s.names.empty() || !s.names.back().empty())
                s.names.push_back(std::string());

            s.names.back().swap(*iter);
        }
        else
        {
            nya_log::log()<<typeid(text_parser).name()<<": unexpected token at line "<<l.line_number;
            break;
        }

        ++iter;
    }
}

std::list<std::string> text_parser::tokenize_line(const line &l)
{
    std::list<std::string> result;
    const size_t line_end=l.offset+l.size;
    size_t char_idx=l.offset;

    while(true)
    {
        size_t token_start_idx, token_size;
        char_idx=get_next_token(l.text,line_end,char_idx,token_start_idx,token_size);
        if(token_start_idx<line_end)
            result.push_back(std::string(l.text+token_start_idx,token_size));
        else
            break;
    }

    return result;
}

size_t text_parser::get_real_text_size(const char *text,size_t supposed_size)
{
    const char *t=text;
    if(supposed_size!=no_size)
        while(*t && t<text+supposed_size) ++t;
    else
        while(*t) ++t;

    return t-text;
}

size_t text_parser::get_next_token(const char *text,size_t text_size,size_t pos,size_t &token_start_idx_out,size_t &token_size_out)
{
    size_t char_idx=pos;
    char_idx=skip_whitespaces(text,text_size,char_idx);
    if(char_idx<text_size && strchr(special_chars,text[char_idx]))
    {
        token_start_idx_out=char_idx;
        token_size_out=1;
        return char_idx+1;
    }

    if(char_idx>=text_size)
    {
        token_start_idx_out=text_size;
        token_size_out=0;
        return text_size;
    }

    token_start_idx_out=char_idx;
    bool quoted_token=false;
    if(text[char_idx]=='"')
    {
        ++char_idx;
        token_start_idx_out=char_idx;
        quoted_token=true;
    }

    size_t token_end_idx=token_start_idx_out;
    bool end_found=false;
    while(char_idx<text_size && !end_found)
    {
        char c=text[char_idx];
        if(quoted_token)
        {
            if(c=='"')
            {
                token_end_idx=char_idx;
                end_found=true;
            }

            ++char_idx;
        }
        else
        {
            if(strchr(whitespaces,c) || strchr(special_chars, c))
            {
                token_end_idx=char_idx;
                end_found=true;
            }
            else
                ++char_idx;
        }
    }

    token_size_out=(end_found?token_end_idx:text_size)-token_start_idx_out;

    return char_idx;
}

size_t text_parser::skip_whitespaces(const char *text,size_t text_size,size_t pos)
{
    while(pos<text_size && strchr(whitespaces,text[pos]))
        ++pos;

    return pos;
}

void text_parser::debug_print(nya_log::ostream_base &os) const
{
    for(size_t i=0;i<m_sections.size();++i)
    {
        const section &s=m_sections[i];
        os<<"section "<<i<<" '"<<s.type<<"':\n";
        for(size_t j=0;j<s.names.size();++j)
            os<<"  name "<<j<<" '"<<s.names[j]<<"'\n";

        if(!s.option.empty())
            os<<"  option '"<<s.option<<"'\n";
        os<<"  value '"<<s.value<<"'\n\n\n";
    }
}

}

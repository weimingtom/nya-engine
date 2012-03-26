//https://code.google.com/p/nya-engine/

#include "html_log.h"
#include <stdio.h>

namespace nya_log
{

const char *html_log_header = 
	"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n"
	"\"http://www.w3.org/TR/html4/strict.dtd\">\n"
	"<html>\n"
	"<head>\n"
	"<title>nya-engine log</title>\n"
	"<style>\n"
	"\t.block{border:1px solid gray;padding: 1px 30px;}\n"
	"\t.hidden { display:none;}\n"
	"</style>\n"
	"<script type=\"text/javascript\">\n"
	"\tfunction show_hide(block_id)\n"
	"\t{\n"
	"\t\tif (document.getElementById(block_id).className == \"block\")\n"
	"\t\t\tdocument.getElementById(block_id).className = \"hidden\";\n"
	"\telse\n"
	"\t\tdocument.getElementById(block_id).className = \"block\";\n"
	"\t}\n"
	"</script>\n"
	"</head>\n"
	"<body>\n"
	"<pre><font color=#66666>";

const char *html_log_tail = 
	"</font></pre>\n"
	"</body>\n"
	"</html>\n";

const char *html_log_block = 
	"<div onclick=\"show_hide(&#39;block%i&#39;)\">+</div>\n"
	"<div class=\"hidden\" id=\"block%i\">\n";

const char *html_log_block_end =
	"<div align=\"right\"><i>%i ms</i></div></div>\n";

const char *html_log_colors[] = 
	{
		"<font color=#000000>",
		"<font color=#6666ff>",
		"<font color=\"green\">",
		"<font color=\"magenta\">",		
		"<font color=\"red\">",
		"<font color=\"red\">"
	};

const char *html_log_color_end =
	"</font>";

log & html_log::operator << (message_type a)
{
	m_message_type = a;
    return *this;
}

log & html_log::operator << (int a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        fprintf(f,"%s%i%s",html_log_colors[m_message_type],a,html_log_color_end);
        fclose(f);
    }
    return *this;
}

log & html_log::operator << (float a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        fprintf(f,"%s%f%s",html_log_colors[m_message_type],a,html_log_color_end);
        fclose(f);
    }
    return *this;
}

log & html_log::operator << (const char * a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        fprintf(f,"%s%s%s",html_log_colors[m_message_type],a,html_log_color_end);
        fclose(f);
    }
    return *this;
}

bool html_log::open(const char*file_name)
{
    if(FILE*f = fopen(file_name,"w+"))
    {
		fprintf(f,"%s",html_log_header);
        fclose(f);
        m_file_name.assign(file_name);
        return true;
    }

    m_file_name.clear();
	return false;
}

void html_log::close()
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
		fprintf(f,"%s",html_log_tail);
        fclose(f);
    }

	m_file_name.clear();
}

void html_log::scope_inc()
{
	++m_scope;

    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
		fprintf(f,html_log_block,m_block,m_block);
        fclose(f);

        ++m_block;
    }
}

void html_log::scope_dec()
{
	--m_scope;
	if(m_scope>=0)
	{
		if(FILE*f = fopen(m_file_name.c_str(),"a+"))
		{
			fprintf(f,html_log_block_end,0);
		    fclose(f);
		}
	}
	else
		m_scope=0;
}

}


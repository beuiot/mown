#ifndef QDEBUGSTREAM_H
#define QDEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>
#include <QStatusBar>
#include <QDateTime>

#include "qtextedit.h"

class QDebugStream : public std::basic_streambuf<char>
{
public:
	QDebugStream(std::ostream &stream, QTextEdit* text_edit, bool isError = false, QStatusBar *statusBar = NULL) : m_stream(stream)
	{
		log_window = text_edit;
		m_old_buf = stream.rdbuf();
		m_IsError = isError;
		m_StatusBar = statusBar;
		stream.rdbuf(this);
	}
	~QDebugStream()
	{
		// output anything that is left
		if (!m_string.empty())
			log_window->append(m_string.c_str());

		m_stream.rdbuf(m_old_buf);
	}

protected:
	virtual int_type overflow(int_type v)
	{
		if (v == '\n')
		{
			std::string result;
			if (m_IsError)
				result = "<font color=\"red\">[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] " + m_string + "</font>";
			else
				result = "<font color=\"blue\">[" + QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() + "] " + m_string + "</font>";

			log_window->append(result.c_str());
			if (m_StatusBar != NULL)
			{
				if (m_IsError)
					m_StatusBar->setStyleSheet("color: red;");
				else
					m_StatusBar->setStyleSheet("");

				m_StatusBar->showMessage(m_string.c_str());
			}
			m_string.erase(m_string.begin(), m_string.end());
		}
		else
			m_string += v;

		return v;
	}

	virtual std::streamsize xsputn(const char *p, std::streamsize n)
	{
		m_string.append(p, p + n);

		size_t pos = 0;
		while (pos != std::string::npos)
		{
			pos = m_string.find('\n');
			if (pos != std::string::npos)
			{
				std::string tmp(m_string.begin(), m_string.begin() + pos);
				log_window->append(tmp.c_str());
				m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
			}
		}

		return n;
	}

private:
	std::ostream &m_stream;
	std::streambuf *m_old_buf;
	std::string m_string;
	bool m_IsError;
	QStatusBar *m_StatusBar;


	QTextEdit* log_window;
};

#endif //QDEBUGSTREAM_H

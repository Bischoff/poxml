#include "parser.h"
#include <stdlib.h>
#include <iostream>

#include <fstream>
#include "GettextLexer.hpp"
#include "GettextParser.hpp"
#include "antlr/AST.hpp"
#include "antlr/CommonAST.hpp"

using namespace std;

QString translate(QString xml, QString orig, QString translation)
{
    orig.replace(QRegExp("\\\\\""), "\"");
    orig.replace(QRegExp("\\\\\n"), "\n");
    orig.replace(QRegExp("\\\\\\"), "\\");
    int index = xml.find(orig);
    if (index == -1) {
        qDebug("can't find\n%s\nin\n%s", orig.latin1(), xml.latin1());
        ASSERT(false);
    }
    xml.replace(index, orig.length(), translation);
    return xml;
}

int main( int argc, char **argv )
{
    if (argc != 3) {
        qWarning("usage: %s english-XML translated-PO", argv[0]);
        ::exit(1);
    }

    MsgList english = parseXML(argv[1]);
    MsgList translated;

    try {
        ifstream s(argv[2]);
        GettextLexer lexer(s);
        GettextParser parser(lexer);
        translated = parser.file();

    } catch(exception& e) {
        cerr << "exception: " << e.what() << endl;
        ::exit(1);
    }

    QMap<QString, QString> translations;
    for (MsgList::ConstIterator it = translated.begin();
         it != translated.end(); ++it)
    {
        translations.insert((*it).msgid, (*it).msgstr);
    }

    QFile xml(argv[1]);
    xml.open(IO_ReadOnly);
    QTextStream ds(&xml);
    QString xml_text = ds.read();
    QString output;
    QTextStream ts(&output, IO_WriteOnly);

    QValueList<int> line_offsets;
    line_offsets.append(0);
    int index = 0;
    while (true) {
        index = xml_text.find('\n', index) + 1;
        if (index <= 0)
            break;
        line_offsets.append(index);
    }

    int old_start_line = -1, old_start_col = -1;
    QString old_text;
    MsgList::Iterator old_it = english.end();

    for (MsgList::Iterator it = english.begin();
         it != english.end(); ++it)
    {
        BlockInfo bi = (*it).lines.first();
        int start_pos = line_offsets[bi.start_line - 1] + bi.start_col;
        int end_pos = line_offsets[bi.end_line - 1] + bi.end_col - 1;

        (*it).start = start_pos;
        if (old_start_line == bi.start_line &&
            old_start_col == bi.start_col)
        {
            (*old_it).end = bi.offset;
            (*it).end = end_pos;
        } else {
            (*it).lines.first().offset = 0;
            (*it).end = 0;
        }

        old_start_line = bi.start_line;
        old_start_col = bi.start_col;
        old_it = it;
    }

    int old_pos = 0;

    for (MsgList::Iterator it = english.begin();
         it != english.end(); ++it)
    {
        BlockInfo bi = (*it).lines.first();
        int start_pos = line_offsets[bi.start_line - 1] + bi.start_col;
        int end_pos = line_offsets[bi.end_line - 1] + bi.end_col - 1;
        while (xml_text.at(end_pos) != '<')
            end_pos--;

        QString xml = xml_text.mid(start_pos, end_pos - start_pos);
        xml = xml.simplifyWhiteSpace();

        if ((*it).end) {
            if (!(*it).lines.first().offset && start_pos != old_pos) {
                ts << xml_text.mid(old_pos, start_pos - old_pos);
            }
            ts << translate(xml.mid(bi.offset, (*it).end - bi.offset),
                                    (*it).msgid, translations[(*it).msgid]);
            old_pos = end_pos;
        } else {
            if (start_pos != old_pos) {
                ts << xml_text.mid(old_pos, start_pos - old_pos);
            }
            old_pos = end_pos;
            ts << translate(xml,
                            (*it).msgid, translations[(*it).msgid]);
        }

    }

    ts << xml_text.mid(old_pos);

    output.replace(QRegExp("\\\\\""), "\"");
    output.replace(QRegExp("\\\\\n"), "\n");

    cout << output.latin1();
    return 0;
}

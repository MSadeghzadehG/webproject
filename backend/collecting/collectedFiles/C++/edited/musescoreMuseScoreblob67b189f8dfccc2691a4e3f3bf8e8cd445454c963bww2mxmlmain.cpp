

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QStringList>

#include "lexer.h"
#include "parser.h"
#include "mxmlwriter.h"

void usage()
{
  }

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QStringList args = QCoreApplication::arguments();

  QFile inFile;
  QFile outFile;

  if (args.size() > 3)
  {
    usage();
    return 1;
  }

  if (args.size() <= 1)
    inFile.open(stdin, QFile::ReadOnly);
  else
  {
    inFile.setFileName(args.at(1));
    if (!inFile.open(QIODevice::ReadOnly))
    {
      return 2;
    }
  }

  if (args.size() < 3)
    outFile.open(stdout, QFile::WriteOnly);
  else
  {
    outFile.setFileName(args.at(2));
    if (!outFile.open(QIODevice::WriteOnly))
    {
      return 2;
    }
  }

  Bww::Lexer lex(&inFile);
  Bww::MxmlWriter wrt;
  wrt.setOutDevice(&outFile);
  Bww::Parser p(lex, wrt);
  p.parse();

  return 0;
}

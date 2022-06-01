#include <QtGui/QWidget>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++)
  {
    qDebug("argv[%d] = %s", i, argv[i]);
  }
  KAboutData aboutData( "testqtargs", 0, ki18n("testqtargs"),
    "1.0", ki18n("testqtargs"), KAboutData::License_GPL);

  KCmdLineOptions options;
  options.add("hello ", ki18n("Says hello"));

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);

  KCmdLineArgs *qtargs = KCmdLineArgs::parsedArgs("qt");
  for (int i = 0; i < qtargs->count(); i++)
  {
    qDebug("qt arg[%d] = %s", i, qtargs->arg(i).toLocal8Bit().data());
  }

  KApplication app;

  KCmdLineArgs *kdeargs = KCmdLineArgs::parsedArgs("kde");
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  // An arg set by Katie
  if(qtargs->isSet("stylesheet"))
  {
    qDebug("arg stylesheet = %s", qtargs->getOption("stylesheet").toLocal8Bit().data());
  }
  // An arg set by KDE
  if(kdeargs->isSet("caption"))
  {
    qDebug("arg caption = %s", kdeargs->getOption("caption").toLocal8Bit().data());
  }
  // An arg set by us.
  if(args->isSet("hello"))
  {
    qDebug("arg hello = %s", args->getOption("hello").toLocal8Bit().data());
  }
  args->clear();

  QWidget *w = new QWidget();
  w->show();

  return app.exec();
}


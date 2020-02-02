#include <qtest_kde.h>
#include <kcharsets.h>

class KCharsetsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void resolveEntities();
};

QTEST_KDEMAIN_CORE( KCharsetsTest )

void KCharsetsTest::resolveEntities()
{
    QString input( "&lt;Hello &amp;World&gt;" );
    QString output = KCharsets::resolveEntities( input );
    QCOMPARE(output, QLatin1String("<Hello &World>"));
}

#include "kcharsetstest.moc"

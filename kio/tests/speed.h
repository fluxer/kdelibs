// -*- c++ -*-
#ifndef SPEED_H
#define SPEED_H

#include <kio/global.h>
#include <kurl.h>
#include <kio/udsentry.h>

class KJob;
namespace KIO {
    class Job;
}

class SpeedTest : public QObject {
    Q_OBJECT

public:
    SpeedTest(const KUrl & url);

private Q_SLOTS:
    void entries( KIO::Job *, const KIO::UDSEntryList& );
    void finished( KJob *job );

};

#endif

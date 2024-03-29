/* This file is part of the KDE libraries
   Copyright (C) 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kservicetypetrader.h"

#include "ktraderparsetree_p.h"
#include <kdebug.h>
#include "kservicetype.h"
#include "kservicetypefactory.h"
#include "kservicefactory.h"

// --------------------------------------------------

KServiceTypeTrader* KServiceTypeTrader::self()
{
    K_GLOBAL_STATIC(KServiceTypeTrader, s_globalServiceTypeTrader)
    return s_globalServiceTypeTrader;
}

KServiceTypeTrader::KServiceTypeTrader()
{
}

// shared with KMimeTypeTrader
void KServiceTypeTrader::applyConstraints( KService::List& lst,
                                const QString& constraint )
{
    if ( lst.isEmpty() || constraint.isEmpty() )
        return;

    const KTraderParse::ParseTreeBase::Ptr constr = KTraderParse::parseConstraints( constraint ); // for ownership
    const KTraderParse::ParseTreeBase* pConstraintTree = constr.data(); // for speed

    if (!constr) { // parse error
        lst.clear();
    } else {
        // Find all services matching the constraint
        // and remove the other ones
        KService::List::iterator it = lst.begin();
        while( it != lst.end() )
        {
            if ( matchConstraint( pConstraintTree, (*it), lst ) != 1 )
                it = lst.erase( it );
            else
                ++it;
        }
    }
}

#if 0
static void dumpOfferList( const KServiceOfferList& offers )
{
    kDebug(7014) << "Sorted list:";
    KServiceOfferList::ConstIterator itOff = offers.constBegin();
    for( ; itOff != offers.constEnd(); ++itOff )
        kDebug(7014) << (*itOff).service()->name() << " allow-as-default=" << (*itOff).allowAsDefault() << " preference=" << (*itOff).preference();
}
#endif

static KServiceOfferList weightedOffers( const QString& serviceType )
{
    //kDebug(7014) << "KServiceTypeTrader::weightedOffers( " << serviceType << " )";

    KServiceType::Ptr servTypePtr = KServiceTypeFactory::self()->findServiceTypeByName( serviceType );
    if ( !servTypePtr ) {
        kWarning(7014) << "KServiceTypeTrader: serviceType " << serviceType << " not found";
        return KServiceOfferList();
    }
    if ( servTypePtr->serviceOffersOffset() == -1 )  // no offers in ksycoca
        return KServiceOfferList();

    // First, get all offers known to ksycoca.
    KServiceOfferList offers = KServiceFactory::self()->offers( servTypePtr->offset(), servTypePtr->serviceOffersOffset() );

    qStableSort( offers );
    //kDebug(7014) << "Found offers: " << offers.count() << " offers";

#if 0
    dumpOfferList( offers );
#endif

    return offers;
}

KService::List KServiceTypeTrader::query( const QString& serviceType,
                                          const QString& constraint ) const
{
    KServiceType::Ptr servTypePtr = KServiceTypeFactory::self()->findServiceTypeByName( serviceType );
    if ( !servTypePtr ) {
        kWarning(7014) << "KServiceTypeTrader: serviceType " << serviceType << " not found";
        return KService::List();
    }
    if ( servTypePtr->serviceOffersOffset() == -1 )
        return KService::List();

    KService::List lst =
        KServiceFactory::self()->serviceOffers( servTypePtr->offset(), servTypePtr->serviceOffersOffset() );

    applyConstraints( lst, constraint );

    //kDebug(7014) << "query for serviceType " << serviceType << constraint
    //             << " : returning " << lst.count() << " offers";
    return lst;
}

KService::Ptr KServiceTypeTrader::preferredService( const QString & serviceType ) const
{
    const KServiceOfferList offers = weightedOffers( serviceType );

    KServiceOfferList::const_iterator itOff = offers.begin();
    // Look for the first one that is allowed as default.
    // Since the allowed-as-default are first anyway, we only have
    // to look at the first one to know.
    if( itOff != offers.end() && (*itOff).allowAsDefault() )
        return (*itOff).service();

    //kDebug(7014) << "No offers, or none allowed as default";
    return KService::Ptr();
}

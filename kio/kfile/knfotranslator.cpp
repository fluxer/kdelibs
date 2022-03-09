/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "knfotranslator_p.h"
#include <klocale.h>
#include <kurl.h>

#include <map>

QString KNfoTranslator::translation(const KUrl& uri)
{
    typedef std::map<QString,QString> TranslationMap;

    static const TranslationMap s_translations = {
        { "kfileitem#modified", i18nc("@label", "Modified") },
        { "kfileitem#owner", i18nc("@label", "Owner") },
        { "kfileitem#permissions", i18nc("@label", "Permissions") },
        { "kfileitem#size", i18nc("@label", "Size") },
        { "kfileitem#totalSize", i18nc("@label", "Total Size") },
        { "kfileitem#type", i18nc("@label", "Type") },
        { "kfileitem#mimetype", i18nc("@label", "MIME Type") },
        // implemented so far
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#fileName", i18nc("@label", "Filename") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#comment", i18nc("@label", "Comment") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#title", i18nc("@label music title", "Title") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#pageCount", i18nc("@label", "Page Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#creator", i18nc("@label", "Creator") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nmo#Email", i18nc("@label", "Creator E-Mail") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#publisher", i18nc("@label", "Publisher") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#url", i18nc("@label file URL", "URL") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashValue", i18nc("@label", "Hash Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hashAlgorithm", i18nc("@label", "Hash Algorithm") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLatitudeRef", i18nc("@label", "GPS Latitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#gpsLongitudeRef", i18nc("@label", "GPS Longitude Reference") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#country", i18nc("@label", "Country") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#description", i18nc("@label", "Description") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#copyright", i18nc("@label", "Copyright") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#keyword", i18nc("@label", "Keyword") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#summary", i18nc("@label", "Summary") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#subject", i18nc("@label", "Subject") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#generator", i18nc("@label Software used to generate content", "Generator") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentCreated", i18nc("@label creation date", "Created") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentLastModified", i18nc("@label modification date", "Modified") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#model", i18nc("@label EXIF", "Model") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureTime", i18nc("@label EXIF", "Exposure Time") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureBiasValue", i18nc("@label EXIF", "Exposure Bias Value") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#apertureValue", i18nc("@label EXIF aperture value", "Aperture") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#flash", i18nc("@label EXIF", "Flash") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLength", i18nc("@label EXIF", "Focal Length") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#focalLengthIn35mmFilm", i18nc("@label EXIF", "Focal Length 35 mm") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#isoSpeedRatings", i18nc("@label EXIF", "ISO Speed Ratings") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#exposureMode", i18nc("@label EXIF", "Exposure Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#meteringMode", i18nc("@label EXIF", "Metering Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#whiteBalance", i18nc("@label EXIF", "White Balance") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#orientation", i18nc("@label EXIF", "Orientation") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#resolution", i18nc("@label EXIF", "Resolution") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#characterSet", i18nc("@label", "Character Set") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#lineCount", i18nc("@label number of lines", "Lines") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#wordCount", i18nc("@label number of words", "Words") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#characterCount", i18nc("@label", "Character Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/04/02/ncal#sequence", i18nc("@label", "Revision") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#duration", i18nc("@label", "Duration") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#musicAlbum", i18nc("@label music album", "Album") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#artist", i18nc("@label music or image artist", "Artist") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#genre",  i18nc("@label music genre", "Genre") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#trackNumber", i18nc("@label music track number", "Track") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#performer", i18nc("@label", "Performer") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#Contact", i18nc("@label", "Contact") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#disclaimer", i18nc("@label", "Disclaimer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#textWriter", i18nc("@label", "Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nco#contributor", i18nc("@label", "Contributor") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#director", i18nc("@label video director", "Director") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#conductor", i18nc("@label", "Conductor") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#interpretedBy", i18nc("@label", "Interpreted By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#composer", i18nc("@label music composer", "Composer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#beatsPerMinute", i18nc("@label", "Beats Per Minute") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#encodedBy", i18nc("@label", "Encoded By") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalArtist", i18nc("@label", "Original Artist") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalTextWriter", i18nc("@label", "Original Text Writer") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#originalReleaseYear", i18nc("@label", "Original Release Year") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#SynchronizedText", i18nc("@label", "Lyrics") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#licensee", i18nc("@label", "Licensee") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#InvolvedPerson", i18nc("@label", "Musician Credits") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nid3#subtitle", i18nc("@label", "Subtitle") },
        // here for reference
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#contentSize", i18nc("@label file content size", "Size") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#depends", i18nc("@label file depends from", "Depends") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasPart", i18nc("@label see http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasPart", "Has Part") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasLogicalPart", i18nc("@label see http://www.semanticdesktop.org/ontologies/2007/01/19/nie#hasLogicalPart", "Has Logical Part") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#isPartOf", i18nc("@label parent directory", "Part of") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#lastModified", i18nc("@label modified date of file", "Modified") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#plainTextContent", i18nc("@label", "Content") },
        { "http://www.semanticdesktop.org/ontologies/2007/01/19/nie#relatedTo", i18nc("@label", "Related To") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#averageBitrate", i18nc("@label", "Average Bitrate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#frameRate", i18nc("@label", "Frame Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#channels", i18nc("@label", "Channels") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#codec",  i18nc("@label", "Codec") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#colorDepth", i18nc("@label", "Color Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#colorCount", i18nc("@label", "Color Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#hasHash", i18nc("@label", "Hash") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#height", i18nc("@label", "Height") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#bitDepth", i18nc("@label", "Bit Depth") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#interlaceMode", i18nc("@label", "Interlace Mode") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#programmingLanguage", i18nc("@label", "Programming Language") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleRate", i18nc("@label", "Sample Rate") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#width", i18nc("@label", "Width") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#FileHash", i18nc("@label", "File Hash") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#bitsPerSample", i18nc("@label", "Bits Per Sample") },
        { "http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#sampleCount", i18nc("@label", "Sample Count") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#make", i18nc("@label EXIF", "Make") },
        { "http://www.semanticdesktop.org/ontologies/2007/05/10/nexif#bitsPerSample", i18nc("@label EXIF", "Bits Per Sample") },
        { "http://www.semanticdesktop.org/ontologies/2009/02/19/nmm#releaseDate", i18nc("@label", "Release Date") },
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#created", i18nc("@label resource created time", "Resource Created")},
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#hasSubResource", i18nc("@label", "Sub Resource")},
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#lastModified", i18nc("@label resource last modified", "Resource Modified")},
        { "http://www.semanticdesktop.org/ontologies/2007/08/15/nao#numericRating", i18nc("@label", "Numeric Rating")},
        { "http://www.semanticdesktop.org/ontologies/2010/04/30/ndo#copiedFrom", i18nc("@label", "Copied From")},
        { "http://www.semanticdesktop.org/ontologies/2010/01/25/nuao#firstUsage", i18nc("@label", "First Usage")},
        { "http://www.semanticdesktop.org/ontologies/2010/01/25/nuao#lastUsage", i18nc("@label", "Last Usage")},
        { "http://www.semanticdesktop.org/ontologies/2010/01/25/nuao#usageCount", i18nc("@label", "Usage Count")},
        { "http://nepomuk.kde.org/ontologies/2010/11/29/kext#unixFileGroup", i18nc("@label", "Unix File Group")},
        { "http://nepomuk.kde.org/ontologies/2010/11/29/kext#unixFileMode", i18nc("@label", "Unix File Mode")},
        { "http://nepomuk.kde.org/ontologies/2010/11/29/kext#unixFileOwner", i18nc("@label", "Unix File Owner")},
        { "http://www.w3.org/1999/02/22-rdf-syntax-ns#type", i18nc("@label file type", "Type") },
    };

    const QString key = uri.prettyUrl();
    const TranslationMap::const_iterator it = s_translations.find(key);
    if (it != s_translations.cend()) {
        return it->second;
    }

    // fallback if the URI is not translated
    const int index = key.indexOf(QChar('#'));
    if (index >= 0) {
        return key.right(key.size() - index - 1);
    }
    return key;
}

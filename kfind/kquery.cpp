#include <stdlib.h>

#include <qfileinfo.h>
#include <kdebug.h>
#include <kfileitem.h>
#include <kfilemetainfo.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kzip.h>

#include "kquery.h"

KQuery::KQuery(QObject *parent, const char *name)
    : QObject(parent, name)
    , m_sizemode(0)
    , m_sizeboundary1(0)
    , m_sizeboundary2(0)
    , m_timeFrom(0)
    , m_timeTo(0)
    , job(0)
    , m_insideCheckEntries(false)
    , m_result(0)
{
    m_regexps.setAutoDelete(true);
    m_fileItems.setAutoDelete(true);
    processLocate = new KProcess(this);
    connect(processLocate, SIGNAL(receivedStdout(KProcess *, char *, int)), this, SLOT(slotreceivedSdtout(KProcess *, char *, int)));
    connect(processLocate, SIGNAL(receivedStderr(KProcess *, char *, int)), this, SLOT(slotreceivedSdterr(KProcess *, char *, int)));
    connect(processLocate, SIGNAL(processExited(KProcess *)), this, SLOT(slotendProcessLocate(KProcess *)));

    // Files with these mime types can be ignored, even if
    // findFormatByFileContent() in some cases may claim that
    // these are text files:
    ignore_mimetypes.append("application/pdf");
    ignore_mimetypes.append("application/postscript");

    // PLEASE update the documentation when you add another
    // file type here:
    ooo_mimetypes.append("application/vnd.sun.xml.writer");
    ooo_mimetypes.append("application/vnd.sun.xml.calc");
    ooo_mimetypes.append("application/vnd.sun.xml.impress");
    // OASIS mimetypes, used by OOo-2.x and KOffice >= 1.4
    // ooo_mimetypes.append("application/vnd.oasis.opendocument.chart");
    // ooo_mimetypes.append("application/vnd.oasis.opendocument.graphics");
    // ooo_mimetypes.append("application/vnd.oasis.opendocument.graphics-template");
    // ooo_mimetypes.append("application/vnd.oasis.opendocument.formula");
    // ooo_mimetypes.append("application/vnd.oasis.opendocument.image");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.presentation-template");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.presentation");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.spreadsheet-template");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.spreadsheet");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.text-template");
    ooo_mimetypes.append("application/vnd.oasis.opendocument.text");
    // KOffice-1.3 mimetypes
    koffice_mimetypes.append("application/x-kword");
    koffice_mimetypes.append("application/x-kspread");
    koffice_mimetypes.append("application/x-kpresenter");
}

KQuery::~KQuery()
{
}

void KQuery::kill()
{
    if(job)
        job->kill(false);
    if(processLocate->isRunning())
        processLocate->kill();
    m_fileItems.clear();
}

void KQuery::start()
{
    m_fileItems.clear();
    if(m_useLocate) // use "locate" instead of the internal search method
    {
        m_url.cleanPath();
        processLocate->clearArguments();
        *processLocate << "locate";
        *processLocate << m_url.path(1).latin1();
        bufferLocate = NULL;
        bufferLocateLength = 0;
        processLocate->start(KProcess::NotifyOnExit, KProcess::AllOutput);
        return;
    }

    if(m_recursive)
        job = KIO::listRecursive(m_url, false);
    else
        job = KIO::listDir(m_url, false);

    connect(job, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)), SLOT(slotListEntries(KIO::Job *, const KIO::UDSEntryList &)));
    connect(job, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));
    connect(job, SIGNAL(canceled(KIO::Job *)), SLOT(slotCanceled(KIO::Job *)));
}

void KQuery::slotResult(KIO::Job *_job)
{
    if(job != _job)
        return;
    job = 0;

    m_result = _job->error();
    checkEntries();
}

void KQuery::slotCanceled(KIO::Job *_job)
{
    if(job != _job)
        return;
    job = 0;

    m_fileItems.clear();
    m_result = KIO::ERR_USER_CANCELED;
    checkEntries();
}

void KQuery::slotListEntries(KIO::Job *, const KIO::UDSEntryList &list)
{
    KFileItem *file = 0;
    KIO::UDSEntryListConstIterator end = list.end();
    for(KIO::UDSEntryListConstIterator it = list.begin(); it != end; ++it)
    {
        file = new KFileItem(*it, m_url, true, true);
        m_fileItems.enqueue(file);
    }
    checkEntries();
}

void KQuery::checkEntries()
{
    if(m_insideCheckEntries)
        return;
    m_insideCheckEntries = true;
    metaKeyRx = new QRegExp(m_metainfokey, true, true);
    KFileItem *file = 0;
    while((file = m_fileItems.dequeue()))
    {
        processQuery(file);
        delete file;
    }
    delete metaKeyRx;
    m_insideCheckEntries = false;
    if(job == 0)
        emit result(m_result);
}

/* List of files found using slocate */
void KQuery::slotListEntries(QStringList list)
{
    KFileItem *file = 0;
    metaKeyRx = new QRegExp(m_metainfokey, true, true);

    QStringList::Iterator it = list.begin();
    QStringList::Iterator end = list.end();

    for(; it != end; ++it)
    {
        file = new KFileItem(KFileItem::Unknown, KFileItem::Unknown, KURL::fromPathOrURL(*it));
        processQuery(file);
        delete file;
    }

    delete metaKeyRx;
}

/* Check if file meets the find's requirements*/
void KQuery::processQuery(KFileItem *file)
{
    QRegExp *filename_match;

    if(file->name() == "." || file->name() == "..")
        return;

    bool matched = false;

    for(filename_match = m_regexps.first(); !matched && filename_match; filename_match = m_regexps.next())
    {
        matched |= filename_match->isEmpty() || (filename_match->exactMatch(file->url().fileName(true)));
    }
    if(!matched)
        return;

    switch(m_sizemode)
    {
        case 1: // "at least"
            if(file->size() < m_sizeboundary1)
                return;
            break;
        case 2: // "at most"
            if(file->size() > m_sizeboundary1)
                return;
            break;
        case 3: // "equal"
            if(file->size() != m_sizeboundary1)
                return;
            break;
        case 4: // "between"
            if((file->size() < m_sizeboundary1) || (file->size() > m_sizeboundary2))
                return;
            break;
        case 0: // "none" -> fall to default
        default:
            break;
    }

    // make sure it's in the correct date range
    // what about 0 times?
    if(m_timeFrom && m_timeFrom > file->time(KIO::UDS_MODIFICATION_TIME))
        return;
    if(m_timeTo && m_timeTo < file->time(KIO::UDS_MODIFICATION_TIME))
        return;

    // username / group match
    if((!m_username.isEmpty()) && (m_username != file->user()))
        return;
    if((!m_groupname.isEmpty()) && (m_groupname != file->group()))
        return;

    // file type
    switch(m_filetype)
    {
        case 0:
            break;
        case 1: // plain file
            if(!S_ISREG(file->mode()))
                return;
            break;
        case 2:
            if(!file->isDir())
                return;
            break;
        case 3:
            if(!file->isLink())
                return;
            break;
        case 4:
            if(!S_ISCHR(file->mode()) && !S_ISBLK(file->mode()) && !S_ISFIFO(file->mode()) && !S_ISSOCK(file->mode()))
                return;
            break;
        case 5: // binary
            if((file->permissions() & 0111) != 0111 || file->isDir())
                return;
            break;
        case 6:                                        // suid
            if((file->permissions() & 04000) != 04000) // fixme
                return;
            break;
        default:
            if(!m_mimetype.isEmpty() && !m_mimetype.contains(file->mimetype()))
                return;
    }

    // match datas in metainfo...
    if((!m_metainfo.isEmpty()) && (!m_metainfokey.isEmpty()))
    {
        bool foundmeta = false;
        QString filename = file->url().path();

        if(filename.startsWith("/dev/"))
            return;

        KFileMetaInfo metadatas(filename);
        KFileMetaInfoItem metaitem;
        QStringList metakeys;
        QString strmetakeycontent;

        if(metadatas.isEmpty())
            return;

        metakeys = metadatas.supportedKeys();
        for(QStringList::Iterator it = metakeys.begin(); it != metakeys.end(); ++it)
        {
            if(!metaKeyRx->exactMatch(*it))
                continue;
            metaitem = metadatas.item(*it);
            strmetakeycontent = metaitem.string();
            if(strmetakeycontent.find(m_metainfo) != -1)
            {
                foundmeta = true;
                break;
            }
        }
        if(!foundmeta)
            return;
    }

    // match contents...
    QString matchingLine;
    if(!m_context.isEmpty())
    {

        if(!m_search_binary && ignore_mimetypes.findIndex(file->mimetype()) != -1)
        {
            kdDebug() << "ignoring, mime type is in exclusion list: " << file->url() << endl;
            return;
        }

        bool found = false;
        bool isZippedOfficeDocument = false;
        int matchingLineNumber = 0;

        // FIXME: doesn't work with non local files

        QString filename;
        QTextStream *stream = 0;
        QFile qf;
        QRegExp xmlTags;
        QByteArray zippedXmlFileContent;

        // KWord's and OpenOffice.org's files are zipped...
        if(ooo_mimetypes.findIndex(file->mimetype()) != -1 || koffice_mimetypes.findIndex(file->mimetype()) != -1)
        {
            KZip zipfile(file->url().path());
            KZipFileEntry *zipfileEntry;

            if(zipfile.open(IO_ReadOnly))
            {
                const KArchiveDirectory *zipfileContent = zipfile.directory();

                if(koffice_mimetypes.findIndex(file->mimetype()) != -1)
                    zipfileEntry = (KZipFileEntry *)zipfileContent->entry("maindoc.xml");
                else
                    zipfileEntry = (KZipFileEntry *)zipfileContent->entry("content.xml"); // for OpenOffice.org

                if(!zipfileEntry)
                {
                    kdWarning() << "Expected XML file not found in ZIP archive " << file->url() << endl;
                    return;
                }

                zippedXmlFileContent = zipfileEntry->data();
                xmlTags.setPattern("<.*>");
                xmlTags.setMinimal(true);
                stream = new QTextStream(zippedXmlFileContent, IO_ReadOnly);
                stream->setEncoding(QTextStream::UnicodeUTF8);
                isZippedOfficeDocument = true;
            }
            else
            {
                kdWarning() << "Cannot open supposed ZIP file " << file->url() << endl;
            }
        }
        else if(!m_search_binary && !file->mimetype().startsWith("text/") && file->url().isLocalFile())
        {
            KMimeType::Format f = KMimeType::findFormatByFileContent(file->url().path());
            if(!f.text)
            {
                kdDebug() << "ignoring, not a text file: " << file->url() << endl;
                return;
            }
        }

        if(!isZippedOfficeDocument) // any other file or non-compressed KWord
        {
            filename = file->url().path();
            if(filename.startsWith("/dev/"))
                return;
            qf.setName(filename);
            qf.open(IO_ReadOnly);
            stream = new QTextStream(&qf);
            stream->setEncoding(QTextStream::Locale);
        }

        while(!stream->atEnd())
        {
            QString str = stream->readLine();
            matchingLineNumber++;

            if(str.isNull())
                break;
            if(isZippedOfficeDocument)
                str.replace(xmlTags, "");

            if(m_regexpForContent)
            {
                if(m_regexp.search(str) >= 0)
                {
                    matchingLine = QString::number(matchingLineNumber) + ": " + str;
                    found = true;
                    break;
                }
            }
            else
            {
                if(str.find(m_context, 0, m_casesensitive) != -1)
                {
                    matchingLine = QString::number(matchingLineNumber) + ": " + str;
                    found = true;
                    break;
                }
            }
            kapp->processEvents();
        }
        delete stream;

        if(!found)
            return;
    }
    emit addFile(file, matchingLine);
}

void KQuery::setContext(const QString &context, bool casesensitive, bool search_binary, bool useRegexp)
{
    m_context = context;
    m_casesensitive = casesensitive;
    m_search_binary = search_binary;
    m_regexpForContent = useRegexp;
    m_regexp.setWildcard(!m_regexpForContent);
    m_regexp.setCaseSensitive(casesensitive);
    if(m_regexpForContent)
        m_regexp.setPattern(m_context);
}

void KQuery::setMetaInfo(const QString &metainfo, const QString &metainfokey)
{
    m_metainfo = metainfo;
    m_metainfokey = metainfokey;
}

void KQuery::setMimeType(const QStringList &mimetype)
{
    m_mimetype = mimetype;
}

void KQuery::setFileType(int filetype)
{
    m_filetype = filetype;
}

void KQuery::setSizeRange(int mode, KIO::filesize_t value1, KIO::filesize_t value2)
{
    m_sizemode = mode;
    m_sizeboundary1 = value1;
    m_sizeboundary2 = value2;
}

void KQuery::setTimeRange(time_t from, time_t to)
{
    m_timeFrom = from;
    m_timeTo = to;
}

void KQuery::setUsername(QString username)
{
    m_username = username;
}

void KQuery::setGroupname(QString groupname)
{
    m_groupname = groupname;
}


void KQuery::setRegExp(const QString &regexp, bool caseSensitive)
{
    QRegExp *regExp;
    QRegExp sep(";");
    QStringList strList = QStringList::split(sep, regexp, false);
    //  QRegExp globChars ("[\\*\\?\\[\\]]", TRUE,  FALSE);

    m_regexps.clear();
    //  m_regexpsContainsGlobs.clear();
    for(QStringList::ConstIterator it = strList.begin(); it != strList.end(); ++it)
    {
        regExp = new QRegExp((*it), caseSensitive, true);
        //    m_regexpsContainsGlobs.append(regExp->pattern().contains(globChars));
        m_regexps.append(regExp);
    }
}

void KQuery::setRecursive(bool recursive)
{
    m_recursive = recursive;
}

void KQuery::setPath(const KURL &url)
{
    m_url = url;
}

void KQuery::setUseFileIndex(bool useLocate)
{
    m_useLocate = useLocate;
}

void KQuery::slotreceivedSdterr(KProcess *, char *str, int)
{
    KMessageBox::error(NULL, QString(str), i18n("Error while using locate"));
}

void KQuery::slotreceivedSdtout(KProcess *, char *str, int l)
{
    int i;

    bufferLocateLength += l;
    str[l] = '\0';
    bufferLocate = (char *)realloc(bufferLocate, sizeof(char) * (bufferLocateLength));
    for(i = 0; i < l; i++)
        bufferLocate[bufferLocateLength - l + i] = str[i];
}

void KQuery::slotendProcessLocate(KProcess *)
{
    QString qstr;
    QStringList strlist;
    int i, j, k;

    if((bufferLocateLength == 0) || (bufferLocate == NULL))
    {
        emit result(0);
        return;
    }

    i = 0;
    do
    {
        j = 1;
        while(bufferLocate[i] != '\n')
        {
            i++;
            j++;
        }
        qstr = "";
        for(k = 0; k < j - 1; k++)
            qstr.append(bufferLocate[k + i - j + 1]);
        strlist.append(qstr);
        i++;

    } while(i < bufferLocateLength);
    bufferLocateLength = 0;
    free(bufferLocate);
    bufferLocate = NULL;
    slotListEntries(strlist);
    emit result(0);
}

#include "kquery.moc"

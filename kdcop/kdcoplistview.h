/*
 * Copyright (C) 2000 by Ian Reinhart Geiser <geiseri@kde.org>
 *
 * Licensed under the Artistic License.
 */

#ifndef __KDCOPLISTVIEW_H__
#define __KDCOPLISTVIEW_H__


#include <klistview.h>
class QDragObject;

class KDCOPListView : public KListView
{
  Q_OBJECT

  public:
  	KDCOPListView ( QWidget * parent = 0, const char * name = 0 );
	virtual ~KDCOPListView();
  	QDragObject *dragObject();
	void setMode(const QString &mode);
	QString getCurrentCode() const;

  private:
  	QString encode(QListViewItem *code);
	QString mode;

};

class DCOPBrowserItem : public QListViewItem
{
  public:

    enum Type { Application, Interface, Function };

    DCOPBrowserItem(QListView * parent, Type type);
    DCOPBrowserItem(QListViewItem * parent, Type type);

    virtual ~DCOPBrowserItem() {}

    Type type() const;

  private:

    Type type_;
};

class DCOPBrowserApplicationItem : public QObject, public DCOPBrowserItem
{
 Q_OBJECT
  public:

    DCOPBrowserApplicationItem(QListView * parent, const QCString & app);
    virtual ~DCOPBrowserApplicationItem() {}

    QCString app() const { return app_; }

    virtual void setOpen(bool o);

  protected:

    virtual void populate();

  private:

    QCString app_;
  private slots:
  /**
   * Theses two slots are used to get the icon of the application
   */
    void retreiveIcon(int callId, const QCString& replyType, const QByteArray &replyData);
	void slotGotWindowName(int callId, const QCString& replyType, const QByteArray &replyData);
};

class DCOPBrowserInterfaceItem : public QObject, public DCOPBrowserItem
{
  public:

    DCOPBrowserInterfaceItem
    (
     DCOPBrowserApplicationItem * parent,
     const QCString & app,
     const QCString & object,
     bool def
    );

    virtual ~DCOPBrowserInterfaceItem() {}

    QCString app() const { return app_; }
    QCString object() const { return object_; }

    virtual void setOpen(bool o);

  protected:

    virtual void populate();

  private:

    QCString app_;
    QCString object_;
};


class DCOPBrowserFunctionItem : public DCOPBrowserItem
{
  public:

    DCOPBrowserFunctionItem
    (
     DCOPBrowserInterfaceItem * parent,
     const QCString & app,
     const QCString & object,
     const QCString & function
    );

    virtual ~DCOPBrowserFunctionItem() {}

    QCString app() const { return app_; }
    QCString object() const { return object_; }
    QCString function() const { return function_; }

    virtual void setOpen(bool o);

  private:

    QCString app_;
    QCString object_;
    QCString function_;
};

#endif

#ifndef _TYPESLISTITEM_H
#define _TYPESLISTITEM_H

#include <qlistview.h>

#include <kmimetype.h>
#include <kuserprofile.h>

class TypesListItem : public QListViewItem
{
public:
  /**
   * Create a filetype group
   */
  TypesListItem(QListView *parent, const QString & major );

  /**
   * Create a filetype item inside a group
   */
  TypesListItem(TypesListItem *parent, KMimeType::Ptr mimetype, bool newItem=false);

  /**
   * Create a filetype item not inside a group (used by keditfiletype)
   */
  TypesListItem(QListView *parent, KMimeType::Ptr mimetype);

  ~TypesListItem();

  QString name() const { return m_major + "/" + m_minor; }
  QString majorType() const { return m_major; }
  QString minorType() const { return m_minor; }
  void setMinor(QString m) { m_minor = m; }
  QString comment() const { return m_comment; }
  void setComment(QString c) { m_comment = c; }
  /**
   * Returns true if "this" is a group
   */
  bool isMeta() const { return metaType; }
  /**
   * Returns true if the type is essential, i.e. can't be deleted
   * (see KMimeType::checkEssentialMimeTypes)
   */
  bool isEssential() const;
  QString icon() const { return m_icon; }
  void setIcon(const QString& i);
  QStringList patterns() const { return m_patterns; }
  void setPatterns(const QStringList &p) { m_patterns = p; }
  QStringList appServices() const;
  void setAppServices(const QStringList &dsl) { m_appServices = dsl; }
  QStringList embedServices() const;
  void setEmbedServices(const QStringList &dsl) { m_embedServices = dsl; }
  int autoEmbed() const { return m_autoEmbed; }
  void setAutoEmbed( int a ) { m_autoEmbed = a; }
  const KMimeType::Ptr& mimeType() const { return m_mimetype; }

  bool isDirty() const;
  void sync();
  void setup();

  static bool defaultEmbeddingSetting(  const QString& major );

private:
  void getServiceOffers( QStringList & appServices, QStringList & embedServices ) const;
  void saveServices( KSimpleConfig & profile, QStringList services, const QString & servicetype2 );
  KMimeType::Ptr m_mimetype;
  void initMeta( const QString & major );
  void init(KMimeType::Ptr mimetype);

  unsigned int groupCount:16; // shared between saveServices and sync
  unsigned int m_autoEmbed:2; // 0 yes, 1 no, 2 use group setting
  unsigned int metaType:1;
  unsigned int m_bNewItem:1;
  unsigned int m_bFullInit:1;
  QString m_major, m_minor, m_comment, m_icon;
  QStringList m_patterns;
  QStringList m_appServices;
  QStringList m_embedServices;
};

#endif

/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -a datamanagementadaptor -c DataManagementAdaptor -m org.kde.nepomuk.DataManagement.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DATAMANAGEMENTADAPTOR_H_1295626440
#define DATAMANAGEMENTADAPTOR_H_1295626440

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.nepomuk.DataManagement
 */
class DataManagementAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.nepomuk.DataManagement")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.nepomuk.DataManagement\">\n"
"    <method name=\"addProperty\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"property\"/>\n"
"      <arg direction=\"in\" type=\"av\" name=\"values\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"    <method name=\"setProperty\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"property\"/>\n"
"      <arg direction=\"in\" type=\"av\" name=\"values\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"    <method name=\"removeProperty\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"property\"/>\n"
"      <arg direction=\"in\" type=\"av\" name=\"values\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"    <method name=\"removeProperties\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"properties\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"    <method name=\"createResource\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"types\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"label\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"description\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"    <method name=\"removeResources\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"force\"/>\n"
"    </method>\n"
"    <method name=\"removeDataByApplication\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"force\"/>\n"
"    </method>\n"
"    <method name=\"removeDataByApplication\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"force\"/>\n"
"    </method>\n"
"    <method name=\"removePropertiesByApplication\">\n"
"      <arg direction=\"in\" type=\"as\" name=\"resources\"/>\n"
"      <arg direction=\"in\" type=\"as\" name=\"properties\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"app\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    DataManagementAdaptor(QObject *parent);
    virtual ~DataManagementAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void addProperty(const QString &resources, const QString &property, const QVariantList &values, const QString &app);
    QString createResource(const QStringList &types, const QString &label, const QString &description, const QString &app);
    void removeDataByApplication(const QString &app, bool force);
    void removeDataByApplication(const QStringList &resources, const QString &app, bool force);
    void removeProperties(const QStringList &resources, const QStringList &properties, const QString &app);
    void removePropertiesByApplication(const QStringList &resources, const QStringList &properties, const QString &app);
    void removeProperty(const QStringList &resources, const QString &property, const QVariantList &values, const QString &app);
    void removeResources(const QStringList &resources, const QString &app, bool force);
    void setProperty(const QString &resources, const QString &property, const QVariantList &values, const QString &app);
Q_SIGNALS: // SIGNALS
};

#endif

/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "servicetest.h"

#include "backend/backendmaster.h"
#include "backend/temporarycollectionmanager.h"
#include "daemon/service.h"

#include <qtest_kde.h>

#include <QtTest/QTest>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QtCrypto/QtCrypto>

#include <QtCore/QDebug>

void ServiceTest::initTestCase()
{
   QVERIFY(QDBusConnection::sessionBus().registerService("org.freedesktop.Secret"));

   QCA::init();
   
   m_master = new BackendMaster;
   m_master->addManager(new TemporaryCollectionManager(m_master));
   m_service = new Service(m_master);
}

void ServiceTest::dbusService()
{
   // make sure the service is available
   QDBusConnectionInterface *ifaceConn = QDBusConnection::sessionBus().interface();
   QVERIFY(ifaceConn && ifaceConn->isValid());

   QDBusReply<bool> registered = ifaceConn->isServiceRegistered("org.freedesktop.Secret");
   QVERIFY(registered.isValid());
   QVERIFY(registered.value());
}

void ServiceTest::session()
{
   QDBusInterface ifaceService("org.freedesktop.Secret", "/org/freedesktop/secrets");
   QVERIFY(ifaceService.isValid());
   QCA::KeyGenerator keygen;
   
   // "unsupported" session algorithm
   QList<QVariant> unsuppInput;
   unsuppInput << QString("unsupported") << QVariant::fromValue(QDBusVariant(QString("")));
   QDBusMessage unsuppReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                                                unsuppInput);
   QCOMPARE(unsuppReply.type(), QDBusMessage::ErrorMessage);
   QCOMPARE(unsuppReply.errorName(),
            QLatin1String("org.freedesktop.Secret.Error.NotSupported"));

   // "plain" session algorithm
   QDBusObjectPath plainPath;
   QList<QVariant> plainInput;
   plainInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
   QDBusMessage plainReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                                               plainInput);
   QCOMPARE(plainReply.type(), QDBusMessage::ReplyMessage);
   QList<QVariant> plainArgs = plainReply.arguments();
   QCOMPARE(plainArgs.size(), 2);
   QCOMPARE(plainArgs.at(0).userType(), qMetaTypeId<QDBusVariant>());
   QCOMPARE(plainArgs.at(0).value<QString>(), QLatin1String(""));
   QCOMPARE(plainArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
   plainPath = plainArgs.at(1).value<QDBusObjectPath>();
   QVERIFY(plainPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/session/")));
   QDBusInterface plainIface("org.freedesktop.Secret", plainPath.path(),
                             "org.freedesktop.Secret.Session");
   QVERIFY(plainIface.isValid());
   QDBusMessage plainReply2 = plainIface.call("Close");
   QCOMPARE(plainReply2.type(), QDBusMessage::ReplyMessage);
   QCOMPARE(plainIface.call("Introspect").type(), QDBusMessage::ErrorMessage);

   // "dh-ietf1024-aes128-cbc-pkcs7" encryption
   QDBusObjectPath dhPath;
   QList<QVariant> dhInput;
   QCA::DLGroup dhDlgroup(keygen.createDLGroup(QCA::IETF_1024));
   QVERIFY(!dhDlgroup.isNull());
   QCA::PrivateKey dhPrivkey(keygen.createDH(dhDlgroup));
   QCA::PublicKey dhPubkey(dhPrivkey);
   QByteArray dhBytePub(dhPubkey.toDH().y().toArray().toByteArray());
   dhInput << QString("dh-ietf1024-aes128-cbc-pkcs7")
           << QVariant::fromValue(QDBusVariant(dhBytePub));
   QDBusMessage dhReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                                            dhInput);
   QCOMPARE(dhReply.type(), QDBusMessage::ReplyMessage);
   QList<QVariant> dhArgs = dhReply.arguments();
   QCOMPARE(dhArgs.size(), 2);
   QCOMPARE(dhArgs.at(0).userType(), qMetaTypeId<QDBusVariant>());
   QVariant dhOutputVar = dhArgs.at(0).value<QDBusVariant>().variant();
   QCOMPARE(dhOutputVar.type(), QVariant::ByteArray);
   QByteArray dhOutput = dhOutputVar.toByteArray();
   QCA::DHPublicKey dhServiceKey(dhDlgroup,
                                 QCA::BigInteger(QCA::SecureArray(dhOutput)));
   QCA::SymmetricKey dhSharedKey(dhPrivkey.deriveKey(dhServiceKey));
   QCA::Cipher *dhCipher = new QCA::Cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::PKCS7);
   QVERIFY(dhSharedKey.size() >= dhCipher->keyLength().minimum());
   QCOMPARE(plainArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
   dhPath = dhArgs.at(1).value<QDBusObjectPath>();
   QVERIFY(dhPath.path().startsWith(QLatin1String("/org/freedesktop/secrets/session/")));
   QDBusInterface dhIface("org.freedesktop.Secret", dhPath.path(),
                          "org.freedesktop.Secret.Session");
   QVERIFY(dhIface.isValid());
   QDBusMessage dhReply2 = dhIface.call("Close");
   QCOMPARE(dhReply2.type(), QDBusMessage::ReplyMessage);
   QCOMPARE(dhIface.call("Introspect").type(), QDBusMessage::ErrorMessage);
}

void ServiceTest::nonBlockingCollection()
{
   QDBusInterface ifaceService("org.freedesktop.Secret", "/org/freedesktop/secrets");
   QVERIFY(ifaceService.isValid());

   // create a session
   QDBusObjectPath sessionPath;
   QList<QVariant> sessionInput;
   sessionInput << QString("plain") << QVariant::fromValue(QDBusVariant(""));
   QDBusMessage sessionReply = ifaceService.callWithArgumentList(QDBus::Block, "OpenSession",
                                                                 sessionInput);
   sessionPath = sessionReply.arguments().at(1).value<QDBusObjectPath>();

   // listen to CollectionCreated/CollectionDeleted/CollectionChanged signals
   ObjectPathSignalSpy createdSpy(&ifaceService, SIGNAL(CollectionCreated(QDBusObjectPath)));
   QVERIFY(createdSpy.isValid());
   ObjectPathSignalSpy deletedSpy(&ifaceService, SIGNAL(CollectionDeleted(QDBusObjectPath)));
   QVERIFY(deletedSpy.isValid());
   ObjectPathSignalSpy changedSpy(&ifaceService, SIGNAL(CollectionChanged(QDBusObjectPath)));
   QVERIFY(changedSpy.isValid());
   
   // create a collection
   QDBusObjectPath collectionPath;
   QMap<QString, QVariant> createProperties;
   QList<QVariant> createInput;
   createProperties["Label"] = "test";
   createProperties["Locked"] = false; // create collection unlocked
   createInput << QVariant::fromValue(createProperties);
   QDBusMessage createReply = ifaceService.callWithArgumentList(QDBus::Block, "CreateCollection",
                                                                createInput);
   QCOMPARE(createReply.type(), QDBusMessage::ReplyMessage);
   QList<QVariant> createArgs = createReply.arguments();
   QCOMPARE(createArgs.size(), 2);
   QCOMPARE(createArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
   QCOMPARE(createArgs.at(1).userType(), qMetaTypeId<QDBusObjectPath>());
   // TemporaryCollection is non-blocking, so the second output (prompt) should be "/".
   QCOMPARE(createArgs.at(1).value<QDBusObjectPath>().path(), QLatin1String("/"));
   collectionPath = createArgs.at(0).value<QDBusObjectPath>();
   QVERIFY(collectionPath.path().startsWith(
           QLatin1String("/org/freedesktop/secrets/collection/")));
   QDBusInterface ifaceCollection("org.freedesktop.Secret", collectionPath.path(),
                                  "org.freedesktop.Secret.Collection");
   QVERIFY(ifaceCollection.isValid());

   // make sure the CollectionCreated signal was sent
   if (createdSpy.size() < 1) {
      createdSpy.waitForSignal(5000);
   }
   QCOMPARE(createdSpy.size(), 1);
   QCOMPARE(createdSpy.takeFirst(), collectionPath);

   // read collection properties
   QVariant propItems = ifaceCollection.property("Items");
   QVERIFY(propItems.isValid());
   QVERIFY(propItems.canConvert<QList<QDBusObjectPath> >());
   QList<QDBusObjectPath> propItemsList = propItems.value<QList<QDBusObjectPath> >();
   QVERIFY(propItemsList.isEmpty());
   QVariant propLabel = ifaceCollection.property("Label");
   QVERIFY(propLabel.isValid());
   QCOMPARE(propLabel.type(), QVariant::String);
   QCOMPARE(propLabel.value<QString>(), QString("test"));
   QVariant propLocked = ifaceCollection.property("Locked");
   QVERIFY(propLocked.isValid());
   QCOMPARE(propLocked.value<bool>(), false);
   QVariant propCreated = ifaceCollection.property("Created");
   QVERIFY(propCreated.isValid());
   QCOMPARE(propCreated.type(), QVariant::ULongLong);
   qulonglong propCreatedUll = propCreated.value<qulonglong>();
   QVERIFY(QDateTime::currentDateTime().toTime_t() - propCreatedUll < 60);
   QVariant propModified = ifaceCollection.property("Modified");
   QVERIFY(propModified.isValid());
   QCOMPARE(propModified.type(), QVariant::ULongLong);
   QCOMPARE(propModified.value<qulonglong>(), propCreatedUll);

   // delete the collection
   QDBusMessage deleteReply = ifaceCollection.call(QDBus::Block, "Delete");
   QCOMPARE(deleteReply.type(), QDBusMessage::ReplyMessage);
   QList<QVariant> deleteArgs = deleteReply.arguments();
   QCOMPARE(deleteArgs.size(), 1);
   QCOMPARE(deleteArgs.at(0).userType(), qMetaTypeId<QDBusObjectPath>());
   // TemporaryCollection is non-blocking, so the output (prompt) should be "/".
   QCOMPARE(deleteArgs.at(0).value<QDBusObjectPath>().path(), QLatin1String("/"));
   // make sure the collection is gone
   QCOMPARE(ifaceCollection.call("Introspect").type(), QDBusMessage::ErrorMessage);
   
   // make sure the CollectionDeleted signal was sent
   if (deletedSpy.size() < 1) {
      deletedSpy.waitForSignal(5000);
   }
   QCOMPARE(deletedSpy.size(), 1);
   QCOMPARE(deletedSpy.takeFirst(), collectionPath);
}

void ServiceTest::cleanupTestCase()
{
   delete m_service;
   delete m_master;

   QCA::deinit();
}

QTEST_KDEMAIN_CORE(ServiceTest)

ObjectPathSignalSpy::ObjectPathSignalSpy(QDBusAbstractInterface *iface,
                                         const char *signal)
{
   m_valid = connect(iface, signal, SLOT(slotReceived(QDBusObjectPath)));
}

bool ObjectPathSignalSpy::isValid() const
{
   return m_valid;
}

void ObjectPathSignalSpy::waitForSignal(int time)
{
   QEventLoop loop;
   loop.connect(this, SIGNAL(doneWaiting()), SLOT(quit()));
   QTimer::singleShot(time, &loop, SLOT(quit()));
   loop.exec();
}

void ObjectPathSignalSpy::slotReceived(const QDBusObjectPath &objectPath)
{
   append(objectPath);
   emit doneWaiting();
}

#include "servicetest.moc"

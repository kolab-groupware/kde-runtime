/*******************************************************************
* drkonqiassistantpages_bugzilla.cpp
* Copyright 2009    Dario Andres Rodriguez <andresbajotierra@gmail.com>
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
******************************************************************/

#include "drkonqiassistantpages_bugzilla.h"
#include "drkonqi.h"
#include "reportinfo.h"
#include "krashconf.h"
#include "backtraceparser.h"
#include "backtracegenerator.h"
#include "statuswidget.h"

#include <QtCore/QDate>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QFormLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QTreeWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QStackedWidget>

#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <ktextedit.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <klineedit.h>
#include <kcombobox.h>

//BEGIN BugzillaLoginPage

BugzillaLoginPage::BugzillaLoginPage( DrKonqiBugReport * parent ) : 
    DrKonqiAssistantPage(parent),
    m_wallet(0)
{
    connect( reportInfo()->getBZ(), SIGNAL(loginFinished(bool)), this, SLOT(loginFinished(bool)));
    connect( reportInfo()->getBZ(), SIGNAL(loginError(QString)), this, SLOT(loginError(QString)));
    
    m_statusWidget = new StatusWidget();
    m_statusWidget->setIdle( i18n( "You need to log-in in your bugs.kde.org account in order to proceed" ) );
    
    m_loginButton = new KPushButton( KGuiItem( i18nc( "button action", "Login in" ), KIcon( "network-connect" ), i18nc( "button explanation", "Use this button to try to login to a bugs.kde.org Bugzilla account using the provided username and password"),  i18nc( "button explanation", "Use this button to try to login to a bugs.kde.org Bugzilla account using the provided username and password") ) );
    connect( m_loginButton, SIGNAL(clicked()) , this, SLOT(loginClicked()) );
    
    m_userEdit = new KLineEdit();
    connect( m_userEdit, SIGNAL(returnPressed()) , this, SLOT(loginClicked()) );
    
    m_passwordEdit = new KLineEdit();
    m_passwordEdit->setEchoMode( QLineEdit::Password );
    connect( m_passwordEdit, SIGNAL(returnPressed()) , this, SLOT(loginClicked()) );
    
    m_form = new QFormLayout();
    m_form->addRow( i18nc( "label for username lineedit input", "Username:" ) , m_userEdit );
    m_form->addRow( i18nc( "label for password lineedit input", "Password:" ) , m_passwordEdit );

    QString url = QString("http://bugs.kde.org");
    QLabel * noticeLabel = new QLabel( i18n("<strong>Notice:</strong> You need a user account at the <link url='%1'>KDE BugTracker</link> in order to file a bug report because we may need to contact you later for requesting further information. <br />If you don't have one: you can freely <link url='%2'>create one here</link>", QLatin1String("http://bugs.kde.org"), QLatin1String("https://bugs.kde.org/createaccount.cgi") ));
    
    noticeLabel->setWordWrap( true );
    noticeLabel->setOpenExternalLinks( true );
    
    QHBoxLayout * buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget( m_loginButton);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( m_statusWidget );
    layout->addLayout( m_form );
    layout->addLayout( buttonLayout );
    layout->addWidget( noticeLabel );
    layout->addStretch();
    setLayout( layout );
}

bool BugzillaLoginPage::isComplete()
{
    return reportInfo()->getBZ()->getLogged();
}

/*
void BugzillaLoginPage::progress( KJob*, unsigned long percent )
{
    //qDebug() << "progress" << percent;
}
*/
void BugzillaLoginPage::loginError( QString err )
{
    loginFinished( false );
    m_statusWidget->setIdle( i18n("Error when trying to login: %1", err ) );
}

void BugzillaLoginPage::aboutToShow()
{
    if( reportInfo()->getBZ()->getLogged() )
    {
        m_loginButton->setEnabled( false );
    
        m_userEdit->setEnabled( false );
        m_userEdit->setText("");
        m_passwordEdit->setEnabled( false );
        m_passwordEdit->setText("");
        
        m_loginButton->setVisible( false );
        m_userEdit->setVisible( false );
        m_passwordEdit->setVisible( false );
        m_form->labelForField(m_userEdit)->setVisible( false );
        m_form->labelForField(m_passwordEdit)->setVisible( false );
        
        m_statusWidget->setIdle( i18nc( "the user is logged at the bugtracker site as USERNAME", "Logged in at KDE Bugtracker (bugs.kde.org) as: %1", reportInfo()->getBZ()->getUsername() ) );
    }
    else
    {
        //Try to show wallet dialog once this dialog is shown
        QTimer::singleShot( 100, this, SLOT(walletLogin())); 
    }
}

void BugzillaLoginPage::walletLogin()
{
    bool login = false;
    if ( !m_wallet )
    {
        if ( !KWallet::Wallet::keyDoesNotExist( KWallet::Wallet::NetworkWallet(), KWallet::Wallet::FormDataFolder(), QLatin1String( "drkonqi_bugzilla" ) ) ) //Key exists!
        {
            m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), this->winId() );
            //Was the wallet opened?
            if( m_wallet )
            {
                m_wallet->setFolder( KWallet::Wallet::FormDataFolder() );
                
                //Use wallet data to try login
                QMap<QString, QString> values;
                m_wallet->readMap( QLatin1String( "drkonqi_bugzilla" ), values);
                QString username = values.value( QLatin1String( "username" ) );
                QString password = values.value( QLatin1String( "password" ) );
                
                if( !username.isEmpty() && !password.isEmpty() )
                {
                    login = true;
                    m_userEdit->setText( username );
                    m_passwordEdit->setText( password );
                }
            }
        }
    }
}

void BugzillaLoginPage::loginClicked()
{
    if( !( m_userEdit->text().isEmpty() && m_passwordEdit->text().isEmpty() ) )
    {
        m_loginButton->setEnabled( false );

        m_userEdit->setEnabled( false );
        m_passwordEdit->setEnabled( false );

        //If the wallet wasn't initialized at startup, launch it now to save the data
        if( !m_wallet )
        {
            m_wallet = KWallet::Wallet::openWallet( KWallet::Wallet::NetworkWallet(), this->winId() );
        }
        //Got wallet open ?
        if( m_wallet )
        {
            m_wallet->setFolder( KWallet::Wallet::FormDataFolder() );
            
            QMap<QString,QString> values;
            values.insert( QLatin1String( "username" ), m_userEdit->text() );
            values.insert( QLatin1String( "password" ), m_passwordEdit->text() );
            m_wallet->writeMap( QLatin1String( "drkonqi_bugzilla" ), values );
        }

        m_statusWidget->setBusy( i18n( "Performing bugs.kde.org login as %1 ...", m_userEdit->text() ) );
        
        reportInfo()->getBZ()->setLoginData( m_userEdit->text(), m_passwordEdit->text() );
        reportInfo()->getBZ()->tryLogin();
    }
    else
    {
        loginFinished( false );
    }
}

void BugzillaLoginPage::loginFinished( bool logged )
{
    if( logged )
    {
        emitCompleteChanged();
        
        aboutToShow();
        if( m_wallet )
            if( m_wallet->isOpen() )
                m_wallet->lockWallet();
    } 
    else
    {
        m_statusWidget->setIdle( i18n( "Invalid username or password" ) );
        
        m_loginButton->setEnabled( true );
    
        m_userEdit->setEnabled( true );
        m_passwordEdit->setEnabled( true );
        
        m_userEdit->setFocus( Qt::OtherFocusReason );
    }
}

BugzillaLoginPage::~BugzillaLoginPage()
{
    if( m_wallet )
    {
        if( m_wallet->isOpen() ) //Close wallet if we close the assistant in this step
            m_wallet->lockWallet();
        delete m_wallet;
    }
}

//END BugzillaLoginPage

//BEGIN BugzillaKeywordsPage

BugzillaKeywordsPage::BugzillaKeywordsPage( DrKonqiBugReport * parent ) : 
    DrKonqiAssistantPage(parent),
    m_keywordsOK(false)
{
    QLabel * detailsLabel = new QLabel(
    i18n( "Enter at least four words to describe the crash. You can enter the application name or another keyword that could describe the crash situation. This keywords will be used to search for already reported bugs that could be the same (possible duplicates). <strong>Notice:</strong> you must use English words" ) //TODO rewrite native-english
    );
    detailsLabel->setWordWrap( true );
    
    m_keywordsEdit = new KLineEdit();
    connect( m_keywordsEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited(QString)) );
    
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( detailsLabel );
    layout->addWidget( m_keywordsEdit );
    layout->addStretch();
    setLayout( layout );
}

void BugzillaKeywordsPage::textEdited( QString newText )
{
    QStringList list = newText.split(' ', QString::SkipEmptyParts);
    
    bool ok = (list.count() >= 4); //At least four (valid) words
    
    //Check words size
    /* FIXME, some words can be short, like version numbers
    if ( ok )
    {
        Q_FOREACH( const QString & word, list)
        {
            if( word.size() <= 3 )
                ok = false;
        }
    }
    */
    
    if ( ok != m_keywordsOK )
    {
        m_keywordsOK = ok;
        emitCompleteChanged();
    }
}

bool BugzillaKeywordsPage::isComplete()
{
    return m_keywordsOK;
}

void BugzillaKeywordsPage::aboutToShow()
{
    textEdited( m_keywordsEdit->text() );
}

void BugzillaKeywordsPage::aboutToHide()
{
    //Save keywords (as short description of the future report)
    reportInfo()->getReport()->setShortDescription( m_keywordsEdit->text() );
}

//END BugzillaKeywordsPage

//BEGIN BugzillaDuplicatesPage

BugzillaDuplicatesPage::BugzillaDuplicatesPage( DrKonqiBugReport * parent ):
    DrKonqiAssistantPage(parent),
    m_searching(false),
    m_infoDialog(0),
    m_infoDialogBrowser(0),
    m_infoDialogLink(0),
    m_infoDialogStatusWidget(0),
    m_mineMayBeDuplicateButton(0),
    m_currentBugNumber(0)
{
    m_endDate = QDate::currentDate();
    m_startDate = m_endDate.addYears( -1 );

    connect( reportInfo()->getBZ(), SIGNAL( searchFinished(BugMapList) ), this, SLOT( searchFinished(BugMapList) ) );
    connect( reportInfo()->getBZ(), SIGNAL( searchError(QString) ), this, SLOT( searchError(QString) ) );
    connect( reportInfo()->getBZ(), SIGNAL( bugReportFetched(BugReport*) ), this, SLOT( bugFetchFinished(BugReport*) ) );
    connect( reportInfo()->getBZ(), SIGNAL( bugReportError(QString) ), this, SLOT( bugFetchError(QString) ) );
        
    m_statusWidget = new StatusWidget();
    
    m_bugListWidget = new QTreeWidget();
    m_bugListWidget->setRootIsDecorated( false );
    m_bugListWidget->setWordWrap( true );
    m_bugListWidget->setHeaderLabels( QStringList() << i18n( "Bug ID" ) << i18n( "Description" ) );
    m_bugListWidget->setAlternatingRowColors( true );
    
    connect( m_bugListWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemClicked(QTreeWidgetItem*,int)) );
    
    QHeaderView * header = m_bugListWidget->header();
    header->setResizeMode( 0, QHeaderView::ResizeToContents );
    header->setResizeMode( 1, QHeaderView::Interactive );
    
    m_searchMoreButton = new KPushButton( KGuiItem( i18nc( "button action", "Search more reports" ), KIcon("edit-find"), i18nc( " button explanation", "Use this button to search more similar bug reports on an earlier date"), i18nc( " button explanation", "Use this button to search more similar bug reports on an earlier date") ) );
    connect( m_searchMoreButton, SIGNAL(clicked()), this, SLOT(searchMore()) );
    m_searchMoreButton->setEnabled( false );
    
    QHBoxLayout * buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget( m_searchMoreButton );
    
    m_foundDuplicateCheckBox = new QCheckBox( i18n( "My crash may be a duplicate of bug number:" ) );
    connect( m_foundDuplicateCheckBox , SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)) );
    
    m_possibleDuplicateEdit = new KLineEdit();
    m_possibleDuplicateEdit->setInputMask( "000000" );
    m_possibleDuplicateEdit->setEnabled( false );

    QHBoxLayout * lay = new QHBoxLayout();
    lay->addWidget( m_foundDuplicateCheckBox );
    lay->addStretch(); 
    lay->addWidget( m_possibleDuplicateEdit );
    
    QLabel * explanationLabel = new QLabel("This is an optional step when you can try to find an already reported bug that could match the crash you got. If there are search results you can double click some list item and compare the situations. Then, you can suggest that your crash could be a duplicate of that report."); //TODO native-english
    explanationLabel->setWordWrap( true );
    
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( explanationLabel ); 
    layout->addWidget( m_statusWidget );
    layout->addWidget( m_bugListWidget );
    layout->addLayout( buttonLayout );
    layout->addLayout( lay );
    setLayout( layout );
}

BugzillaDuplicatesPage::~BugzillaDuplicatesPage()
{
    delete m_infoDialog;
    delete m_infoDialogBrowser;
}

void BugzillaDuplicatesPage::enableControls( bool enable )
{
    m_bugListWidget->setEnabled( enable );
    m_searchMoreButton->setEnabled( enable );
    m_foundDuplicateCheckBox->setEnabled( enable );
    m_possibleDuplicateEdit->setEnabled( enable );
    if ( enable )
        checkBoxChanged( m_foundDuplicateCheckBox->checkState() );
}

void BugzillaDuplicatesPage::searchError( QString err )
{
    enableControls( true );
    
    m_statusWidget->setIdle( i18n( "Error fetching the bug report list" ) ); 
    
    m_searching = false;
    
    //1 year forward
    m_startDate = m_endDate;
    m_endDate = m_startDate.addYears( 1 );
    
    KMessageBox::error( this , i18n( "Error fetching the bug report list<br />%1<br />Please, wait some time and try again", err ) );
}

void BugzillaDuplicatesPage::bugFetchError( QString err )
{
    if( m_infoDialog )
    {
        if ( m_infoDialog->isVisible() )
        {
            KMessageBox::error( this , i18n( "Error fetching the bug report<br />%1<br />Please, wait some time and try again", err ) );
            m_mineMayBeDuplicateButton->setEnabled( false );
            m_infoDialogBrowser->setText( i18n( "Error fetching the bug report" ) );
            m_infoDialogStatusWidget->setIdle( i18n( "Error fetching the bug report" ) );
        }
    }
}

void BugzillaDuplicatesPage::checkBoxChanged( int newState )
{
    m_possibleDuplicateEdit->setEnabled( newState == Qt::Checked );
}

void BugzillaDuplicatesPage::itemClicked( QTreeWidgetItem * item, int col )
{
    Q_UNUSED( col );
    
    if( !m_infoDialog ) //Build info dialog
    {
        m_infoDialog = new KDialog( this );
        m_infoDialog->setButtons( KDialog::Close );
        m_infoDialog->setDefaultButton( KDialog::Close );
        m_infoDialog->setCaption( i18n("Bug Description") );
        m_infoDialog->setModal( true );
        
        m_infoDialogStatusWidget = new StatusWidget();
        
        m_infoDialogBrowser = new KTextBrowser(); 
        
        m_infoDialogLink = new QLabel();
        m_infoDialogLink->setOpenExternalLinks( true );
        
        m_mineMayBeDuplicateButton = new KPushButton( KGuiItem( i18nc("button action", "My crash may be a duplicate of this report"), KIcon(), i18nc( "button explanation", "Use this button to mark your crash as related to this current bug report. This will be used to determine if they are duplicates or completely unrelated"), i18nc( "button explanation", "Use this button to mark your crash as related to this current bug report. This will be used to determine if they are duplicates or completely unrelated") ) );
        connect( m_mineMayBeDuplicateButton, SIGNAL(clicked()) , this, SLOT(mayBeDuplicateClicked()) );
        
        QWidget * widget = new QWidget( m_infoDialog );
        QVBoxLayout * layout = new QVBoxLayout();
        layout->addWidget( m_infoDialogStatusWidget );
        layout->addWidget( m_infoDialogBrowser );
        
        QHBoxLayout * inLayout = new QHBoxLayout();
        inLayout->addWidget( m_infoDialogLink );
        inLayout->addWidget( m_mineMayBeDuplicateButton );
        
        layout->addLayout( inLayout );
        
        widget->setMinimumSize( QSize(400,300) );
        widget->setLayout( layout );
        
        m_infoDialog->setMainWidget( widget );
    }
    
    m_currentBugNumber = item->text(0).toInt();
    
    reportInfo()->getBZ()->fetchBugReport( m_currentBugNumber );

    m_mineMayBeDuplicateButton->setEnabled( false );
    
    m_infoDialogBrowser->setText( i18n("Loading ... " ) );
    m_infoDialogLink->setText( QString("<a href=\"%1\">%2</a>").arg( reportInfo()->getBZ()->urlForBug(m_currentBugNumber), i18n("Bug report page at KDE Bugtracker") ) );
    
    m_infoDialogStatusWidget->setBusy( i18n("Loading information about bug %1 from bugs.kde.org ...", QString::number(m_currentBugNumber) ) );
    
    m_infoDialogBrowser->setEnabled( false );
    
    m_infoDialog->show();
}

void BugzillaDuplicatesPage::mayBeDuplicateClicked()
{
    m_foundDuplicateCheckBox->setCheckState( Qt::Checked );
    m_possibleDuplicateEdit->setText( QString::number( m_currentBugNumber ) );
    m_infoDialog->close();
}

void BugzillaDuplicatesPage::bugFetchFinished( BugReport* report )
{
    if( report->isValid() )
    {
        if( m_infoDialog )
        {
            if ( m_infoDialog->isVisible() )
            {
                QString comments;
                QStringList commentList = report->comments();
                for(int i = 0;i < commentList.count(); i++)
                {
                    QString comment = commentList.at(i);
                    comment.replace('\n', "<br />");
                    comments += "<br /><strong>----</strong><br />" + comment;
                }
                
                QString text = 
                QString( "<strong>%1:</strong> %2<br />" ).arg( i18n("Bug ID"), report->bugNumber() ) +
                QString( "<strong>%1:</strong> %2/%3<br />" ).arg( i18nc("Product name at bugzilla","Product"), report->product(), report->component() ) +
                QString( "<strong>%1:</strong> %2<br />" ).arg( i18n("Short Description"), report->shortDescription() ) +
                QString( "<strong>%1:</strong> %2<br />" ).arg( i18n("Status"), report->bugStatus() ) +
                QString( "<strong>%1:</strong> %2<br />" ).arg( i18n("Resolution"), report->resolution() ) +
                QString( "<strong>%1:</strong><br />%2" ).arg( i18n("Full Description:"), report->description().replace('\n',"<br />") ) +
                QString( "<br /><br /><strong>%1:</strong> %2" ).arg( i18n("Comments"), comments );
                
                m_infoDialogBrowser->setText( text );
                m_mineMayBeDuplicateButton->setEnabled( true );
                
                m_infoDialogStatusWidget->setIdle( i18n("Showing report %1", report->bugNumber() ) );
                m_infoDialogBrowser->setEnabled( true );
            }
        }
    }
    else
    {
        bugFetchError( i18n( "Invalid report" ) );
    }
    
    delete report;
}

bool BugzillaDuplicatesPage::canSearchMore()
{
    return ( m_startDate.year() >= 2002 );
}

void BugzillaDuplicatesPage::aboutToShow()
{
    //This shouldn't happen as I can't move page when I'm searching
    Q_ASSERT( !m_searching );
    
    //If I never searched before, performSearch
    if ( m_bugListWidget->topLevelItemCount() == 0 && canSearchMore())
        performSearch();
}

void BugzillaDuplicatesPage::aboutToHide()
{
    if( (m_foundDuplicateCheckBox->checkState() == Qt::Checked)
        && !m_possibleDuplicateEdit->text().isEmpty() ) 
    {
        reportInfo()->setPossibleDuplicate( m_possibleDuplicateEdit->text() );
    }

}

void BugzillaDuplicatesPage::performSearch()
{
    m_searching = true;

    enableControls( false );

    QString startDateStr = m_startDate.toString("yyyy-MM-dd");
    QString endDateStr = m_endDate.toString("yyyy-MM-dd");
    
    m_statusWidget->setBusy( i18n( "Searching for duplicates (from %1 to %2) ...", startDateStr, endDateStr ) );

    const KrashConfig *krashConfig = DrKonqi::instance()->krashConfig();
    BacktraceParser *btParser = DrKonqi::instance()->backtraceGenerator()->parser();

    reportInfo()->getBZ()->searchBugs( reportInfo()->getReport()->shortDescription(), krashConfig->productName(), "crash", startDateStr, endDateStr , btParser->firstValidFunctions().join(" ") );
   
    //Test search
    //reportInfo()->getBZ()->searchBugs( "konqueror crash toggle mode", "konqueror", "crash", startDateStr, endDateStr , "caret" );
   
}

void BugzillaDuplicatesPage::searchMore()
{
    //1 year back
    m_endDate = m_startDate;
    m_startDate = m_endDate.addYears( -1 );
    
    performSearch();
}

void BugzillaDuplicatesPage::searchFinished( const BugMapList & list )
{
    m_searching = false;
    
    int results = list.count();
    if( results > 0 )
    {
        m_statusWidget->setIdle( i18n( "Showing results from %1 to %2", m_startDate.toString("yyyy-MM-dd"), QDate::currentDate().toString("yyyy-MM-dd") ) );
        
        for(int i = 0; i< results; i++)
        {
            BugMap bug = list.at(i);
            
            QStringList fields = QStringList() << bug["bug_id"] << bug["short_desc"];
            m_bugListWidget->addTopLevelItem( new QTreeWidgetItem( fields )  );
        }
        
        m_bugListWidget->sortItems( 0 , Qt::DescendingOrder );

        enableControls( true );
        
        if( !canSearchMore() )
            m_searchMoreButton->setEnabled( false );
        
    } else {

        if( canSearchMore() )
        {
            searchMore();
        }
        else
        {
            m_statusWidget->setIdle( i18n( "Search Finished. No more possible date ranges to search" ) );
            
            enableControls( false );
        }
    }
}

//END BugzillaDuplicatesPage


//BEGIN BugzillaInformationPage

BugzillaInformationPage::BugzillaInformationPage( DrKonqiBugReport * parent )
    : DrKonqiAssistantPage(parent),
    m_textsOK( false )
{
    m_titleLabel = new QLabel( i18n( "Title of the bug report" ) );
    m_titleEdit = new KLineEdit();
    connect( m_titleEdit, SIGNAL(textChanged(QString)), this, SLOT(checkTexts()) );
    
    m_detailsLabel = new QLabel( i18n( "Details of the crash situation" ) );
    m_detailsEdit = new KTextEdit();
    connect( m_detailsEdit, SIGNAL(textChanged()), this, SLOT(checkTexts()) );

    QVBoxLayout * layout = new QVBoxLayout();
    
    QLabel * explanationLabel = new QLabel( "Complete the bug report fields in order to properly submit it. All the fields should be at least 20 characters. <strong>Notice:</strong> All the texts should be written in english." ); //TODO native-rewrite (text length?)
    explanationLabel->setWordWrap( true );
    
    layout->addWidget( explanationLabel ); 
    
    layout->addWidget( m_titleLabel );
    layout->addWidget( m_titleEdit );
    layout->addWidget( m_detailsLabel );
    layout->addWidget( m_detailsEdit );
    layout->addStretch();
    layout->addWidget( new QLabel( i18n( "<strong>Notice:</strong> The crash information will be automatically integrated into the bug report" ) ) );
    
    setLayout( layout );
}

void BugzillaInformationPage::aboutToShow()
{
    if( m_titleEdit->text().isEmpty() )
        m_titleEdit->setText( reportInfo()->getReport()->shortDescription() );
        
    bool canDetail = reportInfo()->getUserCanDetail();
    
    m_detailsLabel->setVisible( canDetail );
    m_detailsEdit->setVisible( canDetail );
    
    checkTexts(); //May be the options (canDetail) changed and we need to recheck
}
    
void BugzillaInformationPage::checkTexts()
{
    bool detailsEmpty = m_detailsEdit->isVisible() ? m_detailsEdit->toPlainText().isEmpty() : false;
    bool ok = !(m_titleEdit->text().isEmpty() || detailsEmpty );
    
    if( ok != m_textsOK )
    {
        m_textsOK = ok;
        emitCompleteChanged();
    }
}

bool BugzillaInformationPage::showNextPage()
{
    checkTexts();
    if( m_textsOK ) //not empty
    {
        bool titleShort = m_titleEdit->text().size() < 50;
        bool detailsShort = ( m_detailsEdit->isVisible() && m_detailsEdit->toPlainText().size() < 150);
        
        if ( titleShort || detailsShort )
        {
            QString message;
            
            if( titleShort && !detailsShort )
            {
                message = i18n("The title is too short.");
            }
            else if ( detailsShort && !titleShort )
            {
                message = i18n("The description about the crash details is too short.");
            }
            else
            {
                message = i18n("Both the title and the description about the crash details are too short.");
            }
            
            message += i18n(" Can you write a bit more ?"); //FIXME rewrite this
            
            //The user input is less than we want.... encourage to write more
            if( KMessageBox::questionYesNo( this, message,
                i18n("Fields length too short") ) == KMessageBox::Yes )
            {
                return false; //Cancel show next, to allow the user to write more
            } else {
                return true; //Allow to continue
            }
        }
        
        
        else {
            return true;
        }
    } else {
        return false;
    }
}

bool BugzillaInformationPage::isComplete()
{
    return m_textsOK;
}

void BugzillaInformationPage::aboutToHide()
{
    //Save fields data
    reportInfo()->getReport()->setShortDescription( m_titleEdit->text() );
    reportInfo()->setDetailText( m_detailsEdit->toPlainText() );
}

//END BugzillaInformationPage

//BEGIN BugzillaSendPage

BugzillaSendPage::BugzillaSendPage( DrKonqiBugReport * parent )
    : DrKonqiAssistantPage(parent)
{
    connect( reportInfo()->getBZ(), SIGNAL(reportSent(int)), this, SLOT(sent(int)) );
    connect( reportInfo()->getBZ(), SIGNAL(sendReportError(QString)), this, SLOT(sendError(QString)) );
    connect( reportInfo()->getBZ(), SIGNAL(sendReportErrorWrongProduct()), this, SLOT(sendUsingDefaults()) );
    
    m_statusWidget = new StatusWidget();
    m_statusWidget->setStatusLabelWordWrap( true );
    
    m_retryButton = new KPushButton(  KGuiItem( i18nc("button action", "Retry ...") , KIcon("view-refresh"),  i18nc("help text", "Use this button to retry sending the crash report if it failed before"), i18nc("help text", "Use this button to retry sending the crash report if it failed before") ) );
    
    m_retryButton->setVisible( false );
    connect( m_retryButton, SIGNAL(clicked()), this , SLOT(retryClicked()) );
    
    QHBoxLayout * buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget( m_retryButton );
    
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( m_statusWidget );
    layout->addLayout( buttonLayout );
    layout->addStretch();
    setLayout( layout );
}

void BugzillaSendPage::retryClicked()
{
    m_retryButton->setEnabled( false );
    aboutToShow();
}

void BugzillaSendPage::aboutToShow()
{
    m_statusWidget->setBusy( i18n( "Sending crash report ... ( please wait )" ) );
    reportInfo()->fillReportFields();
    reportInfo()->sendBugReport();
}

void BugzillaSendPage::sent( int bug_id )
{
    m_statusWidget->setIdle( i18n("Crash report sent!<br />Bug Number :: %1<br />URL :: <link>%2</link><br />Thanks for contributing with KDE. You can close this window", bug_id, reportInfo()->getBZ()->urlForBug( bug_id ) )); //FIXME text
    
    m_retryButton->setEnabled( false );
    m_retryButton->setVisible( false );
    
    emit finished(false);
}

void BugzillaSendPage::sendError( QString errorString )
{
    m_statusWidget->setIdle( i18n( "Error sending the crash report:  %1", errorString ) );

    m_retryButton->setEnabled( true );
    m_retryButton->setVisible( true );
}

void BugzillaSendPage::sendUsingDefaults()
{
    reportInfo()->setDefaultProductComponent();
    reportInfo()->sendBugReport();
}

//END BugzillaSendPage

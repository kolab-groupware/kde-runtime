//////////////////////////////////////////////////////////////////////////////
// oxygenlineeditdata.cpp
// data container for QLineEdit transition
// -------------------
//
// Copyright (c) 2009 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//////////////////////////////////////////////////////////////////////////////

#include "oxygenlineeditdata.h"
#include "oxygenlineeditdata.moc"

#include <QtCore/QEvent>
#include <QtCore/QTextStream>
#include <QtGui/QStyle>
#include <QtGui/QStyleOptionFrameV2>

namespace Oxygen
{

    //______________________________________________________
    LineEditData::LineEditData( QObject* parent, QLineEdit* target, int duration ):
        TransitionData( parent, target, duration ),
        target_( target ),
        edited_( false )
    {
        target_.data()->installEventFilter( this );
        transition().data()->setFlags( TransitionWidget::GrabFromWindow );
        connect( target_.data(), SIGNAL( textEdited( const QString& ) ), SLOT( textEdited( const QString& ) ) );
        connect( target_.data(), SIGNAL( textChanged( const QString& ) ), SLOT( textChanged( const QString& ) ) );
        connect( target_.data(), SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) );
    }

    //___________________________________________________________________
    bool LineEditData::eventFilter( QObject* object, QEvent* event )
    {

        if( !( enabled() && object && object == target_ ) )
        { return TransitionData::eventFilter( object, event ); }

        switch ( event->type() )
        {
            case QEvent::Show:
            case QEvent::Resize:
            case QEvent::Move:
            timer_.start( 0, this );
            break;

            default: break;
        }

        return TransitionData::eventFilter( object, event );

    }

    //___________________________________________________________________
    void LineEditData::timerEvent( QTimerEvent* event )
    {
        if( event->timerId() == timer_.timerId() )
        {

            timer_.stop();
            if( target_ && target_.data()->isVisible() )
            { transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) ); }

        } else return QObject::timerEvent( event );

    }

    //___________________________________________________________________
    void LineEditData::textChanged( const QString& value )
    {

        // check wether text change was triggered manually
        // in which case do not start transition
        if( edited_ )
        {
            edited_ = false;
            return;
        }

        if( initializeAnimation() ) animate();

    }

    //___________________________________________________________________
    bool LineEditData::initializeAnimation( void )
    {
        if( !( enabled() && target_ && target_.data()->isVisible() ) ) return false;
        transition().data()->setOpacity(0);
        transition().data()->setGeometry( targetRect() );
        transition().data()->setStartPixmap( transition().data()->endPixmap() );
        bool valid( !transition().data()->startPixmap().isNull() );

        if( valid )
        {
            transition().data()->show();
            transition().data()->raise();
        }

        transition().data()->setEndPixmap( transition().data()->grab( target_.data(), targetRect() ) );
        return valid;

    }

    //___________________________________________________________________
    bool LineEditData::animate( void )
    {
        transition().data()->animate();
        return true;
    }

}
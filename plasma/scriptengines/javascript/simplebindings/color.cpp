/*
 *   Copyright 2009 Aaron J. Seigo
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtGui/QColor>
#include "../backportglobal.h"

Q_DECLARE_METATYPE(QColor*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return qScriptValueFromValue(eng, QColor());
    } else if (ctx->argumentCount() == 1) {
        QString namedColor = ctx->argument(0).toString();
        return qScriptValueFromValue(eng, QColor(namedColor));
    }

    int r = 0;
    int g = 0;
    int b = 0;
    int a = 255;
    if (ctx->argumentCount() == 3) {
        r = ctx->argument(0).toInt32();
        g = ctx->argument(1).toInt32();
        b = ctx->argument(2).toInt32();
    }

    if (ctx->argumentCount() == 4) {
        a = ctx->argument(3).toInt32();
    }

    return qScriptValueFromValue(eng, QColor(r, g, b, a));
}

// red, green, blue, alpha, setRed, setGreen, setBlue, setAlpha, isValid, toString, name

static QScriptValue red(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, red);
    return QScriptValue(eng, self->red());
}

static QScriptValue setRed(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setRed);
    QScriptValue arg = ctx->argument(0);
    self->setRed(arg.toInt32());
    return arg;
}

static QScriptValue green(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, green);
    return QScriptValue(eng, self->green());
}

static QScriptValue setGreen(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setGreen);
    QScriptValue arg = ctx->argument(0);
    self->setGreen(arg.toInt32());
    return arg;
}

static QScriptValue blue(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, blue);
    return QScriptValue(eng, self->blue());
}

static QScriptValue setBlue(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setBlue);
    QScriptValue arg = ctx->argument(0);
    self->setBlue(arg.toInt32());
    return arg;
}

static QScriptValue alpha(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, alpha);
    return QScriptValue(eng, self->alpha());
}

static QScriptValue setAlpha(QScriptContext *ctx, QScriptEngine *)
{
    DECLARE_SELF(QColor, setAlpha);
    QScriptValue arg = ctx->argument(0);
    self->setAlpha(arg.toInt32());
    return arg;
}

static QScriptValue isValid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QColor, isValid);
    return QScriptValue(eng, self->isValid());
}

QScriptValue constructColorClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QColor());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("red", eng->newFunction(red), getter);
    proto.setProperty("setRed", eng->newFunction(setRed));
    proto.setProperty("green", eng->newFunction(green), getter);
    proto.setProperty("setGreen", eng->newFunction(setGreen));
    proto.setProperty("blue", eng->newFunction(blue), getter);
    proto.setProperty("setBlue", eng->newFunction(setBlue));
    proto.setProperty("alpha", eng->newFunction(alpha, getter));
    proto.setProperty("setAlpha", eng->newFunction(setAlpha));
    proto.setProperty("isValid", eng->newFunction(red), getter);

    eng->setDefaultPrototype(qMetaTypeId<QColor>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QColor*>(), proto);

    return eng->newFunction(ctor, proto);
}
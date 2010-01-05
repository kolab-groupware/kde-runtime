/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef JAVASCRIPTBINDI18N_H
#define JAVASCRIPTBINDI18N_H

QScriptValue jsi18n(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1) {
        return context->throwError(i18n("i18n() takes at least one argument"));
    }

    KLocalizedString message = ki18n(context->argument(0).toString().toUtf8());

    int numArgs = context->argumentCount();
    for (int i = 1; i < numArgs; ++i) {
        message.subs(context->argument(i).toString());
    }

    return engine->newVariant(message.toString());
}

QScriptValue jsi18nc(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 2) {
        return context->throwError(i18n("i18nc() takes at least two arguments"));
    }

    KLocalizedString message = ki18nc(context->argument(0).toString().toUtf8(),
                                      context->argument(1).toString().toUtf8());

    int numArgs = context->argumentCount();
    for (int i = 2; i < numArgs; ++i) {
        message.subs(context->argument(i).toString());
    }

    return engine->newVariant(message.toString());
}

QScriptValue jsi18np(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 2) {
        return context->throwError(i18n("i18np() takes at least two arguments"));
    }

    KLocalizedString message = ki18np(context->argument(0).toString().toUtf8(),
                                      context->argument(1).toString().toUtf8());

    int numArgs = context->argumentCount();
    for (int i = 2; i < numArgs; ++i) {
        message.subs(context->argument(i).toString());
    }

    return engine->newVariant(message.toString());
}

QScriptValue jsi18ncp(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 3) {
        return context->throwError(i18n("i18ncp() takes at least three arguments"));
    }

    KLocalizedString message = ki18ncp(context->argument(0).toString().toUtf8(),
                                       context->argument(1).toString().toUtf8(),
                                       context->argument(2).toString().toUtf8());

    int numArgs = context->argumentCount();
    for (int i = 3; i < numArgs; ++i) {
        message.subs(context->argument(i).toString());
    }

    return engine->newVariant(message.toString());
}

void bindI18N(QScriptEngine *engine)
{
    QScriptValue global = engine->globalObject();
    global.setProperty("i18n", engine->newFunction(jsi18n));
    global.setProperty("i18nc", engine->newFunction(jsi18nc));
    global.setProperty("i18np", engine->newFunction(jsi18np));
    global.setProperty("i18ncp", engine->newFunction(jsi18ncp));
}

#endif

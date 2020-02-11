/* ============================================================
* Falkon - Qt web browser
* Copyright (C) 2015-2018 David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "scripts.h"
#include "qztools.h"
#include "webpage.h"

#include <QUrlQuery>
#include <QtWebEngineWidgetsVersion>
#include <iostream>

QString Scripts::setupWebChannel()
{
    QString source =  QL1S("(function() {"
                           "%1"
                           ""
                           "function registerExternal(e) {"
                           "    window.external = e;"
                           "    if (window.external) {"
                           "        var event = document.createEvent('Event');"
                           "        event.initEvent('_falkon_external_created', true, true);"
                           "        window._falkon_external = true;"
                           "        document.dispatchEvent(event);"
                           "    }"
                           "}"
                           ""
                           "if (self !== top) {"
                           "    if (top._falkon_external)"
                           "        registerExternal(top.external);"
                           "    else"
                           "        top.document.addEventListener('_falkon_external_created', function() {"
                           "            registerExternal(top.external);"
                           "        });"
                           "    return;"
                           "}"
                           ""
                           "function registerWebChannel() {"
                           "    try {"
                           "        new QWebChannel(qt.webChannelTransport, function(channel) {"
                           "            var external = channel.objects.qz_object;"
                           "            external.extra = {};"
                           "            for (var key in channel.objects) {"
                           "                if (key != 'qz_object' && key.startsWith('qz_')) {"
                           "                    external.extra[key.substr(3)] = channel.objects[key];"
                           "                }"
                           "            }"
                           "            registerExternal(external);"
                           "        });"
                           "    } catch (e) {"
                           "        setTimeout(registerWebChannel, 100);"
                           "    }"
                           "}"
                           "registerWebChannel();"
                           ""
                           "})()");

    return source.arg(QzTools::readAllFileContents(QSL(":/qtwebchannel/qwebchannel.js")));
}

QString Scripts::setupFormObserver()
{
    QString source = QL1S("(function() {"
                          "function findUsername(inputs) {"
                          "    var usernameNames = ['user', 'name', 'login'];"
                          "    for (var i = 0; i < usernameNames.length; ++i) {"
                          "        for (var j = 0; j < inputs.length; ++j)"
                          "            if (inputs[j].type == 'text' && inputs[j].value.length && inputs[j].name.indexOf(usernameNames[i]) != -1)"
                          "                return inputs[j].value;"
                          "    }"
                          "    for (var i = 0; i < inputs.length; ++i)"
                          "        if (inputs[i].type == 'text' && inputs[i].value.length)"
                          "            return inputs[i].value;"
                          "    for (var i = 0; i < inputs.length; ++i)"
                          "        if (inputs[i].type == 'email' && inputs[i].value.length)"
                          "            return inputs[i].value;"
                          "    return '';"
                          "}"
                          ""
                          "function registerForm(form) {"
                          "    form.addEventListener('submit', function() {"
                          "        var form = this;"
                          "        var data = '';"
                          "        var password = '';"
                          "        var inputs = form.getElementsByTagName('input');"
                          "        for (var i = 0; i < inputs.length; ++i) {"
                          "            var input = inputs[i];"
                          "            var type = input.type.toLowerCase();"
                          "            if (type != 'text' && type != 'password' && type != 'email')"
                          "                continue;"
                          "            if (!password && type == 'password')"
                          "                password = input.value;"
                          "            data += encodeURIComponent(input.name);"
                          "            data += '=';"
                          "            data += encodeURIComponent(input.value);"
                          "            data += '&';"
                          "        }"
                          "        if (!password)"
                          "            return;"
                          "        data = data.substring(0, data.length - 1);"
                          "        var url = window.location.href;"
                          "        var username = findUsername(inputs);"
                          "        external.autoFill.formSubmitted(url, username, password, data);"
                          "    }, true);"
                          "}"
                          ""
                          "if (!document.documentElement) return;"
                          ""
                          "for (var i = 0; i < document.forms.length; ++i)"
                          "    registerForm(document.forms[i]);"
                          ""
                          "var observer = new MutationObserver(function(mutations) {"
                          "    for (var mutation of mutations)"
                          "        for (var node of mutation.addedNodes)"
                          "            if (node.tagName && node.tagName.toLowerCase() == 'form')"
                          "                registerForm(node);"
                          "});"
                          "observer.observe(document.documentElement, { childList: true, subtree: true });"
                          ""
                          "})()");

    return source;
}

QString Scripts::setupWindowObject()
{
    QString source = QL1S("(function() {"
                          "var external = {};"
                          "external.AddSearchProvider = function(url) {"
                          "    window.location = 'falkon:AddSearchProvider?url=' + url;"
                          "};"
                          "external.IsSearchProviderInstalled = function(url) {"
                          "    console.warn('NOT IMPLEMENTED: IsSearchProviderInstalled()');"
                          "    return false;"
                          "};"
                          "window.external = external;");
#if QTWEBENGINEWIDGETS_VERSION < QT_VERSION_CHECK(5, 12, 0)
           source += QL1S("window.print = function() {"
                          "    window.location = 'falkon:PrintPage';"
                          "};");
#endif
           source += QL1S("})()");

    return source;
}

QString Scripts::setupSpeedDial()
{
    QString source = QzTools::readAllFileContents(QSL(":html/speeddial.user.js"));
    source.replace(QL1S("%JQUERY%"), QzTools::readAllFileContents(QSL(":html/jquery.js")));
    source.replace(QL1S("%JQUERY-UI%"), QzTools::readAllFileContents(QSL(":html/jquery-ui.js")));
    return source;
}

QString Scripts::setCss(const QString &css)
{
    QString source = QL1S("(function() {"
                          "var head = document.getElementsByTagName('head')[0];"
                          "if (!head) return;"
                          "var css = document.createElement('style');"
                          "css.setAttribute('type', 'text/css');"
                          "css.appendChild(document.createTextNode('%1'));"
                          "head.appendChild(css);"
                          "})()");

    QString style = css;
    style.replace(QL1S("'"), QL1S("\\'"));
    style.replace(QL1S("\n"), QL1S("\\n"));
    return source.arg(style);
}

QString Scripts::sendPostData(const QUrl &url, const QByteArray &data)
{
    QString source = QL1S("(function() {"
                          "var form = document.createElement('form');"
                          "form.setAttribute('method', 'POST');"
                          "form.setAttribute('action', '%1');"
                          "var val;"
                          "%2"
                          "form.submit();"
                          "})()");

    QString valueSource = QL1S("val = document.createElement('input');"
                               "val.setAttribute('type', 'hidden');"
                               "val.setAttribute('name', '%1');"
                               "val.setAttribute('value', '%2');"
                               "form.appendChild(val);");

    QString values;
    QUrlQuery query(data);

    const auto &queryItems = query.queryItems(QUrl::FullyDecoded);
    for (int i = 0; i < queryItems.size(); ++i) {
        const auto &pair = queryItems[i];
        QString value = pair.first;
        QString key = pair.second;
        value.replace(QL1S("'"), QL1S("\\'"));
        key.replace(QL1S("'"), QL1S("\\'"));
        values.append(valueSource.arg(value, key));
    }

    return source.arg(url.toString(), values);
}

QString Scripts::completeFormData(const QByteArray &data)
{
    QString source = QL1S("(function() {"
                          "var data = '%1'.split('&');"
                          "var inputs = document.getElementsByTagName('input');"
                          ""
                          "for (var i = 0; i < data.length; ++i) {"
                          "    var pair = data[i].split('=');"
                          "    if (pair.length != 2)"
                          "        continue;"
                          "    var key = decodeURIComponent(pair[0]);"
                          "    var val = decodeURIComponent(pair[1]);"
                          "    for (var j = 0; j < inputs.length; ++j) {"
                          "        var input = inputs[j];"
                          "        var type = input.type.toLowerCase();"
                          "        if (type != 'text' && type != 'password' && type != 'email')"
                          "            continue;"
                          "        if (input.name == key) {"
                          "            input.value = val;"
                          "            input.dispatchEvent(new Event('change'));"
                          "        }"
                          "    }"
                          "}"
                          ""
                          "})()");

    QString d = data;
    d.replace(QL1S("'"), QL1S("\\'"));
    return source.arg(d);
}

QString Scripts::getOpenSearchLinks()
{
    QString source = QL1S("(function() {"
                          "var out = [];"
                          "var links = document.getElementsByTagName('link');"
                          "for (var i = 0; i < links.length; ++i) {"
                          "    var e = links[i];"
                          "    if (e.type == 'application/opensearchdescription+xml') {"
                          "        out.push({"
                          "            url: e.href,"
                          "            title: e.title"
                          "        });"
                          "    }"
                          "}"
                          "return out;"
                          "})()");

    return source;
}

QString Scripts::getAllImages()
{
    QString source = QL1S("(function() {"
                          "var out = [];"
                          "var imgs = document.getElementsByTagName('img');"
                          "for (var i = 0; i < imgs.length; ++i) {"
                          "    var e = imgs[i];"
                          "    out.push({"
                          "        src: e.src,"
                          "        alt: e.alt"
                          "    });"
                          "}"
                          "return out;"
                          "})()");

    return source;
}

QString Scripts::getAllMetaAttributes()
{
    QString source = QL1S("(function() {"
                          "var out = [];"
                          "var meta = document.getElementsByTagName('meta');"
                          "for (var i = 0; i < meta.length; ++i) {"
                          "    var e = meta[i];"
                          "    out.push({"
                          "        name: e.getAttribute('name'),"
                          "        content: e.getAttribute('content'),"
                          "        httpequiv: e.getAttribute('http-equiv')"
                          "    });"
                          "}"
                          "return out;"
                          "})()");

    return source;
}

QString Scripts::getFormData(const QPointF &pos)
{
    QString source = QL1S("(function() {"
                          "var e = document.elementFromPoint(%1, %2);"
                          "if (!e || e.tagName.toLowerCase() != 'input')"
                          "    return;"
                          "var fe = e.parentElement;"
                          "while (fe) {"
                          "    if (fe.tagName.toLowerCase() == 'form')"
                          "        break;"
                          "    fe = fe.parentElement;"
                          "}"
                          "if (!fe)"
                          "    return;"
                          "var res = {"
                          "    method: fe.method.toLowerCase(),"
                          "    action: fe.action,"
                          "    inputName: e.name,"
                          "    inputs: [],"
                          "};"
                          "for (var i = 0; i < fe.length; ++i) {"
                          "    var input = fe.elements[i];"
                          "    res.inputs.push([input.name, input.value]);"
                          "}"
                          "return res;"
                          "})()");

    return source.arg(pos.x()).arg(pos.y());
}

QString Scripts::scrollToAnchor(const QString &anchor)
{
    QString source = QL1S("(function() {"
                          "var e = document.getElementById(\"%1\");"
                          "if (!e) {"
                          "    var els = document.querySelectorAll(\"[name='%1']\");"
                          "    if (els.length)"
                          "        e = els[0];"
                          "}"
                          "if (e)"
                          "    e.scrollIntoView();"
                          "})()");

    return source.arg(anchor);
}

QString Scripts::fillInLoginData(const QString &username, const QString &password) {
  // code borrowed from https://github.com/gopasspw/gopassbridge/blob/00e83b90648027a7d0b8227b0f0a2ff33fdb1bef/web-extension/content.js

  QString source = QL1S(
      "(function() { \n"
      " \n"
      "const inputEventNames = ['click', 'focus', 'keypress', 'keydown', 'keyup', 'input', 'blur', 'change'], \n"
      "    loginInputIds = [ \n"
      "        'username', \n"
      "        'user_name', \n"
      "        'userid', \n"
      "        'user_id', \n"
      "        'user', \n"
      "        'login', \n"
      "        'email', \n"
      "        'login_field', \n"
      "        'login-form-username', \n"
      "    ], \n"
      "    ignorePasswordIds = ['signup_minireg_password'], \n"
      "    loginInputTypes = ['email', 'text'], \n"
      "    loginInputTypesString = loginInputTypes.map(string => `input[type=${string}]`).join(',') + ',input:not([type])'; \n"
      " \n"
      "function exactMatch(property, string) { \n"
      "    const idstr = `[${property}=${string}]`; \n"
      "    return loginInputTypes.map(string => `input[type=${string}]${idstr}`).join(',') + `,input:not([type])${idstr}`; \n"
      "} \n"
      " \n"
      "function partialMatch(property, string) { \n"
      "    const idstr = `[${property}*=${string}]`; \n"
      "    return ( \n"
      "        loginInputTypes \n"
      "            .map(function(string) { \n"
      "                return `input[type=${string}]${idstr}`; \n"
      "            }) \n"
      "            .join(',') + \n"
      "        ',input:not([type])' + \n"
      "        idstr \n"
      "    ); \n"
      "} \n"
      " \n"
      "const exactLoginInputIdString = loginInputIds.map(exactMatch.bind(null, 'id')).join(','), \n"
      "    partialLoginInputIdString = loginInputIds.map(partialMatch.bind(null, 'id')).join(','), \n"
      "    exactLoginInputNameString = loginInputIds.map(exactMatch.bind(null, 'name')).join(','), \n"
      "    partialLoginInputNameString = loginInputIds.map(partialMatch.bind(null, 'name')).join(','), \n"
      "    allLoginInputStrings = [ \n"
      "        exactLoginInputIdString, \n"
      "        partialLoginInputIdString, \n"
      "        exactLoginInputNameString, \n"
      "        partialLoginInputNameString, \n"
      "        loginInputTypesString, \n"
      "    ], \n"
      "    allLoginInputStringsJoined = allLoginInputStrings.join(','); \n"
      " \n"
      "function isVisible(element) { \n"
      "    const elementStyle = window.getComputedStyle(element); \n"
      "    if (element.offsetWidth < 30) { \n"
      "        return false; \n"
      "    } \n"
      "    if (element.offsetHeight < 10) { \n"
      "        return false; \n"
      "    } \n"
      "    return elementStyle.visibility !== 'hidden'; \n"
      "} \n"
      " \n"
      "function selectFocusedElement(parent) { \n"
      "    parent = parent || document; \n"
      "    if ( \n"
      "        parent.body === parent.activeElement || \n"
      "        parent.activeElement.tagName === 'IFRAME' || \n"
      "        parent.activeElement.tagName === 'FRAME' \n"
      "    ) { \n"
      "        let focusedElement = null; \n"
      "        parent.querySelectorAll('iframe,frame').forEach(iframe => { \n"
      "            if (iframe.src.startsWith(window.location.origin)) { \n"
      "                const focused = selectFocusedElement(iframe.contentWindow.document); \n"
      "                if (focused) { \n"
      "                    focusedElement = focused; \n"
      "                } \n"
      "            } \n"
      "        }); \n"
      "        return focusedElement; \n"
      "    } else { \n"
      "        return parent.activeElement; \n"
      "    } \n"
      "} \n"
      " \n"
      "function selectVisibleElements(selector) { \n"
      "    const visibleElements = []; \n"
      " \n"
      "    document.querySelectorAll(selector).forEach(element => { \n"
      "        if (isVisible(element)) { \n"
      "            visibleElements.push(element); \n"
      "        } \n"
      "    }); \n"
      " \n"
      "    document.querySelectorAll('iframe,frame').forEach(iframe => { \n"
      "        if (iframe.src.startsWith(window.location.origin)) { \n"
      "            iframe.contentWindow.document.body.querySelectorAll(selector).forEach(element => { \n"
      "                if (isVisible(element)) { \n"
      "                    visibleElements.push(element); \n"
      "                } \n"
      "            }); \n"
      "        } \n"
      "    }); \n"
      " \n"
      "    return visibleElements; \n"
      "} \n"
      " \n"
      "function selectFirstVisiblePasswordElement(selector) { \n"
      "    for (let element of selectVisibleElements(selector)) { \n"
      "        if ( \n"
      "            ignorePasswordIds.every(ignore => { \n"
      "                return element.id !== ignore; \n"
      "            }) \n"
      "        ) { \n"
      "            return element; \n"
      "        } \n"
      "    } \n"
      " \n"
      "    return null; \n"
      "} \n"
      " \n"
      "function selectFirstVisibleFormElement(form, selector, afterTabInd) { \n"
      "    for (let element of selectVisibleElements(selector)) { \n"
      "        if (element && form === element.form && (afterTabInd === undefined || element.tabIndex > afterTabInd)) { \n"
      "            return element; \n"
      "        } \n"
      "    } \n"
      " \n"
      "    return null; \n"
      "} \n"
      " \n"
      "function updateElement(element, newValue) { \n"
      "    if (!newValue.length) { \n"
      "        return false; \n"
      "    } \n"
      "    element.setAttribute('value', newValue); \n"
      "    element.value = newValue; \n"
      " \n"
      "    inputEventNames.forEach(name => { \n"
      "        element.dispatchEvent(new Event(name, { bubbles: true })); \n"
      "        // Some sites clear the fields on certain events, refill to make sure that values are in the field are set \n"
      "        element.setAttribute('value', newValue); \n"
      "        element.value = newValue; \n"
      "    }); \n"
      "    return true; \n"
      "} \n"
      " \n"
      "function getLoginInputFromPasswordInputForm(passwordInputForm) { \n"
      "    for (let loginInput of allLoginInputStrings) { \n"
      "        const element = selectFirstVisibleFormElement(passwordInputForm, loginInput); \n"
      "        if (element) return element; \n"
      "    } \n"
      "} \n"
      " \n"
      "function _determineFieldsFromFocusedInput(focusedInput) { \n"
      "    let passwordInput, loginInput; \n"
      "    if (focusedInput.type === 'password') { \n"
      "        passwordInput = focusedInput; \n"
      "    } else if (focusedInput.matches(allLoginInputStringsJoined)) { \n"
      "        passwordInput = \n"
      "            selectFirstVisibleFormElement(focusedInput.form, 'input[type=password]', focusedInput.tabIndex) || \n"
      "            selectFirstVisibleFormElement(focusedInput.form, 'input[type=password]'); \n"
      "        if (passwordInput) { \n"
      "            loginInput = focusedInput; \n"
      "        } \n"
      "    } \n"
      "    return { passwordInput, loginInput }; \n"
      "} \n"
      " \n"
      "function getInputFieldsFromFocus() { \n"
      "    let focusedInput = selectFocusedElement(document); \n"
      "    if (focusedInput && focusedInput.tagName === 'INPUT') { \n"
      "        return _determineFieldsFromFocusedInput(focusedInput); \n"
      "    } \n"
      "    return { \n"
      "        loginInput: undefined, \n"
      "        passwordInput: undefined, \n"
      "    }; \n"
      "} \n"
      " \n"
      "function _getInputFieldsFromPasswordInput(passwordInput) { \n"
      "    const loginInput = getLoginInputFromPasswordInputForm(passwordInput.form); \n"
      "    if (loginInput && loginInput.tabIndex > passwordInput.tabIndex) { \n"
      "        const matchingPasswordInput = selectFirstVisibleFormElement( \n"
      "            loginInput.form, \n"
      "            'input[type=password]', \n"
      "            loginInput.tabIndex \n"
      "        ); \n"
      "        passwordInput = matchingPasswordInput || passwordInput; \n"
      "    } \n"
      "    return { login: loginInput, password: passwordInput }; \n"
      "} \n"
      " \n"
      "function getInputFields() { \n"
      "    const focusedInputs = getInputFieldsFromFocus(); \n"
      "    let loginInput = focusedInputs.loginInput; \n"
      "    let passwordInput = focusedInputs.passwordInput || selectFirstVisiblePasswordElement('input[type=password]'); \n"
      " \n"
      "    if (passwordInput && passwordInput.form && !focusedInputs.loginInput) { \n"
      "        return _getInputFieldsFromPasswordInput(passwordInput); \n"
      "    } \n"
      " \n"
      "    return { \n"
      "        login: loginInput, \n"
      "        password: passwordInput, \n"
      "    }; \n"
      "} \n"
      " \n"
      "function markElement(element) { \n"
      "    element.style.border = '3px solid blue'; \n"
      "} \n"
      " \n"
      "function markLoginFields() { \n"
      "    const inputs = getInputFields(); \n"
      "    if (inputs.login) { \n"
      "        markElement(inputs.login); \n"
      "    } \n"
      "    if (inputs.password) { \n"
      "        markElement(inputs.password); \n"
      "    } \n"
      "} \n"
      " \n"
      "function updateInputFields(login, password) { \n"
      "    const inputs = getInputFields(); \n"
      "    if (inputs.login) { \n"
      "        updateElement(inputs.login, login); \n"
      "    } \n"
      "    if (inputs.password) { \n"
      "        updateElement(inputs.password, password); \n"
      "    } \n"
      "} \n"
      " \n"
      "function tryLogIn() { \n"
      "    const passwortInputs = selectVisibleElements('input[type=password]'); \n"
      "    if (passwortInputs.length > 1) { \n"
      "        passwortInputs[1].select(); \n"
      "    } else { \n"
      "        window.requestAnimationFrame(() => { \n"
      "            if (passwortInputs.length === 1 && passwortInputs[0].form) { \n"
      "                const submitButton = selectFirstVisibleFormElement(passwortInputs[0].form, '[type=submit]'); \n"
      "                if (submitButton) { \n"
      "                    submitButton.click(); \n"
      "                } \n"
      "            } \n"
      "        }); \n"
      "    } \n"
      "} \n"
      " \n"
      "updateInputFields(\"%1\", \"%2\"); \n"
      " \n"
      "})(); \n"
      );
  return source.arg(username, password);
}

#include "zigpass_plugin.h"
#include "zigpass_backend.h"
#include "pluginproxy.h"
#include "browserwindow.h"
#include "../config.h"
#include "mainapplication.h"
#include "autofill.h"
#include "passwordmanager.h"
#include "desktopfile.h"

ZigPassPlugin::ZigPassPlugin()
    : QObject()
    , m_backend(0)
{
}

void ZigPassPlugin::init(InitState state, const QString &settingsPath)
{
    Q_UNUSED(state);
    Q_UNUSED(settingsPath);

    m_backend = new ZigPassBackend;
    mApp->autoFill()->passwordManager()->registerBackend(QSL("ZigPassBackend"), m_backend);
}

void ZigPassPlugin::unload()
{
    mApp->autoFill()->passwordManager()->unregisterBackend(m_backend);
    delete m_backend;
}

bool ZigPassPlugin::testPlugin()
{
    // Require the version that the plugin was built with
    return (Qz::VERSION == QLatin1String(FALKON_VERSION));
}

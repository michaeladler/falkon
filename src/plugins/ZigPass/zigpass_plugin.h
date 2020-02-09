
#ifndef ZIGPASSPLUGIN_H
#define ZIGPASSPLUGIN_H

#include "plugininterface.h"

class ZigPassBackend;

class ZigPassPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "Falkon.Browser.plugin.ZigPass" FILE "zigpass.json")

public:
    explicit ZigPassPlugin();

    void init(InitState state, const QString &settingsPath) override;
    void unload() override;
    bool testPlugin() override;

private:
    ZigPassBackend* m_backend;

};

#endif // ZIGPASSPLUGIN_H

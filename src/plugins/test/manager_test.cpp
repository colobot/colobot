#include <common/logger.h>
#include <common/profile.h>
#include <common/iman.h>
#include <plugins/pluginmanager.h>
#include <sound/sound.h>


int main() {
    new CLogger();
    new CProfile();
    new CInstanceManager();
    CPluginManager *mgr = new CPluginManager();

    if (!GetProfile()->InitCurrentDirectory()) {
        GetLogger()->Error("Config not found!\n");
        return 1;
    }
    mgr->LoadFromProfile();
    CSoundInterface *sound = static_cast<CSoundInterface*>(CInstanceManager::GetInstancePointer()->SearchInstance(CLASS_SOUND));
    sound->Create(true);
    mgr->UnloadAllPlugins();

    return 0;
}

#include <string>
#include <cstdio>
#include <unistd.h>

#include <common/logger.h>
#include <common/iman.h>
#include <sound/sound.h>
#include <plugins/pluginloader.h>


int main() {
    new CLogger();
    new CInstanceManager();

    lt_dlinit();

    CPluginLoader *plugin = new CPluginLoader("libopenalsound");
    if (plugin->LoadPlugin()) {
        CSoundInterface *sound = static_cast<CSoundInterface*>(CInstanceManager::GetInstancePointer()->SearchInstance(CLASS_SOUND));

        sound->Create(true);
        sound->CacheAll();
        sound->Play((Sound)8);
        sound->Play((Sound)18);

        sleep(10);
        /*
        while (1)
        {
            // just a test, very slow
            plugin->FrameMove(0);
            //if ('n' == getchar())
            //    break;
        }*/
        plugin->UnloadPlugin();
    }

    lt_dlexit();
    return 0;
}

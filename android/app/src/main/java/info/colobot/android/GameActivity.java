package info.colobot.android;

import org.libsdl.app.SDLActivity;

public class GameActivity extends SDLActivity
{
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "hidapi",
            "colobot"
        };
    }
}

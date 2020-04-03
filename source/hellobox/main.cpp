#include <FL/fl_ask.H>

auto main()
    -> int
{
    fl_message_title( "An FLTK message box" );
    fl_message( "%s\n", "Hello from FLTK!\nJust press the OK button, please." );
}

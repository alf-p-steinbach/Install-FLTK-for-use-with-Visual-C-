#include <string>
using   std::string, std::to_string;

#include <FL/Fl.H>              // “All programs must include the file <FL/Fl.H>.”

#include <FL/Fl_Box.H>          // Fl_Box
#include <FL/Fl_draw.H>         // drawing functions
#include <FL/Fl_Menu_Item.H>    // Fl_Menu_Item
#include <FL/Fl_Menu_Bar.H>     // Fl_Menu_Bar
#include <FL/Fl_Window.H>       // Fl_Window

using C_str = const char*;

class Main_window:
    public Fl_Window
{
    ~Main_window() override = default;      // Prevents direct instantiation. Not perfect.

    static void callback_for_command_quit( Fl_Widget* menubar, void* )
    {
        menubar->top_window()->hide();      // Destroys window and terminates event loop.
    }

    static auto default_menuitems()
        -> const Fl_Menu_Item*
    {
        static const Fl_Menu_Item the_items[] =
        {
            { "&App", 0, 0, 0, FL_SUBMENU },
                { "E&xit", FL_COMMAND + 'q', &callback_for_command_quit },
                { 0 },
            { 0 }
        };
        return the_items;
    }

    class Client_area:
        public Fl_Box
    {
        void draw() override
        {
            fl_push_clip( x(), y(), w(), h() );
            {
                const string text = string()
                    + "Hello, world! 😃\n"
                    + "This client area is " + to_string( w() ) + "×" + to_string( h() ) + " pixels.";
                fl_color( FL_RED );
                const int stroke_width = 12;
                fl_line_style( 0, stroke_width, nullptr );
                fl_arc( x(), y(), w(), h(), 0.0, 360.0 );
                fl_color( FL_BLACK );
                fl_draw( text.c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER, nullptr, 0 );
            }
            fl_pop_clip();
        }
        
    public:
        using Fl_Box::Fl_Box;
    };

    Fl_Menu_Bar*    m_menu_bar;
    Client_area*    m_client_area;

public:
    Main_window( const int width, const int height, const C_str title ):
        Fl_Window( width, height )
    {
        copy_label( title );

        m_menu_bar = new Fl_Menu_Bar( 0, 0, w(), 22 );
        m_menu_bar->copy( default_menuitems() );
        
        m_client_area = new Client_area( 0, m_menu_bar->h(), w(), h() - m_menu_bar->h(), "" );
        resizable( m_client_area );     // Omit this call to make the /window/ fixed size.

        end();  // End of being the default parent for new widgets.
    }
};

auto main()
    -> int
{
    (new Main_window( 340, 180, "FLTK dynamic ellipse" ))->show();
    return Fl::run();
}

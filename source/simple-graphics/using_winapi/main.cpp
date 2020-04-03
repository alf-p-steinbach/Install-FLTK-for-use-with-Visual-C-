#include <assert.h>

#include <functional>
#include <string>
#include <string_view>
using   std::invoke,
        std::string, std::to_string, std::wstring,
        std::string_view, std::wstring_view;

#include "../../winapi.utf8.hpp"

namespace winapi {
    auto default_ui_font()
        -> HFONT
    {
        static const HFONT the_font = invoke( []() -> HFONT
        {
            NONCLIENTMETRICS metrics = { sizeof( metrics ) };
            if( not SystemParametersInfo(
                SPI_GETNONCLIENTMETRICS, sizeof( metrics ), &metrics, 0 )
                ) {
                return 0;
            }
            return CreateFontIndirect( &metrics.lfMessageFont );
        } );
        
        return the_font;
    }

    auto as_utf16( const string_view& utf8_string )
        -> wstring
    {
        if( utf8_string.empty() ) {
            return L"";
        }

        const int buf_size = MultiByteToWideChar(
            CP_UTF8, 0, utf8_string.data(), utf8_string.size(), nullptr, 0
            );
        auto result = wstring( buf_size, L'\0' );
        const int n_codes_written = MultiByteToWideChar(
            CP_UTF8, 0, utf8_string.data(), utf8_string.size(), &result[0], buf_size
            );
        assert( n_codes_written != 0 );
        return result;
    }

    void draw_text(
        const HDC               dc,
        const wstring_view&     s,
        const RECT&             r,
        const UINT              options
        )
    {
        RECT draw_rect = r;
        if( options & DT_VCENTER ) {
            const int text_height = DrawTextW(
                dc, s.data(), s.size(), &draw_rect, options| DT_CALCRECT
                );
            draw_rect.left = r.left;
            draw_rect.right = r.right;
            draw_rect.top = r.top + ((r.bottom - r.top) - text_height)/2;   // Center it.
            draw_rect.bottom = r.bottom;
        } 
        DrawTextW( dc, s.data(), s.size(), &draw_rect, options );
    }

    void draw_text( const HDC dc, const string_view& s, const RECT& r, const UINT options )
    {
        // DrawTextA doesn't work properly with UTF-8 process codepage as of April 2020.
        draw_text( dc, as_utf16( s ), r, options );
    }
}  // namespace winapi

void paint( const HWND window, const HDC dc )
{
    RECT r_values;
    GetClientRect( window, &r_values );
    const RECT& r = r_values;

    const string s = string()
        + "Hello, world! 😃\n"
        + "This client area is " + to_string( r.right ) + "×" + to_string( r.bottom ) + " pixels.";
    const int stroke_width = 12;
    const HPEN wide_pen = CreatePen( PS_SOLID, stroke_width, RGB( 0xFF, 0, 0 ) );
    const int dc_state_id = SaveDC( dc );
    {
        SelectObject( dc, GetStockObject( NULL_BRUSH ) );       // For the ellipse.
        SetBkMode( dc,TRANSPARENT );                            // For the text drawing.
        
        SelectObject( dc, wide_pen );
        Ellipse( dc, r.left, r.top, r.right, r.bottom );
        
        SelectObject( dc, winapi::default_ui_font() );
        winapi::draw_text( dc, s, r, DT_CENTER | DT_VCENTER | DT_NOPREFIX );
    }
    RestoreDC( dc, dc_state_id );
    DeleteObject( wide_pen );
}

namespace command {
    const int   exit    = 100;
}  // namespace command

void on_command( const HWND window, const int id )
{
    switch( id ) {
        case command::exit: {
            PostMessage( window, WM_CLOSE, 0, 0 );
        }
    }
}

void on_wm_command(
    const HWND      window,
    const int       command_id,
    const HWND      control,
    const UINT      notification
    )
{
    if( control != 0 ) {
        (void) notification;        // Not using any controls in this app.
    } else {
        on_command( window, command_id );
    }
}

void on_wm_close( const HWND window )
{
    EndDialog( window, EXIT_SUCCESS );
}

auto on_wm_initdialog( const HWND window, HWND, LPARAM )
    -> BOOL
{
    const HMENU app_menu = CreatePopupMenu();
    AppendMenu( app_menu, MF_ENABLED, command::exit, "E&xit\tAlt+F4" );

    const HMENU menu_bar = CreateMenu();
    AppendMenu( menu_bar, MF_POPUP, UINT_PTR( app_menu ), "&App" );

    SetMenu( window, menu_bar );
    
    // SetWindowFont( window, winapi::default_ui_font(), false );   // Doesn't work. :-(
    return true;
}

void on_wm_paint( const HWND window )
{
    PAINTSTRUCT params;
    if( BeginPaint( window, &params ) ) {
        paint( window, params.hdc );
        EndPaint( window, &params );
    }
}

void on_wm_size( const HWND window, const UINT state, const int w, const int h )
{
    (void) w; (void) h;
    if( state == SIZE_RESTORED ) {
        InvalidateRect( window, nullptr, true );
    }
}

auto WINAPI message_handler(
    const HWND      window,
    const UINT      message_id,
    const WPARAM    w_param,
    const LPARAM    ell_param
    ) -> INT_PTR
{
    #define INVOKE( m, f ) m( window, w_param, ell_param, f )
    switch( message_id ) {
        case WM_CLOSE:      return INVOKE( HANDLE_WM_CLOSE, on_wm_close );
        case WM_COMMAND:    return INVOKE( HANDLE_WM_COMMAND, on_wm_command );
        case WM_INITDIALOG: return INVOKE( HANDLE_WM_INITDIALOG, on_wm_initdialog );
        case WM_PAINT:      return INVOKE( HANDLE_WM_PAINT, on_wm_paint );
        case WM_SIZE:       return INVOKE( HANDLE_WM_SIZE, on_wm_size );
    }
    #undef INVOKE
    return false;
}

struct Window_spec
{
    // A memory layout required by the DialogBoxIndirect function.
    DLGTEMPLATE     dialog          = {};
    WORD            menu            = 0;
    WORD            window_class    = 0;
    wchar_t         title[80]       = L"Windows API dynamic ellipse";

    Window_spec()
    {
        dialog.style            = DS_CENTER | WS_OVERLAPPEDWINDOW;
        dialog.dwExtendedStyle  = 0;
        dialog.cdit             = 0;
        dialog.x                = 0;
        dialog.y                = 0;
        dialog.cx               = 340/2;    // Dialog units, sort of like em, not pixels.
        dialog.cy               = 180/2;

        assert( not (dialog.style & DS_SETFONT) );  // Requires different struct (!).
    }
};

auto main()
    -> int
{
    const Window_spec spec;
    DialogBoxIndirect( GetModuleHandle( 0 ), &spec.dialog, HWND(), message_handler );
}

#include "main_menu.h"

#include "auto_pickup.h"
#include "avatar.h"
#include "cata_utility.h"
#include "catacharset.h"
#include "catalua.h"
#include "character_id.h"
#include "color.h"
#include "debug.h"
#include "distraction_manager.h"
#include "enums.h"
#include "filesystem.h"
#include "fstream_utils.h"
#include "game.h"
#include "game_info.h"
#include "get_version.h"
#include "help.h"
#include "loading_ui.h"
#include "map/mapbuffer.h"
#include "mapsharing.h"
#include "messages.h"
#include "newcharacter.h"
#include "options.h"
#include "output.h"
#include "overmap/overmapbuffer.h"
#include "path_info.h"
#include "platform/browser_storage.h"
#include "pldata.h"
#include "popup.h"
#include "safemode_ui.h"
#include "scenario.h"
#include "sdlsound.h"
#include "sounds.h"
#include "string_formatter.h"
#include "text_snippets.h"
#include "translations.h"
#include "ui.h"
#include "ui_manager.h"
#include "wcwidth.h"
#include "worldfactory.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <functional>
#include <istream>
#include <memory>
#include <optional>
enum class main_menu_opts : int {
    NEWCHAR = 0,
    LOADCHAR,
    SETTINGS,
    HELP,
    QUIT
};
static constexpr auto max_menu_opts = static_cast<int>( main_menu_opts::QUIT );

static int getopt( main_menu_opts o )
{
    return static_cast<int>( o );
}

namespace
{
point prev_submenu_top_left;
point prev_submenu_size;
} // namespace

void main_menu::on_move() const
{
    sfx::play_variant_sound( "menu_move", "default", 100 );
}

void main_menu::on_error()
{
    sfx::play_variant_sound( "menu_error", "default", 100 );
}

class sound_on_move_uilist_callback : public uilist_callback
{
    private:
        main_menu *mmenu;
        bool first = true;

    public:
        sound_on_move_uilist_callback( main_menu *mmenu ) : mmenu( mmenu ) { }

        void select( uilist * ) override {
            if( first ) {
                // Don't emit sound when menu is opened
                first = false;
                return;
            }
            mmenu->on_move();
        }
};

//CJK characters have a width of 2, etc
static int utf8_width_notags( const char *s )
{
    int len = strlen( s );
    const char *ptr = s;
    int w = 0;
    bool inside_tag = false;
    while( len > 0 ) {
        uint32_t ch = UTF8_getch( &ptr, &len );
        if( ch == UNKNOWN_UNICODE ) {
            continue;
        }
        if( ch == '<' ) {
            inside_tag = true;
        } else if( ch == '>' ) {
            inside_tag = false;
            continue;
        }
        if( inside_tag ) {
            continue;
        }
        w += mk_wcwidth( ch );
    }
    return w;
}

std::vector<int> main_menu::print_menu_items( const catacurses::window &w_in,
        const std::vector<std::string> &vItems,
        size_t iSel, point offset, int spacing, bool main )
{
    const point win_offset( getbegx( w_in ), getbegy( w_in ) );
    std::vector<int> ret;
    std::string text;
    for( size_t i = 0; i < vItems.size(); ++i ) {
        if( i > 0 ) {
            text += std::string( spacing, ' ' );
        }
        ret.push_back( utf8_width_notags( text.c_str() ) );

        std::string temp = shortcut_text( iSel == i ? hilite( c_yellow ) : c_yellow, vItems[i] );
        text += string_format( "[%s]", colorize( temp,
                               iSel == i ? hilite( c_light_gray ) : c_light_gray ) );
    }

    int text_width = utf8_width_notags( text.c_str() );
    if( text_width > getmaxx( w_in ) ) {
        offset.y -= std::ceil( text_width / getmaxx( w_in ) );
    }

    std::vector<std::string> menu_txt = foldstring( text, getmaxx( w_in ), ']' );

    int y_off = 0;
    int sel_opt = 0;
    for( const std::string &txt : menu_txt ) {
        trim_and_print( w_in, offset + point( 0, y_off ), getmaxx( w_in ), c_light_gray, txt );
        if( !main ) {
            y_off++;
            continue;
        }
        std::vector<std::string> tmp_chars = utf8_display_split( remove_color_tags( txt ) );
        for( int x = 0; static_cast<size_t>( x ) < tmp_chars.size(); x++ ) {
            if( tmp_chars.at( x ) == "[" ) {
                for( int x2 = x; static_cast<size_t>( x2 ) < tmp_chars.size(); x2++ ) {
                    if( tmp_chars.at( x2 ) == "]" ) {
                        inclusive_rectangle<point> rec( win_offset + offset + point( x, y_off ),
                                                        win_offset + offset + point( x2, y_off ) );
                        main_menu_button_map.emplace_back( rec, sel_opt++ );
                        break;
                    }
                }
            }
        }
        y_off++;
    }

    return ret;
}

void main_menu::display_sub_menu( int sel, const point &bottom_left )
{
    main_menu_sub_button_map.clear();
    std::vector<std::string> sub_opts;
    int xlen = 0;
    main_menu_opts sel_o = static_cast<main_menu_opts>( sel );
    switch( sel_o ) {
        case main_menu_opts::SETTINGS:
            for( int i = 0; static_cast<size_t>( i ) < vSettingsSubItems.size(); ++i ) {
                nc_color clr = i == sel2 ? hilite( c_yellow ) : c_yellow;
                sub_opts.push_back( shortcut_text( clr, vSettingsSubItems[i] ) );
                int len = utf8_width( shortcut_text( clr, vSettingsSubItems[i] ), true );
                if( len > xlen ) {
                    xlen = len;
                }
            }
            break;
        case main_menu_opts::NEWCHAR:
        case main_menu_opts::LOADCHAR:
        case main_menu_opts::HELP:
        case main_menu_opts::QUIT:
        default:
            return;
    }

    if( sub_opts.empty() ) {
        return;
    }

    const auto max_visible = std::max( 0, bottom_left.y - 1 );
    if( max_visible == 0 ) {
        return;
    }

    const auto visible_count = std::min<int>( static_cast<int>( sub_opts.size() ), max_visible );

    const auto start = std::clamp(
                           std::max( 0, sel2 - visible_count + 1 ),
                           0,
                           static_cast<int>( sub_opts.size() ) - visible_count
                       );
    const auto win_width = std::min( xlen + 4, TERMX );
    const auto content_width = win_width - 2;

    auto top_left = bottom_left + point( 0, -( visible_count + 1 ) );
    top_left.x = clamp( top_left.x, 0, TERMX - win_width );

    prev_submenu_top_left = top_left;
    prev_submenu_size = point( win_width, visible_count + 2 );

    auto w_sub = catacurses::newwin( visible_count + 2, win_width, top_left );
    werase( w_sub );
    draw_border( w_sub, c_light_gray );

    for( int y = 0; y < visible_count; y++ ) {
        const auto opt_index = start + y;
        auto opt = ( sel2 == opt_index ? "» " : "  " ) + sub_opts[opt_index];
        const auto padding = content_width - utf8_width( opt, true );
        if( padding > 0 ) {
            opt.append( padding, ' ' );
        }
        const auto color = sel2 == opt_index ? hilite( c_light_gray ) : c_light_gray;
        trim_and_print( w_sub, point( 1, y + 1 ), content_width, color, opt );
        auto rec = inclusive_rectangle<point> { top_left + point( 1, y + 1 ), top_left + point( content_width, y + 1 ) };
        main_menu_sub_button_map.emplace_back( rec, std::pair<int, int> { sel, opt_index } );
    }
    wnoutrefresh( w_sub );
}

void main_menu::print_menu( const catacurses::window &w_open, int iSel, const point &offset )
{
    main_menu_button_map.clear();

    // If a submenu was drawn on a previous frame, clear it *before* drawing the main menu.
    // This ensures the title/logo background is repainted instead of leaving a black rectangle.
    if( prev_submenu_size.x > 0 && prev_submenu_size.y > 0 ) {
        point tl = prev_submenu_top_left;
        tl.x = clamp( tl.x, 0, TERMX - 1 );
        tl.y = clamp( tl.y, 0, TERMY - 1 );

        const int w = clamp( prev_submenu_size.x, 0, TERMX - tl.x );
        const int h = clamp( prev_submenu_size.y, 0, TERMY - tl.y );
        if( w > 0 && h > 0 ) {
            auto w_clear = catacurses::newwin( h, w, tl );
            werase( w_clear );
            wnoutrefresh( w_clear );
        }
        prev_submenu_top_left = point_zero;
        prev_submenu_size = point_zero;
    }

    // Clear Lines
    werase( w_open );

    // Define window size
    int window_width = getmaxx( w_open );
    int window_height = getmaxy( w_open );

    // Draw horizontal line
    for( int i = 1; i < window_width - 1; ++i ) {
        mvwputch( w_open, point( i, window_height - 5 ), c_white, LINE_OXOX );
    }

    if( iSel == getopt( main_menu_opts::NEWCHAR ) ) {
        const auto lines = foldstring(
                               _( "Allows you to fully customize points pool, scenario, and character's profession, stats, traits, skills and other parameters." ),
                               window_width - 2 );
        center_print( w_open, window_height - 3, c_yellow, lines[0] );
        if( lines.size() > 1 ) {
            center_print( w_open, window_height - 2, c_yellow, lines[1] );
        }
        if( lines.size() > 2 ) {
            center_print( w_open, window_height - 1, c_yellow, lines[2] );
        }
    } else {
        std::vector<std::string> lines = foldstring( string_format( _( "Tip of the day: %s" ),
                                         vdaytip ), window_width - 2 );
        center_print( w_open, window_height - 2, c_light_cyan, lines[0] );
        if( lines.size() > 1 ) {
            center_print( w_open, window_height - 1, c_light_cyan, lines[1] );
        }
    }

    int iLine = 0;
    const int iOffsetX = ( window_width - FULL_SCREEN_WIDTH ) / 2;

    switch( current_holiday ) {
        case holiday::new_year:
        case holiday::easter:
            break;
        case holiday::halloween:
            fold_and_print_from( w_open, point_zero, 30, 0, c_white, halloween_spider() );
            fold_and_print_from( w_open, point( getmaxx( w_open ) - 25, offset.y - 8 ),
                                 25, 0, c_white, halloween_graves() );
            break;
        case holiday::thanksgiving:
        case holiday::christmas:
        case holiday::none:
        case holiday::num_holiday:
        default:
            break;
    }

    if( mmenu_title.size() > 1 ) {
        for( const std::string &i_title : mmenu_title ) {
            nc_color cur_color = c_white;
            nc_color base_color = c_white;
            print_colored_text( w_open, point( iOffsetX, iLine++ ), cur_color, base_color, i_title );
        }
    } else {
        center_print( w_open, iLine++, c_light_cyan, mmenu_title[0] );
    }

    iLine++;
    center_print( w_open, iLine, c_light_blue, string_format( _( "Version: %s" ),
                  getVersionString() ) );

    int menu_length = 0;
    for( size_t i = 0; i < vMenuItems.size(); ++i ) {
        menu_length += utf8_width_notags( vMenuItems[i].c_str() ) + 2;
        if( !vMenuHotkeys[i].empty() ) {
            menu_length += utf8_width( vMenuHotkeys[i][0] );
        }
    }
    const int free_space = std::max( 0, window_width - menu_length - offset.x );
    const int spacing = free_space / ( static_cast<int>( vMenuItems.size() ) + 1 );
    const int width_of_spacing = spacing * ( vMenuItems.size() + 1 );
    const int adj_offset = std::max( 0, ( free_space - width_of_spacing ) / 2 );
    const int final_offset = offset.x + adj_offset + spacing;

    std::vector<int> offsets =
        print_menu_items( w_open, vMenuItems, iSel, point( final_offset, offset.y - 1 ), spacing, true );

    wnoutrefresh( w_open );

    const point p_offset( catacurses::getbegx( w_open ), catacurses::getbegy( w_open ) );

    display_sub_menu( iSel, p_offset + point( offsets[iSel], offset.y - 3 ) );
}

std::vector<std::string> main_menu::load_file( const std::string &path,
        const std::string &alt_text ) const
{
    std::vector<std::string> result;
    read_from_file( path, [&result]( std::istream & fin ) {
        std::string line;
        while( std::getline( fin, line ) ) {
            if( !line.empty() && line[0] == '#' ) {
                continue;
            }
            result.push_back( line );
        }
    }, true );
    if( result.empty() ) {
        result.push_back( alt_text );
    }
    return result;
}

holiday main_menu::get_holiday_from_time()
{
    return ::get_holiday_from_time( 0, true );
}

void main_menu::init_windows()
{
    if( LAST_TERM == point( TERMX, TERMY ) ) {
        return;
    }

    // main window should also expand to use available display space.
    // expanding to evenly use up half of extra space, for now.
    extra_w = ( ( TERMX - FULL_SCREEN_WIDTH ) / 2 ) - 1;
    int extra_h = ( ( TERMY - FULL_SCREEN_HEIGHT ) / 2 ) - 1;
    extra_w = ( extra_w > 0 ? extra_w : 0 );
    extra_h = ( extra_h > 0 ? extra_h : 0 );
    const int total_w = FULL_SCREEN_WIDTH + extra_w;
    const int total_h = FULL_SCREEN_HEIGHT + extra_h;

    // position of window within main display
    const point p0( ( TERMX - total_w ) / 2, ( TERMY - total_h ) / 2 );

    w_open = catacurses::newwin( total_h, total_w, p0 );

    menu_offset.y = total_h - 3;

    LAST_TERM = point( TERMX, TERMY );
}

void main_menu::init_strings()
{
    // ASCII Art
    mmenu_title = load_file( PATH_INFO::title( current_holiday ), _( "Cataclysm: Fault Lines" ) );

    // fill menu with translated menu items
    vMenuItems.clear();
    vMenuItems.emplace_back( pgettext( "Main Menu", "<N|n>ew Game" ) );
    vMenuItems.emplace_back( pgettext( "Main Menu", "<C|c>ontinue" ) );
    vMenuItems.emplace_back( pgettext( "Main Menu", "Se<t|T>tings" ) );
    vMenuItems.emplace_back( pgettext( "Main Menu", "H<e|E|?>lp" ) );
    vMenuItems.emplace_back( pgettext( "Main Menu", "<Q|q>uit" ) );

    // determine hotkeys from translated menu item text
    vMenuHotkeys.clear();
    for( const std::string &item : vMenuItems ) {
        vMenuHotkeys.push_back( get_hotkeys( item ) );
    }

    vWorldSubItems.clear();
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "Show World Mods" ) );
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "Copy World Settings" ) );
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "Reset World" ) );
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "Delete World" ) );
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "Convert to V2 Save Format" ) );
    vWorldSubItems.emplace_back( pgettext( "Main Menu|World", "<= Return" ) );

    vWorldHotkeys = { 'm', 's', 'r', 'd', 'f', 'q' };

    vSettingsSubItems.clear();
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "<O|o>ptions" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "Ke<y|Y>bindings" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "A<u|U>topickup" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "Sa<f|F>emode" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "<D|d>istractions" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "Colo<r|R>s" ) );
    vSettingsSubItems.emplace_back( pgettext( "Main Menu|Settings", "<W|w>orld" ) );

    vSettingsHotkeys.clear();
    for( const std::string &item : vSettingsSubItems ) {
        vSettingsHotkeys.push_back( get_hotkeys( item ) );
    }

    vdaytip = get_random_tip_of_the_day();
}


bool main_menu::opening_screen()
{
    // set holiday based on local system time
    current_holiday = get_holiday_from_time();

    // Play title music, whoo!
    play_music( "title" );

    world_generator->set_active_world( nullptr );
    world_generator->init();

    get_help().load();
    init_strings();

    if( !assure_dir_exist( PATH_INFO::config_dir() ) ) {
        popup( _( "Unable to make config directory.  Check permissions." ) );
        return false;
    }

    if( !assure_dir_exist( PATH_INFO::user_moddir() ) ) {
        popup( _( "Unable to make user mods directory.  Check permissions." ) );
        return false;
    }

    if( !assure_dir_exist( PATH_INFO::savedir() ) ) {
        popup( _( "Unable to make save directory.  Check permissions." ) );
        return false;
    }

    if( !assure_dir_exist( PATH_INFO::user_fontdir() ) ) {
        popup( _( "Unable to make fonts directory.  Check permissions." ) );
        return false;
    }

    if( !assure_dir_exist( PATH_INFO::user_sound() ) ) {
        popup( _( "Unable to make sound directory.  Check permissions." ) );
        return false;
    }

    if( !assure_dir_exist( PATH_INFO::user_gfx() ) ) {
        popup( _( "Unable to make graphics directory.  Check permissions." ) );
        return false;
    }

    std::optional<int> os_bitness = get_os_bitness();
    std::optional<int> game_bitness = game_info::bitness();
    if( os_bitness && *os_bitness == 64 && game_bitness && *game_bitness == 32 ) {
        popup( _(
                   "You are running x32 build of the game on a x64 operating system.  "
                   "This means the game will NOT be able to access all memory, "
                   "and you may experience random out-of-memory crashes.  "
                   "Consider obtaining a x64 build of the game to avoid that, "
                   "but if you *really* want to be running x32 build of the game "
                   "for some reason (or don't have a choice), you may want to lower "
                   "your memory usage by disabling tileset, soundpack and mods "
                   "and increasing autosave frequency."
               ) );
    }


    ctxt.register_cardinal();
    ctxt.register_action( "NEXT_TAB" );
    ctxt.register_action( "PREV_TAB" );
    ctxt.register_action( "PAGE_UP" );
    ctxt.register_action( "PAGE_DOWN" );
    ctxt.register_action( "CONFIRM" );
    ctxt.register_action( "QUIT" );

    // for the menu shortcuts
    ctxt.register_action( "ANY_INPUT" );
    bool start = false;

    avatar &player_character = get_avatar();
    player_character = avatar();

    // Make Continue the default cursor position if there's a saved game available
    if( !world_generator->all_worldnames().empty() ) {
        sel1 = getopt( main_menu_opts::LOADCHAR );
    }

    background_pane background;

    ui_adaptor ui;
    ui.on_redraw( [&]( const ui_adaptor & ) {
        print_menu( w_open, sel1, menu_offset );
    } );
    ui.on_screen_resize( [this]( ui_adaptor & ui ) {
        init_windows();
        ui.position_from_window( w_open );
    } );
    ui.mark_resize();

    bool start_new = false;
    while( !start ) {
        ui_manager::redraw();
        std::string action = ctxt.handle_input();
        input_event sInput = ctxt.get_raw_input();

        // check automatic menu shortcuts
        for( int i = 0; static_cast<size_t>( i ) < vMenuHotkeys.size(); ++i ) {
            for( const std::string &hotkey : vMenuHotkeys[i] ) {
                if( sInput.text == hotkey && sel1 != i ) {
                    sel1 = i;
                    sel2 = 0;
                    if( i == getopt( main_menu_opts::HELP ) ) {
                        action = "CONFIRM";
                    } else if( i == getopt( main_menu_opts::QUIT ) ) {
                        action = "QUIT";
                    }
                }
            }
        }
        if( sel1 == getopt( main_menu_opts::SETTINGS ) ) {
            for( int i = 0; static_cast<size_t>( i ) < vSettingsSubItems.size(); ++i ) {
                for( const std::string &hotkey : vSettingsHotkeys[i] ) {
                    if( sInput.text == hotkey ) {
                        sel2 = i;
                        action = "CONFIRM";
                    }
                }
            }
        }

        // also check special keys
        if( action == "QUIT" ) {
            ui_manager::redraw();
            if( query_yn( _( "Really quit?" ) ) ) {
                return false;
            }
        } else if( action == "LEFT" || action == "PREV_TAB" ) {
            if( sel1 > 0 ) {
                sel1--;
            } else {
                sel1 = max_menu_opts;
            }
            sel2 = 0;
            on_move();
        } else if( action == "RIGHT" || action == "NEXT_TAB" ) {
            if( sel1 < max_menu_opts ) {
                sel1++;
            } else {
                sel1 = 0;
            }
            sel2 = 0;
            on_move();
        } else if( action == "UP" || action == "DOWN" ||
                   action == "PAGE_UP" || action == "PAGE_DOWN" ||
                   action == "SCROLL_UP" || action == "SCROLL_DOWN" ) {
            int max_item_count = 0;
            int min_item_val = 0;
            main_menu_opts opt = static_cast<main_menu_opts>( sel1 );
            switch( opt ) {
                case main_menu_opts::SETTINGS:
                    max_item_count = vSettingsSubItems.size();
                    break;
                case main_menu_opts::HELP:
                case main_menu_opts::QUIT:
                default:
                    break;
            }
            if( max_item_count > 0 ) {
                if( action == "UP" || action == "PAGE_UP" || action == "SCROLL_UP" ) {
                    sel2--;
                    if( sel2 < min_item_val ) {
                        sel2 = max_item_count - 1;
                    }
                } else if( action == "DOWN" || action == "PAGE_DOWN" || action == "SCROLL_DOWN" ) {
                    sel2++;
                    if( sel2 >= max_item_count ) {
                        sel2 = min_item_val;
                    }
                }
                on_move();
            }
        } else if( action == "CONFIRM" ) {
            switch( static_cast<main_menu_opts>( sel1 ) ) {
                case main_menu_opts::HELP:
                    get_help().display_help();
                    break;
                case main_menu_opts::QUIT:
                    return false;
                case main_menu_opts::SETTINGS:
                    if( sel2 == 0 ) {        /// Options
                        get_options().show( false );
                        // The language may have changed- gracefully handle this.
                        init_strings();
                    } else if( sel2 == 1 ) { /// Keybindings
                        input_context ctxt_default = get_default_mode_input_context();
                        ctxt_default.display_menu();
                    } else if( sel2 == 2 ) { /// Autopickup
                        get_auto_pickup().show();
                    } else if( sel2 == 3 ) { /// Safemode
                        get_safemode().show();
                    } else if( sel2 == 4 ) {
                        get_distraction_manager().show();
                    } else if( sel2 == 5 ) {
                        all_colors.show_gui();
                    } else if( sel2 == 6 ) {
                        world_tab();
                    }
                    break;
                case main_menu_opts::LOADCHAR:
                    start = load_character_tab();
                    break;
                case main_menu_opts::NEWCHAR:
                    start = new_character_tab();
                    if( start ) {
                        start_new = true;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    if( start_new && get_scenario() ) {
        add_msg( get_scenario()->description( player_character.male ) );
    }
    return true;
}

bool main_menu::new_character_tab()
{
    auto &pc = get_avatar();
    on_out_of_scope cleanup( [&pc]() {
        pc = avatar();
        world_generator->set_active_world( nullptr );
    } );

    auto *world = world_generator->pick_world();
    if( world == nullptr ) {
        return false;
    }
    world_generator->set_active_world( world );
    try {
        g->setup();
    } catch( const std::exception &err ) {
        debugmsg( "Error: %s", err.what() );
        return false;
    }

    if( !pc.create() ) {
        MAPBUFFER.clear();
        get_primary_overmapbuffer().clear();
        return false;
    }

    if( !g->start_game() ) {
        return false;
    }
    cleanup.cancel();
    return true;
}

auto main_menu::load_character_tab() -> bool
{
    namespace ranges = std::ranges;

    struct continue_entry {
        WORLDINFO *world;
        save_t save;
        std::string character_name;
    };

    auto saves = std::vector<continue_entry>{};
    for( const auto &world_name : world_generator->all_worldnames() ) {
        auto *world = world_generator->get_world( world_name );
        for( const auto &save : world->world_saves ) {
            const auto name = save.decoded_name();
            if( !MAP_SHARING::isSharing() || name == MAP_SHARING::getUsername() ) {
                saves.push_back( { .world = world, .save = save, .character_name = name } );
            }
        }
    }

    if( saves.empty() ) {
        on_error();
        popup( _( "No saved characters to continue." ) );
        return false;
    }

    ranges::sort( saves, []( const auto &lhs, const auto &rhs ) {
        return lhs.character_name == rhs.character_name ? lhs.world->world_name < rhs.world->world_name
               : lhs.character_name < rhs.character_name;
    } );

    auto mmenu = uilist( _( "Continue as which character?" ), {} );
    mmenu.border_color = c_light_gray;
    mmenu.hotkey_color = c_yellow;
    auto cb = sound_on_move_uilist_callback( this );
    mmenu.callback = &cb;
    auto index = 0;
    for( const auto &entry : saves ) {
        if( entry.world->world_name == world_generator->last_world_name &&
            entry.character_name == world_generator->last_character_name ) {
            mmenu.selected = index;
        }
        //~ %1$s is the character name, %2$s is the world name.
        mmenu.entries.emplace_back( index++, true, MENU_AUTOASSIGN,
                                    string_format( _( "%1$s (%2$s)" ), entry.character_name, entry.world->world_name ) );
    }
    mmenu.entries.emplace_back( index, true, 'q', _( "<= Return" ) );
    mmenu.query();
    const auto selected = mmenu.ret;
    if( selected < 0 || static_cast<size_t>( selected ) >= saves.size() ) {
        return false;
    }
    const auto &entry = saves[selected];

    auto &pc = get_avatar();
    on_out_of_scope cleanup( [&pc]() {
        pc = avatar();
        world_generator->set_active_world( nullptr );
    } );

    world_generator->last_world_name = entry.world->world_name;
    world_generator->last_character_name = entry.character_name;
    world_generator->save_last_world_info();
    world_generator->set_active_world( entry.world );

    try {
        g->setup();
    } catch( const std::exception &err ) {
        debugmsg( "Error: %s", err.what() );
        return false;
    }

    if( g->load( entry.save ) ) {
        cleanup.cancel();
        return true;
    }

    return false;
}

auto main_menu::world_tab() -> void
{
    const auto worlds = world_generator->all_worldnames();
    auto menu = uilist( pgettext( "Main Menu|Settings", "World" ), {} );
    menu.border_color = c_light_gray;
    menu.hotkey_color = c_yellow;
    auto cb = sound_on_move_uilist_callback( this );
    menu.callback = &cb;
    menu.entries.emplace_back( 0, true, MENU_AUTOASSIGN, _( "Create World" ) );
    auto index = 1;
    for( const auto &name : worlds ) {
        const auto count = world_generator->get_world( name )->world_saves.size();
        menu.entries.emplace_back( index++, true, MENU_AUTOASSIGN,
                                   string_format( "%s (%zu)", name, count ) );
    }
    menu.entries.emplace_back( index, true, 'q', _( "<= Return" ) );
    menu.query();
    if( menu.ret == 0 ) {
        if( world_generator->make_new_world() ) {
            browser_storage::flush_with_feedback();
        }
    } else if( menu.ret > 0 && menu.ret < index ) {
        manage_world( worlds[menu.ret - 1] );
    }
}

auto main_menu::manage_world( const std::string &worldname ) -> void
{
    auto *world = world_generator->get_world( worldname );
    const auto is_v2_world = world->world_save_format == save_format::V2_COMPRESSED_SQLITE3;

    uilist mmenu( string_format( _( "Manage world \"%s\"" ), worldname ), {} );
    mmenu.border_color = c_light_gray;
    mmenu.hotkey_color = c_yellow;
    sound_on_move_uilist_callback cb( this );
    mmenu.callback = &cb;
    for( size_t i = 0; i < vWorldSubItems.size(); i++ ) {
        const auto enabled = !( i == 4 && is_v2_world );
        mmenu.entries.emplace_back( static_cast<int>( i ), enabled, vWorldHotkeys[i], vWorldSubItems[i] );
    }
    mmenu.query();
    int opt_val = mmenu.ret;
    if( opt_val < 0 || static_cast<size_t>( opt_val ) >= vWorldSubItems.size() ) {
        return;
    }

    auto clear_world = [&worldname]( bool do_delete ) {
        world_generator->delete_world( worldname, do_delete );
        browser_storage::flush_with_feedback();
        MAPBUFFER.clear();
        get_primary_overmapbuffer().clear();
    };

    auto convert_v2 = [&worldname]() {
        world_generator->set_active_world( nullptr );
        MAPBUFFER.clear();
        get_primary_overmapbuffer().clear();
        world_generator->convert_to_v2( worldname );
        browser_storage::flush_with_feedback();
    };

    switch( opt_val ) {
        case 4: // Convert to V2 Save Format
            if( query_yn(
                    _( "Convert to V2 Save Format? A backup will be created. Conversion may take several minutes." ) ) ) {
                convert_v2();
            }
            break;
        case 3: // Delete World
            if( query_yn( _( "Delete the world and all saves within?" ) ) ) {
                clear_world( true );
            }
            break;
        case 2: // Reset World
            if( query_yn( _( "Remove all saves and regenerate world?" ) ) ) {
                clear_world( false );
            }
            break;
        case 0: // Active World Mods
            world_generator->show_active_world_mods(
                world_generator->get_world( worldname )->active_mod_order );
            break;
        case 1: // Copy World settings
            if( world_generator->make_new_world( true, worldname ) ) {
                browser_storage::flush_with_feedback();
            }
            break;
        default:
            break;
    }
}

std::string main_menu::halloween_spider()
{
    static const std::string spider =
        "\\ \\ \\/ / / / / / / /\n"
        " \\ \\/\\/ / / / / / /\n"
        "\\ \\/__\\/ / / / / /\n"
        " \\/____\\/ / / / /\n"
        "\\/______\\/ / / /\n"
        "/________\\/ / /\n"
        "__________\\/ /\n"
        "___________\\/\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "        |\n"
        "  , .   |  . ,\n" // NOLINT(cata-text-style)
        "  { | ,--, | }\n" // NOLINT(cata-text-style)
        "   \\\\{~~~~}//\n"
        "  /_/ {<color_c_red>..</color>} \\_\\\n"
        "  { {      } }\n"
        "  , ,      , ."; // NOLINT(cata-text-style)

    return spider;
}

std::string main_menu::halloween_graves()
{
    static const std::string graves =
        "                    _\n"
        "        -q       __(\")_\n"
        "         (\\      \\_  _/\n"
        " .-.   .-''\"'.     |/\n" // NOLINT(cata-text-style)
        "|RIP|  | RIP |   .-.\n"
        "|   |  |     |  |RIP|\n"
        ";   ;  |     | ,'---',"; // NOLINT(cata-text-style)

    return graves;
}

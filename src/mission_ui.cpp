#include "game.h" // IWYU pragma: associated

#include "overmap/omdata.h"
#include "overmap/overmapbuffer.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "avatar.h"
#include "calendar.h"
#include "color.h"
#include "debug.h"
#include "faction.h"
#include "input.h"
#include "item.h"
#include "mission.h"
#include "npc.h"
#include "output.h"
#include "string_formatter.h"
#include "string_utils.h"
#include "translations.h"
#include "ui.h"
#include "ui_manager.h"

namespace
{
/// Describe only tiles the player has seen in this dimension.
auto known_mission_location( const tripoint_abs_omt &pos, const dimension_id &dimension )
-> std::string
{
    auto &buffer = get_overmapbuffer( dimension );
    if( !buffer.seen( pos ) ) {
        return {};
    }
    return buffer.ter( pos )->get_name();
}

auto mission_details( const mission &job, const npc *giver ) -> std::string
{
    auto details = std::string();
    auto giver_location = std::string();
    if( giver ) {
        details += string_format( _( "Quest giver: %s" ), giver->disp_name() ) + "\n";
        if( const auto *fac = giver->get_faction() ) {
            details += string_format( _( "Faction: %s" ), fac->name ) + "\n";
        }
        giver_location = known_mission_location( giver->abs_omt_pos(), giver->get_dimension() );
        if( !giver_location.empty() ) {
            details += string_format( _( "Quest giver location: %s" ), giver_location ) + "\n";
        }
    } else if( job.get_npc_id().is_valid() ) {
        details += _( "Quest giver: Unavailable" ) + std::string( "\n" );
    }

    auto objective = job.get_description();
    for( const auto &[count, id] : job.get_likely_rewards() ) {
        objective = replace_all( objective, "<reward_count:" + id.str() + ">", std::to_string( count ) );
    }
    details += "\n" + std::string( _( "Objective" ) ) + "\n" +
               ( objective.empty() ? _( "No objective details recorded." ) : objective ) + "\n";

    details += "\n" + std::string( _( "Reward" ) ) + "\n";
    const auto &authored_reward = job.get_type().reward_description;
    if( !authored_reward.empty() ) {
        details += authored_reward.translated() + "\n";
    } else {
        for( const auto &[count, id] : job.get_likely_rewards() ) {
            details += string_format( _( "%1$d %2$s" ), count, item::nname( id, count ) ) + "\n";
        }
        if( job.get_likely_rewards().empty() ) {
            details += _( "No reward recorded." ) + std::string( "\n" );
        }
    }
    if( const auto destination = job.get_objective_destination() ) {
        // Legacy saves may only have the return marker after an objective was completed.
        if( !giver || giver->get_dimension() != job.get_dimension() ||
            giver->abs_omt_pos() != *destination ) {
            const auto location = known_mission_location( *destination, job.get_dimension() );
            details += "\n" + std::string( _( "Destination" ) ) + "\n";
            details += location.empty() ? _( "Location undiscovered" ) : location;
            details += "\n";
        }
    }
    if( giver ) {
        details += "\n" + std::string( _( "Return to" ) ) + "\n" + giver->disp_name();
        if( !giver_location.empty() ) {
            details += string_format( _( " (%s)" ), giver_location );
        }
        details += "\n";
    }
    return details;
}
} // namespace

auto game::list_missions() -> void
{
    catacurses::window w_missions;

    enum class tab_mode : int {
        TAB_ACTIVE = 0,
        TAB_COMPLETED,
        TAB_FAILED,
        NUM_TABS,
        FIRST_TAB = 0,
        LAST_TAB = NUM_TABS - 1
    };
    tab_mode tab = tab_mode::FIRST_TAB;
    size_t selection = 0;
    auto entries_per_page = 0;
    auto details_offset = 0;
    auto max_details_offset = 0;
    input_context ctxt( "MISSIONS" );
    ctxt.register_cardinal();
    ctxt.register_action( "CONFIRM" );
    ctxt.register_action( "PAGE_UP" );
    ctxt.register_action( "PAGE_DOWN" );
    ctxt.register_action( "QUIT" );
    ctxt.register_action( "HELP_KEYBINDINGS" );

    ui_adaptor ui;
    ui.on_screen_resize( [&]( ui_adaptor & ui ) {
        w_missions = new_centered_win( FULL_SCREEN_HEIGHT, FULL_SCREEN_WIDTH );

        // content ranges from y=3 to FULL_SCREEN_HEIGHT - 2
        entries_per_page = std::max( 1, getmaxy( w_missions ) - 4 );

        ui.position_from_window( w_missions );
    } );
    ui.mark_resize();

    std::vector<mission *> umissions;

    ui.on_redraw( [&]( const ui_adaptor & ) {
        werase( w_missions );
        const auto height = getmaxy( w_missions );
        const auto width = getmaxx( w_missions );
        const auto divider = std::min( 30, width / 3 );
        const auto details_width = width - divider - 3;
        // entries_per_page * page number
        const int top_of_page = entries_per_page * ( selection / entries_per_page );
        const int bottom_of_page =
            std::min( top_of_page + entries_per_page - 1, static_cast<int>( umissions.size() ) - 1 );

        for( int i = 3; i < height - 1; i++ ) {
            mvwputch( w_missions, point( divider, i ), BORDER_COLOR, LINE_XOXO );
        }

        const std::vector<std::pair<tab_mode, std::string>> tabs = {
            { tab_mode::TAB_ACTIVE, _( "ACTIVE MISSIONS" ) },
            { tab_mode::TAB_COMPLETED, _( "COMPLETED MISSIONS" ) },
            { tab_mode::TAB_FAILED, _( "FAILED MISSIONS" ) },
        };
        draw_tabs( w_missions, tabs, tab );
        draw_border_below_tabs( w_missions );

        mvwputch( w_missions, point( divider, 2 ), BORDER_COLOR,
                  tab == tab_mode::TAB_COMPLETED ? ' ' : LINE_OXXX ); // ^|^
        mvwputch( w_missions, point( divider, height - 1 ), BORDER_COLOR, LINE_XXOX ); // _|_

        draw_scrollbar( w_missions, selection, entries_per_page, umissions.size(), point( 0, 3 ) );

        for( int i = top_of_page; i <= bottom_of_page; i++ ) {
            const auto miss = umissions[i];
            const nc_color col = u.get_active_mission() == miss ? c_light_green : c_white;
            const int y = i - top_of_page + 3;
            trim_and_print( w_missions, point( 1, y ), divider - 2,
                            static_cast<int>( selection ) == i ? hilite( col ) : col,
                            miss->name() );
        }

        if( selection < umissions.size() ) {
            const auto miss = umissions[selection];
            const nc_color col = u.get_active_mission() == miss ? c_light_green : c_white;
            const auto *giver = miss->get_npc_id().is_valid() ? find_npc( miss->get_npc_id() ) : nullptr;
            auto details = mission_details( *miss, giver );
            if( miss->has_deadline() ) {
                const auto deadline = miss->get_deadline();
                details += "\n" + string_format( _( "Deadline: %s" ), to_string( deadline ) );
                if( tab == tab_mode::TAB_ACTIVE ) {
                    const auto remaining = deadline - calendar::turn;
                    const auto remaining_time = remaining <= 0_turns ? _( "None!" ) :
                                                u.has_watch() ? to_string( remaining ) : to_string_approx( remaining );
                    details += "\n" + string_format( _( "Time remaining: %s" ), remaining_time );
                }
            }
            trim_and_print( w_missions, point( divider + 1, 3 ), details_width, col, miss->name() );
            const auto lines = foldstring( details, details_width );
            const auto visible_lines = std::max( 0, height - 6 );
            max_details_offset = std::max( 0, static_cast<int>( lines.size() ) - visible_lines );
            details_offset = std::clamp( details_offset, 0, max_details_offset );
            for( auto row = 0; row < visible_lines && row + details_offset < static_cast<int>( lines.size() );
                 ++row ) {
                trim_and_print( w_missions, point( divider + 1, row + 4 ), details_width,
                                c_white, lines[row + details_offset] );
            }
            if( max_details_offset > 0 ) {
                trim_and_print( w_missions, point( divider + 1, height - 2 ), details_width, c_light_gray,
                                _( "%1$s / %2$s to scroll" ), ctxt.get_desc( "PAGE_UP" ), ctxt.get_desc( "PAGE_DOWN" ) );
            }
        } else {
            static const std::map< tab_mode, std::string > nope = {
                { tab_mode::TAB_ACTIVE, translate_marker( "You have no active missions!" ) },
                { tab_mode::TAB_COMPLETED, translate_marker( "You haven't completed any missions!" ) },
                { tab_mode::TAB_FAILED, translate_marker( "You haven't failed any missions!" ) }
            };
            fold_and_print( w_missions, point( divider + 1, 4 ), details_width, c_light_red,
                            _( nope.at( tab ) ) );
        }

        wnoutrefresh( w_missions );
    } );

    while( true ) {
        umissions.clear();
        if( tab < tab_mode::FIRST_TAB || tab >= tab_mode::NUM_TABS ) {
            debugmsg( "The sanity check failed because tab=%d", static_cast<int>( tab ) );
            tab = tab_mode::FIRST_TAB;
        }
        switch( tab ) {
            case tab_mode::TAB_ACTIVE:
                umissions = u.get_active_missions();
                break;
            case tab_mode::TAB_COMPLETED:
                umissions = u.get_completed_missions();
                break;
            case tab_mode::TAB_FAILED:
                umissions = u.get_failed_missions();
                break;
            default:
                break;
        }
        if( ( !umissions.empty() && selection >= umissions.size() ) ||
            ( umissions.empty() && selection != 0 ) ) {
            debugmsg( "Sanity check failed: selection=%d, size=%d", static_cast<int>( selection ),
                      static_cast<int>( umissions.size() ) );
            selection = 0;
        }
        ui_manager::redraw();
        const std::string action = ctxt.handle_input();
        if( action == "RIGHT" ) {
            details_offset = 0;
            tab = static_cast<tab_mode>( static_cast<int>( tab ) + 1 );
            if( tab >= tab_mode::NUM_TABS ) {
                tab = tab_mode::FIRST_TAB;
            }
            selection = 0;
        } else if( action == "LEFT" ) {
            details_offset = 0;
            tab = static_cast<tab_mode>( static_cast<int>( tab ) - 1 );
            if( tab < tab_mode::FIRST_TAB ) {
                tab = tab_mode::LAST_TAB;
            }
            selection = 0;
        } else if( action == "DOWN" ) {
            details_offset = 0;
            selection++;
            if( selection >= umissions.size() ) {
                selection = 0;
            }
        } else if( action == "UP" ) {
            details_offset = 0;
            if( selection == 0 ) {
                selection = umissions.empty() ? 0 : umissions.size() - 1;
            } else {
                selection--;
            }
        } else if( action == "PAGE_UP" && !umissions.empty() ) {
            details_offset = std::max( 0, details_offset - std::max( 1, getmaxy( w_missions ) - 6 ) );
        } else if( action == "PAGE_DOWN" && !umissions.empty() ) {
            details_offset = std::min( max_details_offset, details_offset + std::max( 1,
                getmaxy( w_missions ) - 6 ) );
        } else if( action == "CONFIRM" ) {
            if( tab == tab_mode::TAB_ACTIVE && selection < umissions.size() ) {
                u.set_active_mission( *umissions[selection] );
            }
            break;
        } else if( action == "QUIT" ) {
            break;
        }
    }
}

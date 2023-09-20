#include <iostream>
#include <vector>

#include "util/debug_console.h"

#include "logging/logging.h"
#include "util/string_help.h"

#include "common/assets/assets.h"
#include "engine/level/level.h"

using namespace util;


static void handleProfiler (game::TestApp* app, const std::vector<std::string>& args) {
    if (args.empty()) {
        logging::log(LEVEL_WARNING, "Missing argument after \"profiler\"");
    } else if (args[0] == "display") {
        if (args.size() == 1 || (args[1] != "on" && args[1] != "off")) {
            logging::log(LEVEL_WARNING, R"(Unknown or missing argument after "display". Should be either "on" or "off")");
        } else {
            //bus->raise(event::ProfilerChangeEvent(args[1] == "on"));
            app->updateProfileRender(args[1] == "on");
        }
    } else {
        logging::log(LEVEL_WARNING, "Unknown argument: \"{}\"", args[0]);
    }
}

static void handleThemes (game::TestApp* app, std::vector<std::string>& args) {
    if (args.empty()) {
        logging::log(LEVEL_WARNING, "Missing argument after \"theme\"!");
    } else if (args[0] == "load") {
        if (args.size() != 2) {
            logging::log(LEVEL_WARNING, R"(Unknown argument after "theme".)");
        } else {
            //bus->raise(event::ChangeThemeEvent{args[1]});
            auto theme = common::Assets::Load<graphics::ui::Theme>(args[1]);
            if (!theme) {
                logging::log(LEVEL_ERROR, "Failed to load theme \"{}\"!", args[1]);
            } else {
                app->changeTheme(theme);
            }
        }
    }
}

static void doDebugConsole (game::TestApp* app) {
    std::cout << ">";
    std::string debugInput;
    std::getline(std::cin, debugInput);

    std::vector<std::string> args  = stringSplit(debugInput);

    std::string command = std::move(args[0]);
    args.erase(args.begin());

    if (command == "profiler") {
        handleProfiler(app, args);
    } else if (command == "level"){
        /*if (args.size() == 1 && args[0] == "reload") {
            bus->raise(event::ReloadMapEvent());
        } else*/ if (args.size() == 2 && args[0] == "dump") {
            //bus->raise(event::DumpMapEvent(args[1]));
            app->dumpLevel(args[1]);
        } else if (args.size() == 2 && args[0] == "load") {
            //bus->raise(event::MapLoadRequestEvent(args[1]));
            auto level = common::Assets::Load<game::Level>(args[1]);
            if (level) {
                level->load();
            }
        } else {
            logging::log(LEVEL_WARNING, "Unknown arguments for level command: \"{}\"", util::joinStrings(" ", args));
        }
    } else if (command == "theme") {
        handleThemes(app, args);
    } else if (command == "debug_render") {
        if (args.size() == 1) {
            if (args[0] == "true") {
                app->updateDebugRender(true);
            } else if (args[0] == "false") {
                app->updateDebugRender(false);
            } else {
                logging::log(LEVEL_WARNING, "Argument for debug_render command must be true or false");
            }
        } else {
            logging::log(LEVEL_WARNING, "debug_render command requires one argument: true/false");
        }
    } else if (command == "debug_step") {
        if (args.size() == 1) {
            if (args[0] == "true") {
                app->startStepping();
            } else if (args[0] == "false") {
                app->stopStepping();
            } else {
                logging::log(LEVEL_WARNING, "Argument for debug_step command must be true or false");
            }
        } else {
            logging::log(LEVEL_WARNING, "debug_step command requires one argument: true/false");
        }
    } else {
        logging::log(LEVEL_WARNING, "Unknown debug command: \"{}\"", command);
    }

}

void util::doDebugConsole (game::TestApp* app) {
    app->pause();
    ::doDebugConsole(app);
    app->queueResume();
}
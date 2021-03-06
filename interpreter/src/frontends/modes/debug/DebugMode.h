//
// Copyright (c) 2016 by Borja Lorente.
// Distributed under the GPLv3 license.
//

#ifndef NAYLANG_DEBUGMODE_H
#define NAYLANG_DEBUGMODE_H

#include <frontends/modes/ConsoleExecutionMode.h>
#include <core/control/Interpreter.h>
#include <core/control/Debugger.h>
#include <frontends/ConsoleFrontend.h>
#include "DebugCommand.h"

namespace naylang {
class Debugger;
class DebugCommand;
class DebugMode : public ConsoleExecutionMode {
    std::string _code;
    std::unique_ptr<Debugger> _debugger;
    std::unique_ptr<DebugCommand> _command;

public:
    DebugMode(ConsoleFrontend *frontend, const std::string &filename);

    void prompt() override;
    void runCommand(const std::string &name, const std::string &body) override;

private:

    void setCommand(const std::string &name, const std::string &body);
    void readCodeFile(const std::string &filename);
    int parseInt(const std::string &raw);
};
}

#endif //NAYLANG_DEBUGMODE_H

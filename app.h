#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class ICommand {
   public:
    ICommand() = default;
    virtual ~ICommand() = default;

    virtual void run(std::span<const std::string_view> args) = 0;
};

template <typename Config>
class Command : public ICommand {
   public:
    using Parser = std::function<Config(std::span<const std::string_view>)>;
    using Handler = std::function<void(const Config&)>;

    Command(Parser p, Handler h) : m_parser(std::move(p)), m_handler(std::move(h)) {}

    void run(std::span<const std::string_view> args) override { m_handler(m_parser(args)); }

   private:
    Parser m_parser;
    Handler m_handler;
};

struct RenderConfig {};

class App {
   private:
    std::map<std::string, std::unique_ptr<ICommand>> m_commands;
    std::vector<std::string_view> m_args;

    // Commands known at compile time; add new ones here.
    void registerBuiltins() {
        registerCommand<RenderConfig>(
            "render", [](std::span<const std::string_view>) { return RenderConfig{}; },
            [](const RenderConfig&) { /* TODO: render */ });
    }

   public:
    App(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) m_args.emplace_back(argv[i]);
        registerBuiltins();
    }

    template <typename Config>
    void registerCommand(std::string name, typename Command<Config>::Parser parser,
                         typename Command<Config>::Handler handler) {
        m_commands.insert_or_assign(std::move(name), std::make_unique<Command<Config>>(
                                                         std::move(parser), std::move(handler)));
    }

    int run() {
        if (m_args.empty()) {
            std::cerr << "no command given\n";
            return 1;
        }
        auto it = m_commands.find(std::string(m_args[0]));
        if (it == m_commands.end()) {
            std::cerr << "unknown command: " << m_args[0] << '\n';
            return 1;
        }
        it->second->run(std::span<const std::string_view>(m_args).subspan(1));
        return 0;
    }
};

#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <optional>

#include "json_loader.h"
#include "request_handler.h"
#include "logger.h"
#include "app.h"
#include "extra_data.h"
#include "ticker.h"
#include "model_serialization.h"
#include "infrastructure.h"
#include "postgres.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;


namespace {

    template <typename Fn>
    void RunWorkers(unsigned num_threads, const Fn& fn) {
        num_threads = std::max(1u, num_threads);
        std::vector<std::jthread> workers;
        workers.reserve(num_threads - 1);
        while (--num_threads) {
            workers.emplace_back(fn);
        }
        fn();
    }

    struct Args {
        int tick_period;
        int save_state_period;
        std::string config_file;
        std::string static_dir;
        std::string state_file;
        bool ramdomize = false;
        bool without_state_file = false;
    }; 

    [[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
        namespace po = boost::program_options;

        po::options_description desc{"Allowed options"s};
        Args args;
        desc.add_options()
            ("help,h", "produced help message")
            ("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s), "set tick period")
            ("save-state-period,s", po::value(&args.save_state_period)->value_name("milliseconds"s), "set save state period")
            ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config path")
            ("www-root,w", po::value(&args.static_dir)->value_name("dir"s), "set static file root")
            ("state-file,f", po::value(&args.state_file)->value_name("file"s), "set state path")
            ("randomize-spawn-points", "spawn dogs at random positions");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help"s)) {
            std::cout << desc;
            return std::nullopt;
        }
        if(!vm.contains("state-file")){
            args.without_state_file= true;
        }
        if(vm.contains("randomize-spawn-points")){
            args.ramdomize = true;
        }
        if(!vm.contains("save-state-period")){
            args.save_state_period = -1;
        }   
        if(!vm.contains("tick-period")){
            args.tick_period = -1;
        } 
        if (!vm.contains("config-file"s)) {
            throw std::runtime_error("Config file have not been specified"s);
        }
        if (!vm.contains("www-root"s)) {
            throw std::runtime_error("Static directory have not been specified"s);
        }
        return args;
    }

}  // namespace


int main(int argc, const char* argv[]) {   
    try {
        logger::LogParametr();
        if(auto args = ParseCommandLine(argc, argv)){     
            model::Game game = json_loader::LoadGame(args->config_file);  
            extra_data::LootJsonData loot = json_loader::LoadLootData(args->config_file);

            if(args->ramdomize){
                game.SetRandomaizer();
            }

            app::Application app{game, postgres::GetConfigFromEnv()}; 
            
            insfrastruct::SerializationListener ser_lis(std::chrono::milliseconds(args->save_state_period)); 

            if(!args->without_state_file){   
                if(args->save_state_period != -1){
                    ser_lis.SetMainSerFile(args->state_file);
                    app.SetListener(ser_lis);
                } 
                serialization::AppDeserialization(args->state_file, app); 
            }

            const unsigned num_threads = std::thread::hardware_concurrency();
            net::io_context ioc(num_threads);

            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    ioc.stop();
                }
            });

            std::filesystem::path static_files_root = args->static_dir;
            auto api_strand = net::make_strand(ioc);

            bool accept_tick = true;
            if(args->tick_period != -1){
                auto ticker = std::make_shared<ticker::Ticker>(api_strand, std::chrono::milliseconds(args->tick_period),
                    [&app](std::chrono::milliseconds delta) { app.Tick(delta); }
                );
                ticker->Start();
                accept_tick  = false;
            }

            auto handler = std::make_shared<http_handler::RequestHandler>(
                static_files_root, api_strand, app, accept_tick, loot);

            http_handler::LoggingRequestHandler<http_handler::RequestHandler> log_handler{*handler, api_strand} ;

            const auto address = net::ip::make_address("0.0.0.0");
            constexpr net::ip::port_type port = 8080;
            http_server::ServeHttp(ioc, {address, port}, [&log_handler](auto endpoint, auto&& req, auto&& send) {
                log_handler(endpoint, std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            });

            json::value server_start_data({{"port"s, port}, {"address"s, address.to_string()}});
            logger::LogInfo(server_start_data, "server started"sv);

            RunWorkers(std::max(1u, num_threads), [&ioc] {
                ioc.run();
            });

            if(!args->without_state_file){
                serialization::AppSerialization(args->state_file, app);
            }

            json::value server_end_data{{"code"s, 0}};
            logger::LogInfo(server_end_data, "server exited"sv);
        } 
    }
    catch(const std::exception& e){
        json::value server_end_data({{"code"s, EXIT_FAILURE}, {"exeption"s, e.what()}});   
        logger::LogInfo(server_end_data, "server exited"sv);
        return EXIT_FAILURE;
    }  
}

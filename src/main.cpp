#include <gvox/gvox.h>
#include <cxxopts.hpp>
#include <filesystem>

struct Options {
    std::optional<int> return_value{};
    std::string i_fmt{};
    std::string o_fmt{};
    bool o_raw{};
    bool i_raw{};
    std::filesystem::path i_path{};
    std::filesystem::path o_path{};
};

auto process_options(int argc, char **argv) -> Options try {
    auto result = Options{};
    cxxopts::Options options_config("gvox-cli", "gvox-cli is a command line interface for the gvox format library");
    options_config.add_options()                                                                          //
        ("i,input", "Input file", cxxopts::value<std::string>()->default_value("input"))                  //
        ("i_fmt", "Input file format", cxxopts::value<std::string>()->default_value("gvox"))              //
        ("o,output", "Output file", cxxopts::value<std::string>()->default_value("output"))               //
        ("o_fmt", "Output file format", cxxopts::value<std::string>()->default_value("gvox_u32_palette")) //
        ("o_raw", "Output the file without the gvox meta wrapper")                                        //
        ("v,version", "Print Version")                                                                    //
        ("h,help", "Print usage")                                                                         //
        ;
    options_config.allow_unrecognised_options();
    auto options = options_config.parse(argc, argv);
    auto const &unmatched = options.unmatched();
    if (!unmatched.empty()) {
        std::cout << "\nPassed unknown options (";
        for (size_t i = 0; i < unmatched.size() - 1; ++i) {
            std::cout << unmatched[i] << ", ";
        }
        std::cout << unmatched[unmatched.size() - 1] << ")\n\n"
                  << options_config.help() << "\nClosing." << std::endl;
        result.return_value = -1;
        return result;
    }
    if ((options.count("help") != 0u) || argc < 2) {
        std::cout << options_config.help() << std::endl;
        result.return_value = 0;
        return result;
    }
    if (options.count("version") != 0u) {
        std::cout << "gvox-cli version " << GVOX_CLI_VERSION_STRING << std::endl;
        result.return_value = 0;
        return result;
    }
    auto input = options["input"].as<std::string>();
    auto output = options["output"].as<std::string>();
    result.i_fmt = options["i_fmt"].as<std::string>();
    result.o_fmt = options["o_fmt"].as<std::string>();
    result.o_raw = static_cast<bool>(options.count("o_raw"));
    result.i_raw = result.i_fmt != "gvox";
    result.i_path = std::filesystem::path{input};
    result.o_path = std::filesystem::path{output};
    if (result.i_path.has_extension() && result.i_path.extension() != "gvox") {
        result.i_raw = true;
    }
    if (!std::filesystem::exists(result.i_path)) {
        if (!result.i_path.has_extension()) {
            auto potential_extensions = std::vector<std::string>{};
            potential_extensions.push_back(result.i_fmt);
            if (result.i_fmt == "magicavoxel") {
                potential_extensions.emplace_back("vox");
            } else if (result.i_fmt == "ace_of_spades") {
                potential_extensions.emplace_back("vxl");
            }
            potential_extensions.emplace_back("gvox");
            for (auto const &potential_extension : potential_extensions) {
                auto i_path_test = std::filesystem::path{result.i_path.string() + "." + potential_extension};
                if (!std::filesystem::exists(i_path_test)) {
                    if (&potential_extension == &potential_extensions.back()) {
                        std::cerr << "[ERROR] Failed to find the specified input file " << result.i_path << std::endl;
                        result.return_value = -1;
                        return result;
                    }
                } else {
                    if (&potential_extension != &potential_extensions.back()) {
                        result.i_raw = true;
                    }
                    result.i_path = i_path_test;
                    break;
                }
            }
        } else {
            std::cerr << "[ERROR] Failed to find the specified input file " << result.i_path << std::endl;
            result.return_value = -1;
            return result;
        }
    }
    if (!result.o_raw) {
        if (result.o_path.has_extension()) {
            std::cout << "[WARNING] The specified output file has a custom file extension, but is being saved as a gvox file. Consider saving as a raw file, or removing the file extension from the command line argument. Appending .gvox" << std::endl;
        }
        result.o_path = std::filesystem::path{result.o_path.string() + ".gvox"};
    } else {
        auto o_ext = result.o_fmt;
        if (result.o_fmt == "magicavoxel") {
            o_ext = "vox";
        } else if (result.o_fmt == "ace_of_spades") {
            o_ext = "vxl";
        }
        if (!result.o_path.has_extension()) {
            result.o_path = std::filesystem::path{result.o_path.string() + "." + o_ext};
        }
    }
    return result;
} catch (...) {
    std::cerr << "[ERROR] Caught an exception. This should not happen! Please open a GitHub issue with details of what arguments you supplied at https://github.com/GabeRundlett/gvox-cli/issues" << std::endl;
    return Options{.return_value = -1};
}

auto main(int argc, char **argv) -> int {
    auto options = process_options(argc, argv);
    if (options.return_value.has_value()) {
        return options.return_value.value();
    }
    auto *gvox_ctx = gvox_create_context();
    GVoxScene scene;
    if (options.i_raw) {
        scene = gvox_load_from_raw(gvox_ctx, options.i_path.string().c_str(), options.i_fmt.c_str());
    } else {
        scene = gvox_load(gvox_ctx, options.i_path.string().c_str());
    }
    auto handle_errors = [&]() {
        while (gvox_get_result(gvox_ctx) != GVOX_SUCCESS) {
            size_t msg_size = 0;
            gvox_get_result_message(gvox_ctx, nullptr, &msg_size);
            std::string msg;
            msg.resize(msg_size);
            gvox_get_result_message(gvox_ctx, nullptr, &msg_size);
            gvox_pop_result(gvox_ctx);
            std::cerr << "(gvox) - " << msg << std::endl;
        }
        gvox_destroy_scene(&scene);
        gvox_destroy_context(gvox_ctx);
    };
    if (gvox_get_result(gvox_ctx) != GVOX_SUCCESS) {
        std::cerr << "[ERROR] Internal gvox error while loading:\n";
        handle_errors();
        return -1;
    }
    if (options.o_raw) {
        gvox_save_as_raw(gvox_ctx, &scene, options.o_path.string().c_str(), options.o_fmt.c_str());
    } else {
        gvox_save(gvox_ctx, &scene, options.o_path.string().c_str(), options.o_fmt.c_str());
    }
    if (gvox_get_result(gvox_ctx) != GVOX_SUCCESS) {
        std::cerr << "[ERROR] Internal gvox error while saving:\n";
        handle_errors();
        return -1;
    }
    gvox_destroy_scene(&scene);
    gvox_destroy_context(gvox_ctx);
}

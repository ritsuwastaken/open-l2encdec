#include "webp.h"
#include "write_queue.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <l2encdec.h>
#include <string>
#include <thread>
#include <ueviewer.h>
#include <vector>

namespace
{
constexpr std::string_view DEFAULT_OUTPUT_DIR = "webp_output";
constexpr int PROGRESS_BAR_WIDTH = 50;
constexpr size_t DEFAULT_HEADER_SIZE = 28;

inline std::vector<unsigned char> read_file(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    return std::vector<unsigned char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

void write_file(const std::string &filename, const std::vector<unsigned char> &data)
{
    std::ofstream file(filename, std::ios::binary);
    std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
    file.write(reinterpret_cast<const char *>(data.data()), data.size());
}

int read_protocol_from_input_data(const std::vector<unsigned char> &data)
{
    if (data.size() < DEFAULT_HEADER_SIZE)
        return 0;

    std::string string;
    for (size_t i = 0; i < DEFAULT_HEADER_SIZE; i += 2)
    {
        if (data[i + 1] == 0)
        {
            string += static_cast<char>(data[i]);
        }
    }

    try
    {
        int protocol = std::stoi(string.substr(11));
        if (std::find(std::begin(l2encdec::SUPPORTED_PROTOCOLS), std::end(l2encdec::SUPPORTED_PROTOCOLS), protocol) != std::end(l2encdec::SUPPORTED_PROTOCOLS))
            return protocol;
        return 0;
    }
    catch (...)
    {
        return 0;
    }
}

void print_progress(size_t current, size_t total)
{
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(PROGRESS_BAR_WIDTH * progress);

    std::cout << "\r[";
    for (int i = 0; i < PROGRESS_BAR_WIDTH; ++i)
        std::cout << (i < pos ? "=" : i == pos ? ">"
                                               : " ");

    std::cout << "] " << std::fixed << std::setprecision(1)
              << (progress * 100.0) << "% (" << current << "/" << total << ")" << std::flush;
}

void process_textures(const std::vector<TextureData> &textures, const std::string &output_path)
{
    std::atomic<size_t> exported_textures = 0;
    std::vector<std::thread> threads;
    WriteQueue write_queue;
    bool processing_complete = false;
    std::thread progress_thread([&]()
                                {
            while (!processing_complete) {
                print_progress(exported_textures, textures.size());
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            print_progress(exported_textures, textures.size());
            std::cout << std::endl; });

    std::thread writer_thread([&write_queue, &output_path]()
                              {
            WriteTask task;
            while (write_queue.pop(task)) {
                write_file(output_path + "/" + task.filename, task.data);
            } });

    const size_t num_threads = std::min(static_cast<size_t>(std::thread::hardware_concurrency()), textures.size());
    const size_t textures_per_thread = (textures.size() + num_threads - 1) / num_threads;
    for (size_t t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&, t]()
                             {
                const size_t start = t * textures_per_thread;
                const size_t end = std::min(start + textures_per_thread, textures.size());

                for (size_t i = start; i < end; ++i) {
                    const auto& texture = textures[i];
                    std::vector<unsigned char> webp_data;

                    switch (texture.format) {
                    case TextureFormat::TPF_DXT1:
                        webp::from_dxt1(texture.data, texture.width, texture.height, webp_data);
                        exported_textures++;
                        break;
                    case TextureFormat::TPF_DXT3:
                        webp::from_dxt3(texture.data, texture.width, texture.height, webp_data);
                        exported_textures++;
                        break;
                    default:
                        break;
                    }

                    if (!webp_data.empty()) {
                        write_queue.push(texture.name + ".webp", std::move(webp_data));
                    }
                } });
    }

    for (auto &thread : threads)
        thread.join();

    processing_complete = true;
    progress_thread.join();
    write_queue.finish();
    writer_thread.join();

    std::cout << "Exported to " << output_path << std::endl;
}
} // namespace

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./utx121webp <input_file> [output_path]\n";
        return 1;
    }

    try
    {
        std::string filename = std::filesystem::path(argv[1]).filename().string();
        std::string filename_no_ext = std::filesystem::path(argv[1]).filename().stem().string();
        std::transform(filename_no_ext.begin(), filename_no_ext.end(), filename_no_ext.begin(), ::tolower);
        const std::string output_path = (argc > 2) ? std::string(argv[2]) : std::string(DEFAULT_OUTPUT_DIR) + "/" + filename_no_ext;

        auto encrypted_package = read_file(argv[1]);

        int protocol = read_protocol_from_input_data(encrypted_package);

        std::vector<unsigned char> decoded_package;
        if (protocol != 0)
        {
            l2encdec::Params params;
            if (!l2encdec::init_params(&params, protocol, filename))
                throw std::runtime_error("Failed to initialize L2 protocol parameters");

            if (l2encdec::decode(encrypted_package, decoded_package, params) != l2encdec::DecodeResult::SUCCESS)
                throw std::runtime_error("Failed to decode package: " + std::string(argv[1]));
        }
        else
        {
            decoded_package = std::move(encrypted_package);
        }

        UEViewer::getInstance().initialize();
        std::vector<TextureData> textures;
        if (!UEViewer::getInstance().extract_textures(filename, decoded_package, textures))
            throw std::runtime_error("Failed to load package: " + filename);

        process_textures(textures, output_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

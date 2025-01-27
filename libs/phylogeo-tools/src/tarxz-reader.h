#ifndef __TARXZ_READER_H__
#define __TARXZ_READER_H__
#include <archive.h>
#include <archive_entry.h>
#include <lzma.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <deque>

class TarXZReader {
private:
    struct archive *a = nullptr;
    std::deque<char> line_buffer;
    bool eof = true;

public:
    la_int64_t current_file_size = 0;
    la_int64_t current_file_offset = 0;

    TarXZReader() = default;

    ~TarXZReader() {
        close();
    }

    // Open the .tar.xz file
    bool open(const std::string &filename) {
        eof = true;
        a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);

        int r;
        if ((r = archive_read_open_filename(a, filename.c_str(), 10240))) {
            std::cerr << "Failed to open file " << filename << std::endl;
            return false;
        }
        return true;
    }

    // Close all resources
    void close() {
        if (a != nullptr) {
            archive_read_close(a);
            archive_read_free(a);
            a = nullptr;
        }
    }

    int last_no_eol_pos = 0;
    // Seek into a specific file inside the archive
    bool seekFile(const std::string &filePath) {
        struct archive_entry *entry;

        for (;;) {
            int r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            if (r < ARCHIVE_OK) {
                std::cerr << archive_error_string(a) << std::endl;
                return false;
            }
            if (r < ARCHIVE_WARN) {
                std::cerr << "W: " << archive_error_string(a) << std::endl;
                return false;
            }
            if (archive_entry_size(entry) > 0) {
                current_file_size = archive_entry_size(entry);
                // std::cerr << archive_entry_pathname(entry) << " " << "size=" << current_file_size << std::endl;
                if (archive_entry_pathname(entry) == filePath) {
                    line_buffer.clear();
                    last_no_eol_pos = 0;
                    eof = false;
                    return true;
                }
            }
        }
        return false;
    }

    // Read the next line from the currently selected file
    bool getline(std::string &line) {

        // TODO: we will have problem with files without \n at the end!
        for (; !eof;) {
            auto pos = std::find(line_buffer.begin() + last_no_eol_pos, line_buffer.end(), '\n');
            // std::cerr << "R.gl " << (pos - line_buffer.begin()) << "/" << line_buffer.size() << " " << last_no_eol_pos << std::endl;
            if (pos != line_buffer.end()) {
                // line = line_buffer.substr(0, pos);
                line = std::string(line_buffer.begin(), pos);
                // line = ">hCoV-19/A/A/B|2021-01-01";
                // line_buffer.erase(0, pos + 1);
                line_buffer.erase(line_buffer.begin(), pos+1);
                last_no_eol_pos = 0;
                return true;
            } else {
                last_no_eol_pos = line_buffer.end() - line_buffer.begin();
            }

            const void *buff;
            size_t size;
            la_int64_t offset;
            int r = archive_read_data_block(a, &buff, &size, &offset);
            current_file_offset = offset;
            // std::cerr << "R " << size << " " << offset << " " << r << std::endl;
            if (r == ARCHIVE_OK && size > 0) {
                // line_buffer.append((char*)buff, size);
                // for (size_t s = 0; s < size; s++)
                //     line_buffer.push_back(static_cast<const char*>(buff)[s]);
                line_buffer.insert(line_buffer.end(), static_cast<const char*>(buff), static_cast<const char*>(buff)+size);
                continue;
            }
            if (r == ARCHIVE_EOF || (r == ARCHIVE_OK && size == 0)) {
                // std::cerr << "R-EOF '" << line_buffer << "'" << std::endl;
                eof = true;
                // line = line_buffer;
                // line_buffer = "";
                // return true;
                return false;
            }
            if (r < ARCHIVE_OK) {
                std::cerr << "Error: " << r << std::endl;
                return false;
            }
        }

        return false;
    }
};

bool getline(TarXZReader& is, std::string& line) {
    return is.getline(line);
}

#endif

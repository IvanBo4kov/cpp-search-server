#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace filesystem;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories);

// helper
string GetFileContents(const path& file) {
    ifstream stream(file);

    if (!stream.is_open()) {
        cerr << "Unable to open file: " << file << endl;
        return "";
    }

    ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

void PrintUnknownIncludeError(const string& include, const string& file, size_t line) {
    cout << "unknown include file " << include << " at file " << file << " at line " << line << endl;
}

bool ProcessFile(const path& file, ostream& output, const vector<path>& include_directories);

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ofstream output(out_file);

    if (!output.is_open()) {
        cerr << "Unable to create output file: " << out_file << endl;
        return false;
    }

    return ProcessFile(in_file, output, include_directories);
}

bool ProcessFile(const path& file, ostream& output, const vector<path>& include_directories) {
    static const regex local_include_regex(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static const regex system_include_regex(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

    string content = GetFileContents(file);

    if (content.empty()) {
        return false;
    }

    smatch match;
    size_t line_number = 1;
    istringstream content_stream(content);
    string line;

    while (getline(content_stream, line)) {
        if (regex_match(line, match, local_include_regex)) {
            path include_path = match[1].str();
            bool found_include = false;

            path full_path = file.parent_path() / include_path;
            if (!exists(full_path)) {
                for (const path& dir : include_directories) {
                    full_path = dir / include_path;
                    if (exists(full_path)) {
                        found_include = true;
                        if (!ProcessFile(full_path, output, include_directories)) {
                            return false;
                        }
                        break;
                    }
                }
            } else {
                found_include = true;
                if (!ProcessFile(full_path, output, include_directories)) {
                    return false;
                }
            }

            if (!found_include) {
                PrintUnknownIncludeError(include_path.string(), file.string(), line_number);
                return false;
            }
        } else if (regex_match(line, match, system_include_regex)) {
            bool found_include = false;
            for (const path& dir : include_directories) {
                path full_path = dir / match[1].str();
                if (exists(full_path)) {
                    found_include = true;
                    if (!ProcessFile(full_path, output, include_directories)) {
                        return false;
                    }
                    break;
                }
            }

            if (!found_include) {
                PrintUnknownIncludeError(match[1].str(), file.string(), line_number);
                return false;
            }
        } else {
            output << line << endl;
        }
        line_number++;
    }

    return true;
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}


int main() {
    // Test the Preprocess function
    Test();
    return 0;
}

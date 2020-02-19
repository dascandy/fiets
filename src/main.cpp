#include <iostream>
#include "parser.h"
#include <filesystem>
#include <fstream>
#include "html.h"

int main(int , char** argv) {
  std::string body;
  body.resize(std::filesystem::file_size(argv[1]));
  std::ifstream(argv[1]).read(body.data(), body.size());
  Document doc = parse(body);
  std::string html = as_html(doc);
  std::ofstream(argv[2]).write(html.data(), html.size());
}



#include "parser.h"
#include <string_view>

std::vector<std::string_view> split(std::string_view data, std::string chars) {
  std::vector<std::string_view> lines;
  size_t start = 0;
  size_t end = data.find_first_of(chars);
  while (start != std::string::npos) {
    lines.push_back(std::string_view(data.data() + start, (end == std::string::npos ? data.size() : end) - start));
    if (end == std::string::npos) break;
    start = end + 1;
    end = data.find_first_of(chars, start);
  }
  return lines;
}

std::string_view parseText(std::string_view line) {
  return line;
}

Chapter parse(std::string_view file) {
  size_t lineNumber = 0;
  Chapter doc(0, "");
  Chapter* currentChapter = &doc;
  std::string accumulated = "";
  std::string codeLanguage = "";
  enum {
    Toplevel,
    Codeblock,
  } state = Toplevel;
  for (auto& line : split(file, "\n")) {
    lineNumber++;
    if (lineNumber == 1) {
      currentChapter->text = line;
      continue;
    }
    switch(state) {
    case Toplevel:
      if (line.empty()) { }
      else if (line.starts_with("#")) {
        size_t hashCount = line.find_first_not_of("#");
        currentChapter = &doc;
        for (size_t n = 0; n < hashCount - 1; n++) {
          if (currentChapter->subchapters.empty()) {
            fprintf(stderr, "Chapter omitted in sequence on line %zu", lineNumber);
            currentChapter->subchapters.emplace_back(n+1, "");
          }
          currentChapter = &currentChapter->subchapters.back();
        }
        currentChapter->subchapters.emplace_back(hashCount, line.substr(hashCount + 1));
        currentChapter = &currentChapter->subchapters.back();
      } else if (line.starts_with("```")) {
        state = Codeblock;
        codeLanguage = line.substr(3);
        accumulated = "";
      } else if (line.starts_with("- ")) {
        if (currentChapter->entries.empty() ||
          !std::holds_alternative<List>(currentChapter->entries.back()))
          currentChapter->entries.push_back(List{});

        std::get<List>(currentChapter->entries.back()).entries.push_back(line.substr(2));
      } else if (line.starts_with("'") && line.find("':") != std::string::npos) {
        size_t closePos = line.find("':");
        currentChapter->entries.push_back(IdentifierDefinition{line.substr(1, closePos), line.substr(closePos+2)});
      } else if (line.starts_with("|")) {
        if (currentChapter->entries.empty() ||
          !std::holds_alternative<Table>(currentChapter->entries.back()))
          currentChapter->entries.push_back(Table{});

        std::get<Table>(currentChapter->entries.back()).entries.push_back(split(line.substr(1, line.size() - 2), "|"));
      } else if ( false /* is an ordered list entry */) {
        // TODO
      } else {
        currentChapter->entries.push_back(Text{line});
      }
      break;
    case Codeblock:
      if (line == "```") {
        state = Toplevel;
        currentChapter->entries.push_back(Code{codeLanguage, accumulated});
      } else {
        accumulated += "\n";
        accumulated += line;
      }
      break;
    }
  }
  return doc;
}



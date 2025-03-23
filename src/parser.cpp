#include "parser.h"
#include <string_view>

uint32_t Document::addReference(std::string name, std::string url) {
  for (size_t n = 0; n < references.size(); n++) {
    if (references[n].url == url) return references[n].index;
  }
  references.push_back(Referenced{(uint32_t)references.size() + 1, url, name});
  return references.back().index;
}

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

Text parseText(std::string_view line, Document& doc) {
  Text text;
  std::string accum;
  size_t offset = 0;
  while (offset != line.size()) {
    size_t end = line.find_first_of("`+-\\['", offset);
    if (end == std::string::npos) {
      accum += line.substr(offset);
      text.seq.push_back(accum);
      offset = line.size();
      break;
    } else {
      accum += line.substr(offset, end - offset);
      offset = end;
      switch(line[offset]) {
      case '\\':
        accum += line[offset+1];
        offset += 2;
        break;
      case '+': 
        if (line[offset+1] == '+' && line[offset+2] == '+') {
          if (!accum.empty()) text.seq.push_back(accum);
          accum = "";
          size_t end = line.find("+++", offset + 3);
          text.seq.push_back(Insertion{parseText(line.substr(offset + 3, end == std::string::npos ? end : end - offset - 3), doc)});
          offset = end == std::string::npos ? line.size() : end + 3;
        } 
        else 
        {
          accum += '+';
          offset++;
        }
        break;
      case '-':
        if (line[offset+1] == '-' && line[offset+2] == '-') {
          if (!accum.empty()) text.seq.push_back(accum);
          accum = "";
          size_t end = line.find("---", offset + 3);
          text.seq.push_back(Deletion{parseText(line.substr(offset + 3, end - offset - 3), doc)});
          offset = end + 3;
        } 
        else 
        {
          accum += '-';
          offset++;
        }
        break;
      case '\'':
      {
        if (!std::isspace(line[offset-1]) && !std::isspace(line[offset+1])) {
          // Apostrophe inside a word, ignore
          accum += '\'';
          offset++;
        } else {
          if (!accum.empty()) text.seq.push_back(accum);
          accum = "";
          size_t end = line.find("'", offset + 1);
          text.seq.push_back(Identifier{line.substr(offset + 1, end - offset - 1)});
          offset = end + 1;
        }
      }
        break;
      case '[':
      {
        if (!accum.empty()) text.seq.push_back(accum);
        accum = "";
        size_t end = line.find("]", offset + 1);
        if (line[end+1] == '(') {
          size_t end2 = line.find(")", end + 2);
          uint32_t index = doc.addReference(std::string(line.substr(end + 2, end2 - end - 2)), std::string(line.substr(offset + 1, end - offset - 1)));
          text.seq.push_back(Reference{index});
          offset = end2 + 1;
        } else {
          uint32_t index = doc.addReference(std::string(line.substr(offset + 1, end - offset - 1)), std::string(line.substr(offset + 1, end - offset - 1)));
          text.seq.push_back(Reference{index});
          offset = end + 1;
        }
      }
        break;
      case '`':
      {
        if (!accum.empty()) text.seq.push_back(accum);
        accum = "";
        size_t end = line.find("`", offset + 1);
        text.seq.push_back(CodeSpan{line.substr(offset + 1, end - offset - 1)});
        offset = end + 1;
      }
        break;
      }
    }
  }
  return text;
}

Document parse(std::string_view file) {
  size_t lineNumber = 0;
  Document doc;
  Chapter* currentChapter = &doc;
  std::string accumulated = "";
  std::string codeLanguage = "";
  enum {
    Toplevel,
    Codeblock,
  } state = Toplevel;
  for (auto& line : split(file, "\n")) {
    lineNumber++;
    printf("line %zu\n", lineNumber);
    if (lineNumber == 1) {
      doc.title = line;
      continue;
    } else if (lineNumber == 2 && !line.empty()) {
      doc.subtitle = line;
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
      } else if (line.starts_with("> ")) {
        Text text = parseText(line.substr(2), doc);
        if (currentChapter->entries.empty() ||
            not std::holds_alternative<Quote>(currentChapter->entries.back())) {
          currentChapter->entries.push_back(Quote{});
        }
        std::get<Quote>(currentChapter->entries.back()).texts.push_back(text);
      } else if (line.starts_with("[[references]]")) {
        currentChapter->entries.push_back(References());
      } else if (line.starts_with("[[TOC]]")) {
        currentChapter->entries.push_back(TOC());
      } else if (line.starts_with("- ")) {
        if (currentChapter->entries.empty() ||
          !std::holds_alternative<List>(currentChapter->entries.back()))
          currentChapter->entries.push_back(List{});

        std::get<List>(currentChapter->entries.back()).entries.push_back(parseText(line.substr(2), doc));
      } else if (line.starts_with("'") && line.find("':") != std::string::npos) {
        size_t closePos = line.find("':");
        currentChapter->entries.push_back(IdentifierDefinition{line.substr(1, closePos), parseText(line.substr(closePos+2), doc)});
      } else if (line.starts_with("|")) {
        if (currentChapter->entries.empty() ||
          !std::holds_alternative<Table>(currentChapter->entries.back()))
          currentChapter->entries.push_back(Table{});

        std::get<Table>(currentChapter->entries.back()).entries.push_back({});
        auto& lineEntries = std::get<Table>(currentChapter->entries.back()).entries.back();
        for (auto entry : split(line.substr(1, line.size() - 2), "|")) {
          lineEntries.push_back(parseText(entry, doc));
        }
      } else if (line.size() >= 10 && line.find_first_not_of("-") == std::string::npos) {
//        currentChapter->entries.push_back(PageBreak{});
      } else if ( false /* is an ordered list entry */) {
        // TODO
      } else {
        currentChapter->entries.push_back(parseText(line, doc));
      }
      break;
    case Codeblock:
      if (line == "```") {
        state = Toplevel;
        currentChapter->entries.push_back(Code{codeLanguage, accumulated});
      } else {
        if (not accumulated.empty())
          accumulated += "\n";
        accumulated += line;
      }
      break;
    }
  }
  return doc;
}



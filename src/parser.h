#pragma once

#include <variant>
#include <vector>
#include <string>

struct Code {
  std::string_view language = "c++";
  std::string_view body;
};
struct List {
  std::vector<std::string_view> entries; 
};
struct OrderedList {
  std::vector<std::string_view> entries; 
};
struct IdentifierDefinition {
  std::string_view identifier; 
  std::string_view definition;
};
struct Table {
  std::vector<std::vector<std::string_view>> entries;
};
struct Text {
  std::string_view text;
};

using DocumentEntry = std::variant<Code, List, OrderedList, IdentifierDefinition, Table, Text>;

struct Chapter {
  int level;
  std::string_view text;
  std::vector<DocumentEntry> entries;
  std::vector<Chapter> subchapters;
  Chapter(int level, std::string_view text) 
  : level(level)
  , text(text)
  {}
};

Chapter parse(std::string_view file);

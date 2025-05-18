#pragma once

#include <variant>
#include <vector>
#include <string>

struct Insertion;
struct Deletion;
struct Reference;
struct FootnoteRef;
struct Identifier;
struct CodeSpan { std::string_view text; };
struct Code {
  std::string_view language = "cpp";
  std::string body;
};
struct Text {
  std::vector<std::variant<std::string, Insertion, Deletion, Reference, Identifier, CodeSpan>> seq; 
};

struct Referenced {
  uint32_t index;
  std::string url; 
  std::string name;
};

struct Insertion { Text text; };
struct Deletion { Text text; };
struct Reference { uint32_t index; };
struct Identifier { std::string_view text; };

struct List {
  std::vector<Text> entries; 
};
struct OrderedList {
  std::vector<Text> entries; 
};
struct IdentifierDefinition {
  std::string_view identifier; 
  Text definition;
};
struct Table {
  std::vector<std::vector<Text>> entries;
};
struct Quote { std::vector<Text> texts; };

struct References {};
struct TOC {};

using DocumentEntry = std::variant<Code, List, OrderedList, IdentifierDefinition, Table, Text, References, TOC, Quote>;

struct Chapter {
  int level;
  std::string_view title;
  std::vector<DocumentEntry> entries;
  std::vector<Chapter> subchapters;
  Chapter(int level, std::string_view title) 
  : level(level)
  , title(title)
  {}
};

struct Document : Chapter {
  std::string_view subtitle;
  Document() : Chapter{0, ""} {}
  std::vector<Referenced> references;
  uint32_t addReference(std::string url, std::string name);
};

Document parse(std::string_view file);
